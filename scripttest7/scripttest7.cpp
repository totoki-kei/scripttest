// scripttest7.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <unordered_map>
#include <variant>
#include <vector>
#include <string>

#include "string_manipulation.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

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

OpcodeDesc descset[] = {
	{ "ret", 1 },
	{"data", 1 },
	{"call", 3 },
};

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
	SegmentType segment;
	SegmentType addressing_segment;
	int32_t offset;
	std::string label;
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

	Entity(const SourceLine* line_info)
		: line_info(line_info) {
	}
	Entity(const SourceLine* line_info, std::unique_ptr<EntityDetail>&& detail)
		: line_info(line_info), detail(std::move(detail)) {
	}
};


std::shared_ptr<Entity> make_entity(const SourceLine* source_line, std::vector<std::string>& tokens) {
	auto detail = std::make_unique<OperationEntityDetail>();



	return std::make_shared<Entity>(source_line, std::move(detail));
}



int main() {


	std::fstream fs;
	fs.open(".\\c.txt");

	auto lines = split_by_line(std::istreambuf_iterator(fs), std::istreambuf_iterator<char>());
	std::vector<SourceLine> source_lines;
	for (size_t i = 0; i < lines.size(); ++i) {
		source_lines.emplace_back(i, std::string(lines[ i ]));
	}
	lines.clear();

	std::unordered_map<std::string, Segment> segments;
	std::vector<Entity> annotations;
	Segment* current_segment = nullptr;
	std::string last_label_name;
	bool in_annotation_block = false;

	for (const auto& line : source_lines) {
		auto token_list = tokenize(line.text.begin(), line.text.end());
		for (auto& token : token_list) {
			std::cout << "<" << token << "> ";
		}
		std::cout << std::endl;

		continue;

		if (token_list.empty()) continue;

		if (token_list[ 0 ].starts_with("###")) {
			// アノテーションブロックの開始/終了
			in_annotation_block = !in_annotation_block;
		}
		else if (in_annotation_block || token_list[ 0 ].starts_with('#')) {
			// アノテーション行、またはアノテーションブロック内の行
			annotations.emplace_back(Entity{ &line });
		}
		else if (token_list[ 0 ].starts_with('.')) {
			// セグメント指定
			if (auto found = segments.find(token_list[ 0 ]); found == segments.end()) {
				segments.insert(
					std::make_pair(
						token_list[ 0 ],
						Segment{ token_list[ 0 ], {}, {} }
				));
			}
			current_segment = &segments[ token_list[ 0 ] ];
		}
		else if (token_list[ 0 ].ends_with(':')) {
			// ラベル
			last_label_name = token_list[ 0 ];
		}
		else {
			// エンティティ行
			if (current_segment == nullptr) {
				throw std::domain_error("no segment specified");
			}

			std::shared_ptr<Entity> ent = make_entity(&line, token_list);
			if (!ent) {
				throw std::domain_error("no segment specified");
			}
			
		}
	}

}




