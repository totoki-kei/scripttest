#include "script.h"
#include "scriptOp.h"

#include <unordered_map>
#include <fstream>
#include <locale>
#include <string>

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

	size_t ParseFloat(const char* str, float* outFloat) {
		while (*str != '\0' && isspace(*str)) str++;

		char* end;
		*outFloat = strtof(str, &end);

		return (size_t)(end - str);
	}

	size_t ParseSymbol(const char* str, std::string& outSymbol) {
		auto p = str;

		while (*p != '\0' && isspace(*p)) p++;

		auto begin = p;

		std::function<bool(const char)> checker = [](const char c) { return isalpha(c) || strchr("#$%_", c); };
		while (*p != '\0' && !isspace(*p) && *p != '>' && checker(*p)) {
			p++;
			checker = [](const char c) { return isalnum(c) || strchr("#$%_", c); };
		}

		if (begin != p) {
			outSymbol.assign(begin, p);
			return (size_t)(p - str);
		}

		return 0;
	}

	size_t ParseEntryPoint(const char* str, std::string& outLabel) {
		auto p = str;

		while (*p != '\0' && isspace(*p)) p++;

		if (*p++ != '<') return 0u;
		
		while (*p != '\0' && isspace(*p)) p++;

		if (size_t symbol_len = ParseSymbol(p, outLabel)) {
			p += symbol_len;
			// 閉じ'>'が無かったら失敗
			while (*p != '\0' && isspace(*p)) p++;
			if (*p != '>') return 0;
		}
		else {
			return 0;
		}

		return (size_t)(p + 1 - str);
	}

	size_t ParseLabel(const char* str, int* outLabelId) {
		auto p = str;
		while (*p != '\0' && isspace(*p)) p++;

		if (*p != '*') return 0;
		p++;

		char* end = nullptr;
		*outLabelId = strtol(p, &end, 0);
		if (p == end) {
			return 0;
		}

		return (size_t)(end - str);
	}

	size_t ParseAttribute(const char* str, std::string& outAttr) {
		auto p = str;
		while (*p != '\0' && isspace(*p)) p++;

		if (*p != '[') {
			// 属性開始じゃない
			return 0;
		}
		p++;

		auto start = p;
		while (*p != '\0' && *p != ']') {
			p++;
		}

		if (*p == ']') {
			outAttr.assign(start, p);
			return (size_t)(p + 1 - str);
		}
		
		return 0;
	}

	std::shared_ptr<CodeProvider> FromString(const std::string& source, Generator& gen) {
		std::vector<Code> codes;
		codes.reserve((size_t)source.size() / 8);
		std::unordered_map<std::string, int> entrypoints;
		std::unordered_multimap<std::string, int> epToSolve;

		auto c_start = source.c_str();
		for (auto c = c_start; *c != '\0'; /* nop */) {

			//namespace Q = boost::spirit::qi;


			float num;
			std::string strlabel;
			int intlabel;

			while (*c != '\0' && isspace(*c)) c++;
			if (*c == '\0') break;

			if (size_t len = ParseFloat(c, &num)) {
				// float

				codes.emplace_back(opPush, num);
				c += len;
			}
			else if (size_t len = ParseEntryPoint(c, strlabel)) {
				// エントリポイント

				entrypoints[strlabel] = codes.size();
				c += len;
			}
			else if (size_t len = ParseLabel(c, &intlabel)) {
				// ラベル

				codes.emplace_back(nullptr, intlabel);
				c += len;
			}
			else {
				// 命令のはず

				std::string sig, attr;

				size_t symbol_len = ParseSymbol(c, sig);
				if (symbol_len == 0) {
					// 失敗した
					std::string failed_part;
					int dist = std::distance(c_start, c);
					if (strlen(c) > 12) {
						failed_part.assign(c, c + 8);
						failed_part += "...";
					}
					else {
						failed_part.assign(c);
					}
					throw std::domain_error("cannot parse at [" + failed_part + "] (byte " + std::to_string(dist) + ")");
				}
				c += symbol_len;

				size_t attr_len = ParseAttribute(c, attr);

				std::string ep;
				try {
					Code code = gen(sig, attr, ep);
					if (ep.size()) {
						epToSolve.insert({ ep, (int)codes.size() });
					}

					codes.push_back(code);
					c += attr_len;
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
		while (isspace(*a) && a != b) a++;
		while (isspace(*b) && a != b) b--;

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

