#pragma once

#include <memory>
#include <string>
#include <vector>
#include <string>
#include <optional>

#include <cstdio>

namespace ast {
	struct SourcePosition {
		int start_line, start_column;
		int end_line, end_column;
	};

	struct Literal {
		enum {
			TYPE_INVALID,
			TYPE_INTEGER,
			TYPE_FLOAT,
			TYPE_STRING,
			TYPE_NIL,
			TYPE_BOOL,
		} type;

		union {
			int64_t int_value;
			double float_value;
		};
		std::string string_value;
		Literal()
			: type(TYPE_INVALID)
			, int_value() {}
		explicit Literal(int64_t i)
			: type(TYPE_INTEGER)
			, int_value(i) {}
		explicit Literal(double f)
			: type(TYPE_FLOAT)
			, float_value(f) {}

		explicit Literal(std::string s)
			: type(TYPE_STRING)
			, int_value()
			, string_value(s) {}
		template <size_t N>
		explicit Literal(const char(&str)[N])
			: type(TYPE_STRING)
			, int_value()
			, string_value(str) {}

		explicit Literal(std::nullptr_t, std::string nil_str)
			: type(TYPE_NIL)
			, int_value()
			, string_value(nil_str) {}
		explicit Literal(bool b)
			: type(TYPE_BOOL)
			, int_value(b ? 1 : 0) {}

		Literal(const Literal& other) = default;
		Literal(Literal&& other) = default;

		Literal& operator =(const Literal& other) = default;
		Literal& operator =(Literal&& other) = default;

		bool operator==(const Literal& other) const {
			if (type == other.type) {
				switch (type) {
				case TYPE_INTEGER:
				case TYPE_BOOL:
					return int_value == other.int_value;
				case TYPE_FLOAT:
					return float_value == other.float_value;
				case TYPE_STRING:
					return string_value == other.string_value;
				case TYPE_NIL:
					return string_value == other.string_value;
				}
			}
			return false;
		}
	};

	struct Param {
		enum {
			TYPE_INVALID = -1,
			TYPE_LITERAL,
			TYPE_IDENT,
			TYPE_MEMORY,
		} type;

		Literal literal;

		char segment;
		char addr_segment;
		int index;
		bool is_extern;
		std::string symbol;

		Param()
			: type(TYPE_INVALID)
			, segment('\0')
			, addr_segment('\0')
			, index(0)
			, is_extern(false) {}
		Param(Literal lit)
			: type(TYPE_LITERAL)
			, literal(lit)
			, segment('\0')
			, addr_segment('\0')
			, index(0)
			, is_extern(false) {}

		Param(std::string ident, bool ext)
			: type(TYPE_IDENT)
			, segment('\0')
			, addr_segment('\0')
			, index(0)
			, is_extern(ext)
			, symbol(ident) {}
		template <size_t N>
		Param(const char(&ident)[N], bool ext)
			: type(TYPE_IDENT)
			, segment('\0')
			, addr_segment('\0')
			, index(0)
			, is_extern(ext)
			, symbol(ident) {}

		Param(int i, char seg0, char seg1)
			: type(TYPE_MEMORY)
			, segment(seg0)
			, addr_segment(seg1)
			, index(i)
			, is_extern(false) {}

		Param(const Param& other) = default;
		Param(Param&& other) = default;

		Param& operator =(const Param& other) = default;
		Param& operator =(Param&& other) = default;

		bool operator==(const Param& other) const {
			if (type == other.type) {
				switch (type) {
				case TYPE_LITERAL:
					return literal == other.literal;
				case TYPE_IDENT:
					return is_extern == other.is_extern
						&& symbol == other.symbol;
				case TYPE_MEMORY:
					return index == other.index;
				}
			}
			return false;
		}
	};

	struct Annotation {
		std::string name;
		std::vector<std::string> flags;
		std::vector<std::pair<std::string, std::string>> id_and_types;

		bool operator==(const Annotation& other) const {
			return name == other.name && flags == other.flags && id_and_types == other.id_and_types;
		}
	};

	struct Line {};
	using LinePtr = std::shared_ptr<Line>;

	struct LabelLine : Line {
		std::string name;
	};

	struct SegmentLine : Line {
		std::string name;
	};

	struct OperationLine : Line {
		std::string opname;
		std::vector<Param> input;
		std::optional<Param> output;
	};

	struct AnnotationLine : Line {
		Annotation annotation;

		AnnotationLine(const Annotation& annotation)
			: annotation(annotation) {}
	};

	struct AnnotationBlock : Line {
		std::vector<Annotation> annotation_list;
	};

	struct Root {
		std::vector<LinePtr> lines;
	};

}

void test_script(const std::string&);


struct SourceLine;
struct Entity;

struct SourceLine {
	size_t line_number;
	std::string text;
};

struct Label {
	std::string name;
	size_t index;
};

struct Segment {
	std::string name;
	std::vector<std::shared_ptr<Entity>> entities;
	std::vector<Label> labels;
};

struct OpcodeDesc {
	const char* name;
	int input_count;
};

//OpcodeDesc descset[] = {
//	{ "ret", 1 },
//	{"data", 1 },
//	{"call", 3 },
//};

enum class SegmentType : int32_t {
	// 無効 / (間接参照)間接参照しない
	Invalid = -1,
	// 各スクリプトインスタンス内データ領域
	Local = 0,
	// スクリプト共通データ領域(静的データ領域)
	Script = 1,
	// 実行スタック
	Stack = 2,
	// 外部
	Extern = 3,
};

struct OperandDesc {
	SegmentType line_segment;
	SegmentType addressing_segment;
	int32_t offset;
	std::string line_label;
};

struct EntityDetail {
	virtual ~EntityDetail() = 0 {}
};

struct OperationEntityDetail : EntityDetail {
	OpcodeDesc opcode;
	OperandDesc destination;
	OperandDesc operand[3];
};

struct AnnotationEntityDetail : EntityDetail {
	std::string name;
	std::vector<std::string> flags;
	std::vector<std::tuple<std::string, std::string>> params;
};

struct Entity {
	const SourceLine* line_info;
	std::unique_ptr<EntityDetail> detail;

	Entity(const SourceLine* line_info);
	Entity(const SourceLine* line_info, std::unique_ptr<EntityDetail>&& detail);
};


std::shared_ptr<Entity> make_entity(const SourceLine* source_line, std::vector<std::string>& tokens);

void test_script_parser();

