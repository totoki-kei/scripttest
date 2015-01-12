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
					Code c = gen(sig, attr, ep);
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

		auto ret = new SimpleCodeProvider;
		ret->codes = std::move(codes);
		ret->entrypoints = std::move(entrypoints);
		ret->stringTable = std::move(gen.stringTable);

		return std::shared_ptr<CodeProvider>(ret);
	}

	std::shared_ptr<CodeProvider> FromCodeSet(std::vector<Code>&& codes,
											  std::unordered_map<std::string, int>&& entrypoints) {
		auto ret = new SimpleCodeProvider;
		ret->codes = std::move(codes);
		ret->entrypoints = std::move(entrypoints);

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

	std::shared_ptr<CodeProvider> FromCodeSet(const std::vector<Code>& codes) {
		auto ret = new SimpleCodeProvider;
		ret->codes = codes;

		return std::shared_ptr<CodeProvider>(ret);
	}

	Generator::Generator() {
		BuildOpcodes(map);
	}

	Code Generator::operator ()(const std::string& sig, const std::string& attr, std::string& outBindSymbol) {
		auto &sk = map.at(sig);
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


}}

