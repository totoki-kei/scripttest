#include "script.h"
#include "scriptOp.h"

#include <unordered_map>
#include <fstream>


#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>
#include <boost/fusion/tuple.hpp>

namespace Script { namespace Loader {
	
	class SimpleCodeProvider : public CodeProvider {
	public:

		std::vector<Code> codes;
		std::unordered_map<std::string, int> entrypoints;

		std::vector<std::string> stringTable;

		const Code& Get(int index) override {
			return codes[index];
		}

		int Length() override {
			return codes.size();
		}

		int Label(const char* ent) override {
			auto it = entrypoints.find(ent);
			if (it == entrypoints.end()) return -1;
			return it->second;
		}

		const char* GetString(int id) {
			if (id < 0 || stringTable.size() <= (size_t)id) return nullptr;
			return stringTable[id].c_str();
		}

		
	};

	std::shared_ptr<CodeProvider> Load(const char* filepath, Generator& gen) {
		std::ifstream file(filepath);
		if (file.bad()) return nullptr;

		std::string wholeText((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		return FromString(wholeText, gen);
	}

	std::shared_ptr<CodeProvider> FromString(const std::string& source, Generator& gen) {
		std::vector<Code> codes;
		codes.reserve((size_t)source.size() / 8);
		std::unordered_map<std::string, int> entrypoints;
		std::unordered_multimap<std::string, int> epToSolve;



		auto end = source.end();
		for (auto begin = source.begin(); begin != end; /* nop */) {

			namespace Q = boost::spirit::qi;

			auto &skip = ("/*" >> *((Q::byte_ - '*') | ('*' >> (Q::byte_ - '/'))) >> "*/") | (Q::space);

			using boost::spirit::qi::phrase_parse;
			using boost::spirit::qi::parse;

			float num;
			std::string strlabel;
			int intlabel;

			if (phrase_parse(begin, end, Q::float_, skip, num)) {
				// float

				codes.emplace_back(opPush, num);
			}
			else if (phrase_parse(begin, end, ('<' >> Q::lexeme[(Q::alpha | Q::char_("#$%_")) >> *(Q::alnum | Q::char_("#$%_"))] >> '>'), skip, strlabel)) {
				// エントリポイント

				entrypoints[strlabel] = codes.size();
			}
			else if (phrase_parse(begin, end, ('*' >> Q::int_), skip, intlabel)) {
				// ラベル

				codes.emplace_back(nullptr, intlabel);
			}
			else {
				// 命令のはず

				std::string sig, attr;

				if (!phrase_parse(begin, end, Q::lexeme[(Q::alpha | Q::char_("#$%_")) >> *(Q::alnum | Q::char_("#$%_"))], skip, sig)) {
					// 失敗した
					std::string failed_part;
					int dist = std::distance(source.begin(), begin);
					if (std::distance(begin, end) > 12) {
						failed_part.assign(begin, begin + 8);
						failed_part += "...";
					}
					else {
						failed_part.assign(begin, end);
					}
					throw std::domain_error("cannot parse at [" + failed_part + "] (byte " + std::to_string(dist) + ")");
				}

				phrase_parse(begin, end, '[' >> Q::no_skip[*(Q::char_ - ']')] >> ']', skip, attr);

				std::string ep;
				try {
					Code c = gen.MakeCode(sig, attr, ep);
					if (ep.size()) {
						epToSolve.insert({ ep, (int)codes.size() });
					}

					codes.push_back(c);

				}
				catch (...) {
					throw std::domain_error("failed to generate code (signature = " + sig + ", attribute = " + attr + ")");
				}

			}

		}

		codes.push_back(Code{ opEnd });

		if (epToSolve.size() > 0) {
			int lastSize = epToSolve.size();
			do {
				auto it = epToSolve.begin();
				while (it != epToSolve.end()) {
					auto ep = entrypoints.find(it->first);
					if (ep != entrypoints.end()) {
						codes[it->second].attr.ep_ = (uint32_t)ep->second;

						auto next = it;
						next++;
						epToSolve.erase(it);
						it = next;
						continue;
					}
					it++;
				}
			} while (epToSolve.size() > 0 && epToSolve.size() != lastSize);

			if (epToSolve.size() > 0) {
				// 一部シンボルを解決できなかった
				throw std::domain_error("cannot resolve symbol.");
			}
		}

		auto ret = std::make_unique<SimpleCodeProvider>();
		ret->codes = std::move(codes);
		ret->entrypoints = std::move(entrypoints);
		ret->stringTable = std::move(gen.stringTable);

		return std::shared_ptr<CodeProvider>(std::move(ret));
	}

	std::shared_ptr<CodeProvider> FromCodeSet(const Code* codes, size_t codes_length) {
		auto ret = std::make_unique<SimpleCodeProvider>();
		ret->codes = std::vector<Code>(codes, codes + codes_length);

		return std::shared_ptr<CodeProvider>(std::move(ret));
	}

	std::shared_ptr<CodeProvider> FromCodeSet(const Code* codes, size_t codes_length,
		                                      const std::unordered_map<std::string, int>& entrypoints) {
		auto ret = new SimpleCodeProvider;
		ret->codes = std::vector<Code>(codes, codes+codes_length);
		ret->entrypoints = entrypoints;

		return std::shared_ptr<CodeProvider>(ret);
	}

	std::shared_ptr<CodeProvider> FromCodeSet(const std::vector<Code>& codes) {
		auto ret = new SimpleCodeProvider;
		ret->codes = codes;

		return std::shared_ptr<CodeProvider>(ret);
	}

	std::shared_ptr<CodeProvider> FromCodeSet(const std::vector<Code>& codes,
		                                      const std::unordered_map<std::string, int>& entrypoints) {
		auto ret = new SimpleCodeProvider;
		ret->codes = codes;
		ret->entrypoints = entrypoints;

		return std::shared_ptr<CodeProvider>(ret);
	}

	std::shared_ptr<CodeProvider> FromCodeSet(std::vector<Code>&& codes) {
		auto ret = new SimpleCodeProvider;
		ret->codes = std::move(codes);

		return std::shared_ptr<CodeProvider>(ret);
	}

	std::shared_ptr<CodeProvider> FromCodeSet(std::vector<Code>&& codes,
											  std::unordered_map<std::string, int>&& entrypoints) {
		auto ret = std::make_unique<SimpleCodeProvider>();
		ret->codes = std::move(codes);
		ret->entrypoints = std::move(entrypoints);

		return std::shared_ptr<CodeProvider>(std::move(ret));
	}

	std::shared_ptr<CodeProvider> FromCodeSet(const CodeUnit* codes, size_t codes_length) {
		auto ret = std::make_unique<SimpleCodeProvider>();

		const auto* begin = codes;
		const auto* end = codes + codes_length;


		for (auto p = begin; p != end; ++p) {

			if (p->opcode && p->str.size() > 0) {
				// 文字列属性のOpcode
				// 文字列テーブルに追加してインデックスを属性値として使用する
				int index;

				auto it = std::find(ret->stringTable.begin(), ret->stringTable.end(), p->str);
				if (it != ret->stringTable.end()) {
					// すでに同じ文字列が存在する場合は同じインデックスを使う
					index = std::distance(ret->stringTable.begin(), it);
				} else {
					index = (int)ret->stringTable.size();
					ret->stringTable.push_back(p->str);
				}
				ret->codes.emplace_back(p->opcode, index);
			}
			else if (!p->opcode) {
				// エントリポイント
				// codesへの登録はせずエントリポイントの追加のみ行う
				auto index = ret->codes.size();
				ret->entrypoints.emplace(p->str, (int)index);
			}
			else {
				// 通常Opcode
				ret->codes.emplace_back(p->opcode, p->attr);
			}

		}

		return std::shared_ptr<CodeProvider>(std::move(ret));
	}

	

	Generator::Generator() {
		BuildOpcodes(codeMap);
	}

	Code Generator::MakeCode(const std::string& sig, const std::string& attr, std::string& outBindSymbol) {
		auto &sk = codeMap.at(sig);
		Code c;
		c.opcode = sk.opcode;
		switch (sk.type) {
			case AttrType::Integer:
				ParseAttrAsInteger(c, attr);
				break;
			case AttrType::Float:
				ParseAttrAsFloat(c, attr);
				break;
			case AttrType::Comparer:
				ParseAttrAsComparer(c, attr);
				break;
			case AttrType::NumType:
				ParseAttrAsNumType(c, attr);
				break;
			case AttrType::EntryPointSymbol:
				if (!ParseAttrAsInteger(c, attr)) {
					outBindSymbol.assign(attr);
				}
				break;
			case AttrType::Property:
				ParseAttrAsProperty(c, attr);
				break;
			case AttrType::String:
				ParseAttrAsString(c, attr);
				break;
		}

		return c;
	}

	bool Generator::ParseAttrAsInteger(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		size_t szt;
		try {
			c.attr.int_ = std::stoi(attr, &szt, 0); // 基数自動判別
		}
		catch (...) {
			return false;
		}
		return szt != 0;
	}
	bool Generator::ParseAttrAsFloat(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		size_t szt;
		try {
			c.attr.float_ = std::stof(attr, &szt);
		}
		catch (...) {
			return false;
		}
		return szt != 0;
	}
	bool Generator::ParseAttrAsComparer(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		std::initializer_list<std::string> list = { 
			"==", 
			"!=", 
			">", 
			"<=", 
			"<", 
			">=", 
			"and", 
			"nand", 
			"or", 
			"nor", 
			"xor", 
			"nxor" 
		};
		auto it = std::find(list.begin(), list.end(), attr);
		auto dist = std::distance(list.begin(), it);
		c.attr.cmp_ = static_cast<ComparerAttribute>(dist);
		return dist != list.size();
	}
	bool Generator::ParseAttrAsNumType(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		std::initializer_list<std::string> list = {
			"zero", 
			"notzero", 
			"plus", 
			"notplus", 
			"minus", 
			"notminus", 
			"posinf", 
			"notposinf", 
			"neginf", 
			"notneginf", 
			"nan", 
			"notnan" 
		};
		auto it = std::find(list.begin(), list.end(), attr);
		auto dist = std::distance(list.begin(), it);
		c.attr.ntype_ = static_cast<NumTypeAttribute>(dist);
		return dist != list.size();
	}
	bool Generator::ParseAttrAsProperty(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		std::initializer_list<std::string> list = { "get", "set" };
		auto it = std::find(list.begin(), list.end(), attr);
		auto dist = std::distance(list.begin(), it);
		c.attr.prop_ = static_cast<PropertyAttribute>(dist);
		return dist != list.size();
	}

	bool Generator::ParseAttrAsString(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;

		auto a = attr.begin();
		auto b = attr.end() - 1;

		// 左右の空白を除去
		while (std::isspace(*a) && a != b) a++;
		while (std::isspace(*b) && a != b) b--;

		// ダブルクォートまたはシングルクォートを除去
		if (*a == '"' && *b == '"') {
			a++;
			b--;
		}
		else if (*a == '\'' && *b == '\'') {
			a++;
			b--;
		}

		// イテレータが交差してしまった場合は終了
		if (a > b) return false;

		std::string s(a, b + 1);

		// 同じ文字列が局所的に出現するケースの方が多そうな気がするので逆順検索
		auto it = std::find(stringTable.rbegin(), stringTable.rend(), s);
		if (it != stringTable.rend()) {
			// 同一インデックスを返す
			c.attr.str_ = (int)(std::distance(it, stringTable.rend()) - 1);
		}
		else {
			// 末尾追加
			c.attr.str_ = (int)stringTable.size();
			stringTable.push_back(s);
		}
		return true;
	}

	BuilderCore::BuilderCore() {}

	CodeProvider::Ptr BuilderCore::MakeCodeProvider() const {
		auto ret = std::make_unique<SimpleCodeProvider>();

		// TODO: ここに未解決のバインドが残っていたときのエラー処理を記述

		ret->codes = this->code_array;
		ret->entrypoints = this->entrypoints;
		ret->stringTable = this->string_table;

		return CodeProvider::Ptr{ std::move(ret) };
	}

#define MAKEOP(name, op, attr) void BuilderCore::emit_ ## name ( Code::Attribute operand ) { emit_op(op, operand); }
#define MAKEOP_INT(name, op) void BuilderCore::emit_ ## name ( int operand ) { emit_op(op, operand); } void BuilderCore::emit_ ## name () { emit_op(op); }
#define MAKEOP_FLOAT(name, op) void BuilderCore::emit_ ## name ( float operand ) { emit_op(op, operand); }  void BuilderCore::emit_ ## name () { emit_op(op); }
#define MAKEOP_CMP(name, op) void BuilderCore::emit_ ## name ( ComparerAttribute operand ) { emit_op(op, operand); }
#define MAKEOP_NT(name, op) void BuilderCore::emit_ ## name ( NumTypeAttribute operand ) { emit_op(op, operand); }
#define MAKEOP_ENTRYPOINT(name, op) void BuilderCore::emit_ ## name ( const std::string& entrypoint_name ) { emit_op(op, entrypoint_name, true); } void BuilderCore::emit_ ## name () { emit_op(op); }
#define MAKEOP_PROP(name, op) void BuilderCore::emit_ ## name ( PropertyAttribute direction ) { emit_op(op, direction); }
#define MAKEOP_STR(name, op) void BuilderCore::emit_ ## name ( const std::string& str ) { emit_op(op, str, false); } void BuilderCore::emit_ ## name () { emit_op(op); }
#define MAKEOP_UNIT(name, op) void BuilderCore::emit_ ## name () { emit_op(op); }

#include "scriptOp.inl"

#undef MAKEOP_UNIT
#undef MAKEOP_STR
#undef MAKEOP_PROP
#undef MAKEOP_ENTRYPOINT
#undef MAKEOP_NT
#undef MAKEOP_CMP
#undef MAKEOP_FLOAT
#undef MAKEOP_INT
#undef MAKEOP

	int BuilderCore::AddString(const std::string& str) {
		auto it = std::find(string_table.rbegin(), string_table.rend(), str);
		if (it != string_table.rend()) {
			// 同一インデックスを返す
			// 同じ文字列が局所的に出現するケースの方が多そうな気がするので逆順検索
			return (int)std::distance(it, string_table.rend());
		}
		else {
			// 末尾追加
			int ret = (int)string_table.size();
			string_table.push_back(str);
			return ret;
		}
	}

	void BuilderCore::AddEntryPoint(const std::string& name, int pos) {
		entrypoints.emplace(name, pos);
	}

	void BuilderCore::emit_op(Opcode op) {
		code_array.emplace_back(Code{ op });
		
	}

	void BuilderCore::emit_op(Opcode op, Code::Attribute attr) {
		code_array.emplace_back(Code{ op, attr });
		
	}

	void BuilderCore::emit_op(Opcode op, const std::string& attr, bool is_entrypoint) {
		if (is_entrypoint) {
			if (attr.empty()) {
				// スタックトップを使用する方式
				// -1(デフォルト値)を使用してコード生成
				code_array.emplace_back(op);
			}
			else {
				// エントリポイント名指定
				auto it = entrypoints.find(attr);
				if (it == entrypoints.end()) {
					// 見つからない: 遅延バインド対象として記憶しておく
					int index = (int)code_array.size();
					code_array.emplace_back(op);

					DifferedBind bind = { index, attr };
					differed_bind_list.push_back(bind);
				}
				else {
					// 見つかった場合そのエントリポイントのインデックスを使用
					code_array.emplace_back(op, it->second);
				}
			}
		}
		else {
			// 文字列を登録しそれを使用してコード生成
			int str_index = AddString(attr);
			code_array.emplace_back(op, str_index);
		}

		
	}

	void BuilderCore::emit_label(const std::string& label_name) {
		auto index = (int)code_array.size();
		entrypoints.emplace(label_name, index);

		for (auto it = differed_bind_list.begin(); it != differed_bind_list.end(); /* nop */) {
			if (it->entrypoint_name == label_name) {
				// 遅延バインドの解決
				code_array[it->index].attr = index;
				it = differed_bind_list.erase(it);
			}
			else {
				++it;
			}
		}

		
	}

	void BuilderCore::emit_checkpoint(int checkpoint_id) {
		code_array.push_back(Code{ nullptr, checkpoint_id });
		
	}


}}

