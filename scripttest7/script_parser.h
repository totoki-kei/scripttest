#pragma once

#include <memory>
#include <string>
#include <vector>
#include <string>

namespace ast {
	struct Line{};
	using LinePtr = std::shared_ptr<Line>;

	struct Root{
		std::vector<LinePtr> lines;
	};
	
	struct Literal {
		enum {
			TYPE_INVALID,
			TYPE_INTEGER,
			TYPE_FLOAT,
			TYPE_STRING_SQ,
			TYPE_STRING_DQ,
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
		Literal(int64_t i)
			: type(TYPE_INTEGER)
			, int_value(i) {}
		Literal(double f)
			: type(TYPE_FLOAT)
			, float_value(f) {}
		Literal(std::string s, char quote_char)
			: type(quote_char == '"' ? TYPE_STRING_SQ : TYPE_STRING_DQ)
			, float_value()
			, string_value(s) {}
		Literal(std::nullptr_t, std::string nil_str)
			: type(TYPE_NIL)
			, int_value()
			, string_value(nil_str) {}
		Literal(bool b)
			: type(TYPE_BOOL)
			, int_value(b ? 1 : 0) {}

		// デバッグ用ダミー
		// 何が入ってくる？
		template <typename T>
		Literal(T&& t) {
			_ASSERT(0);
		}


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
				case TYPE_STRING_SQ:
				case TYPE_STRING_DQ:
					return string_value == other.string_value;
				case TYPE_NIL:
					return string_value == other.string_value;
				}
			}
			return false;
		}
	};

	struct Param {
		char segment;
		char addr_segment;
		int index;
		std::string symbol;
	};

	struct Annotation {

	};
}






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
	OperandDesc operand[ 3 ];
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

void test_ident();



