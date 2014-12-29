#include "script.h"

#include <unordered_map>
#include <fstream>

//#define SCRIPT_LARGE_EXPR

#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>
#include <boost/fusion/tuple.hpp>

namespace Script { namespace Loader {
	
	class SimpleCodeProvider : public CodeProvider {
	public:

		std::vector<Code> codes;
		std::unordered_map<std::string, int> entrypoints;

		const Code& Get(int index) override {
			return codes[index];
		}

		int Length() override {
			return codes.size();
		}

		int EntryPoint(const char* ent) override {
			return entrypoints[ent];
		}
	};

	std::shared_ptr<CodeProvider> Load(const char* filepath, Generator& gen) {
		std::ifstream file(filepath);
		if (file.bad()) return nullptr;



		std::string wholeText((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		std::vector<Code> codes;
		codes.reserve((size_t)wholeText.size() / 8);
		std::unordered_map<std::string, int> entrypoints;
		std::unordered_multimap<std::string, int> epToSolve;

		int lastLabel = 0;

#ifdef SCRIPT_LARGE_EXPR
		
		using boost::spirit::qi::int_;
		using boost::spirit::qi::float_;
		using boost::spirit::ascii::char_;
		using boost::spirit::ascii::alpha;
		using boost::spirit::ascii::alnum;
		using boost::spirit::qi::lexeme;
		using boost::spirit::qi::phrase_parse;

		auto r
			= ('<' >> lexeme[+alpha] >> '>')
			| float_
			| ('*' >> int_)
			| (lexeme[(alpha | char_("_#$%@")) >> +(alnum | char_("_#$%@"))] >> -('[' >> lexeme[*(char_ - ']')] >> ']'));

		for (auto begin = wholeText.begin(); begin != wholeText.end(); /* nop */) {
			boost::variant <
				std::vector<char>,
				float,
				int,
				boost::fusion::tuple <
				boost::fusion::tuple<char, std::vector<char> >,
				boost::optional < std::vector<char> >
				>
			> result;


			bool succeeded = phrase_parse(begin, wholeText.end(),
										  r,
										  boost::spirit::ascii::space, result);


			if (!succeeded) {
				break;
			}

			if (result.which() == 0) {
				// エントリポイント
				auto& v = boost::get<std::vector<char>>(result);
				std::string name(v.begin(), v.end());
				entrypoints[name] = codes.size();
			}
			if (result.which() == 1) {
				// float
				codes.emplace_back(&Thread::opPush, boost::get<float>(result));
				codes.back().label = lastLabel;
				lastLabel = 0;
			}
			else if (result.which() == 2) {
				// ラベル
				lastLabel = boost::get<int>(result);
			}
			else if (result.which() == 3) {
				// 命令
				auto &info = boost::get < boost::fusion::tuple <
					boost::fusion::tuple<char, std::vector<char> >,
					boost::optional<std::vector<char>>
					>> (result);

				auto sig_0 = boost::fusion::at_c<0>(info);
				auto sig_head = boost::fusion::at_c<0>(sig_0);
				auto sig_trail = boost::fusion::at_c<1>(sig_0);
				auto attr_ = boost::fusion::at_c<1>(info);

				std::string sig;
				sig += sig_head;
				sig.insert(sig.end(), sig_trail.begin(), sig_trail.end());

				std::string attr;
				if (attr_) attr.assign(attr_->begin(), attr_->end());

				std::string ep;
				Code c = gen(sig, attr, ep);

				if (ep.size()) {
					epToSolve.insert({ ep, (int)codes.size() });
				}

				c.label = lastLabel;
				codes.push_back(c);
				lastLabel = 0;
			}
		}

#else
		
		auto end = wholeText.end();
		for (auto begin = wholeText.begin(); begin != end; /* nop */) {
			
			auto &skip = boost::spirit::ascii::space;

			using boost::spirit::qi::phrase_parse;
			namespace Q = boost::spirit::qi;

			float num;
			std::string strlabel;

			if (phrase_parse(begin, end, Q::float_, skip, num)) {
				// float
				codes.emplace_back(&Thread::opPush, num);
				codes.back().label = lastLabel;
				lastLabel = 0;
			}
			else if (phrase_parse(begin, end, ('<' >> Q::lexeme[(Q::alpha | Q::char_("#$%_")) >> *(Q::alnum | Q::char_("#$%_"))] >> '>'), skip, strlabel)) {
				// エントリポイント
				std::string name(strlabel.begin(), strlabel.end());
				entrypoints[name] = codes.size();
			}
			else if (phrase_parse(begin, end, ('*' >> Q::int_), skip, lastLabel)) {
				// ラベル
			}
			else {
				// 命令のはず
				std::string sig, attr;

				if (!phrase_parse(begin, end, Q::lexeme[(Q::alpha | Q::char_("#$%_")) >> *(Q::alnum | Q::char_("#$%_"))], skip, sig)) {
					// 失敗した
					//std::string failed_part;
					//if (std::distance(begin, end) > 8) {
					//	failed_part.assign(begin, begin + 8);
					//	failed_part += "...";
					//}
					//else {
					//	failed_part.assign(begin, end);
					//}
					
					break;
				}

				phrase_parse(begin, end, '[' >> Q::lexeme[*(Q::char_ - ']')] >> ']', skip, attr);

				std::string ep;
				Code c = gen(sig, attr, ep);

				if (ep.size()) {
					epToSolve.insert({ ep, (int)codes.size() });
				}

				c.label = lastLabel;
				codes.push_back(c);
				lastLabel = 0;

			}



		}
#endif
		codes.push_back(Code{ &Thread::opEnd });

		if (epToSolve.size() > 0) {
			int lastSize = epToSolve.size();
			do {
				auto it = epToSolve.begin();
				while (it != epToSolve.end()) {
					auto ep = entrypoints.find(it->first);
					if (ep != entrypoints.end()) {
						codes[it->second].option = ep->second;

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
#define OPMAPI(name, op) \
	map[ #name ] = {&Thread:: op , AttrType::Integer }

#define OPMAPF(name, op) \
	map[ #name ] = {&Thread:: op , AttrType::Float }

#define OPMAP(name, op, attr) \
	map[ #name ] = {&Thread:: op , attr }

		OPMAPI(nop, opNull);

		OPMAPI(wait, opWait);
		OPMAPI(end, opEnd);
		
		OPMAP(goto, opGoto, AttrType::EntryPointSymbol);

		OPMAPI(jump, opJmp);
		OPMAPI(jump_eq, opJeq);
		OPMAPI(jump_neq, opJne);
		OPMAPI(jump_gt, opJgt);
		OPMAPI(jump_geq, opJge);
		OPMAPI(jump_lt, opJlt);
		OPMAPI(jump_leq, opJle);
		OPMAPI(jump_zero, opJz);
		OPMAPI(jump_nonzero, opJnz);
		OPMAPI(jump_pos, opJpos);
		OPMAPI(jump_neg, opJneg);

		OPMAP(cmp, opCmp, AttrType::Comparer);
		OPMAP(chk, opIs, AttrType::SpecialNumbers);

		OPMAPI(fwd, opFwd);
		OPMAPI(rew, opRew);

		OPMAPF(add, opAdd);
		OPMAPI(adds, opAdds);
		OPMAPF(mul, opMul);
		OPMAPI(muls, opMuls);
		OPMAPF(sub, opSub);
		OPMAPF(neg, opNeg);
		OPMAPF(div, opDiv);
		OPMAPF(mod, opMod);
		OPMAPF(sin, opSin);
		OPMAPF(cos, opCos);
		OPMAPF(tan, opTan);
		OPMAPF(atan, opArg);
		OPMAPF(sqrt, opSqrt);
		OPMAPF(pow, opPow);
		OPMAPF(log, opLog);
		OPMAPF(ln, opLog10);
		OPMAPI(len, opLen);

		OPMAPI(get, opLod);
		OPMAPI(set, opSto);
		OPMAPI(vget, opVlod);
		OPMAPI(vset, opVsto);
		OPMAP(n, opSpps, AttrType::SpecialNumbers);

		OPMAPI(dup, opDup);
		OPMAPI(pop, opDel);
		OPMAPI(clear, opCls);

		OPMAP(call, opCall, AttrType::EntryPointSymbol);
		OPMAPI(ret, opRet);

		OPMAP(push, opPush, AttrType::Float);

		OPMAPI(dadd, opNsAdd);
		OPMAPI(dsub, opNsSub);
		OPMAPI(dmul, opNsMul);
		OPMAPI(ddiv, opNsDiv);

#undef OPMAP
#undef OPMAPI

	}

	Code Generator::operator ()(const std::string& sig, const std::string& attr, std::string& outBindSymbol) {
		auto &sk = map.at(sig);
		Code c;
		c.label = 0;
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
			case AttrType::SpecialNumbers:
				ParseAttrAsSpecialNumbers(c, attr);
				break;
			case AttrType::EntryPointSymbol:
				if (!ParseAttrAsInteger(c, attr)) {
					outBindSymbol.assign(attr);
				}
				break;
		}

		return c;
	}

	bool Generator::ParseAttrAsInteger(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		size_t szt;
		try {
			c.option = std::stoi(attr, &szt, 0); // 基数自動判別
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
			c.val = std::stof(attr, &szt);
		}
		catch (...) {
			return false;
		}
		return szt != 0;
	}
	bool Generator::ParseAttrAsComparer(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		std::initializer_list<std::string> list = { "==", "!=", ">", ">=", "<", "<=", "and", "nand", "or", "nor", "xor", "nxor" };
		auto it = std::find(list.begin(), list.end(), attr);
		c.option = std::distance(list.begin(), it);
		return c.option != list.size();
	}
	bool Generator::ParseAttrAsSpecialNumbers(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		std::initializer_list<std::string> list = { "zero", "nonzero", "plus", "notplus", "minus", "notminus", "posinf", "notposinf", "neginf", "notneginf", "nan", "notnan" };
		auto it = std::find(list.begin(), list.end(), attr);
		c.option = std::distance(list.begin(), it);
		return c.option != list.size();
	}
	bool Generator::ParseAttrAsProperty(Code& c, const std::string& attr) {
		if (attr.size() == 0) return false;
		std::initializer_list<std::string> list = { "get", "set" };
		auto it = std::find(list.begin(), list.end(), attr);
		c.option = std::distance(list.begin(), it);
		return c.option != list.size();
	}

}}

