#include "script.h"

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

		const Code& Get(int index) override {
			return codes[index];
		}

		int EntryPoint(const char* ent) override {
			return entrypoints[ent];
		}
	};

	std::shared_ptr<CodeProvider> Load(const char* filepath, Generator& gen) {
		std::ifstream file(filepath);

		using boost::spirit::qi::int_;
		using boost::spirit::qi::float_;
		using boost::spirit::ascii::char_;
		using boost::spirit::ascii::alpha;
		using boost::spirit::ascii::alnum;
		using boost::spirit::qi::lexeme;
		using boost::spirit::qi::phrase_parse;


		std::string wholeText((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		std::vector<Code> codes((size_t)wholeText.size() / 8);
		std::unordered_map<std::string, int> entrypoints;

		auto r
			= ('<' >> lexeme[+alpha] >> '>')
			| float_
			| ('*' >> int_)
			| (lexeme[(alpha | char_("_#$%@")) >> +(alnum | char_("_#$%@"))] >> -('[' >> lexeme[*(char_ - ']')] >> ']'));

		int lastLabel = 0;
		for (auto begin = wholeText.begin(); begin != wholeText.end(); /* nop */ ) {
			boost::variant <
				std::vector<char>,
				float,
				int,
				boost::fusion::tuple <
					boost::fusion::tuple<char, std::vector<char> >,
					boost::optional<std::vector<char>>
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
					>>(result);

				auto sig_0 = boost::fusion::at_c<0>(info);
				auto sig_head = boost::fusion::at_c<0>(sig_0);
				auto sig_trail = boost::fusion::at_c<1>(sig_0);
				auto attr_ = boost::fusion::at_c<1>(info);

				std::string sig;
				sig += sig_head;
				sig.insert(sig.end(), sig_trail.begin(), sig_trail.end());

				std::string attr;
				if (attr_) attr.assign(attr_->begin(), attr_->end());

				Code c = gen(sig, attr);
				c.label = lastLabel;
				codes.push_back(c);
				lastLabel = 0;
			}
		}

		auto ret = new SimpleCodeProvider;
		ret->codes = std::move(codes);
		ret->entrypoints = std::move(entrypoints);

		return std::shared_ptr<CodeProvider>(ret);
	}

	Generator::Generator() {
#define OPMAPI(name, op) \
	map[ #name ] = {ToOpcode(&Thread:: op ) , AttrType::Integer }

#define OPMAP(name, op, attr) \
	map[ #name ] = {ToOpcode(&Thread:: op ) , attr }

		OPMAPI(nop, opNull);

		OPMAPI(wait, opWait);
		OPMAPI(end, opEnd);
		
		OPMAPI(goto, opGoto);

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

		OPMAPI(add, opAdd);
		OPMAPI(adds, opAdds);
		OPMAPI(mul, opMul);
		OPMAPI(muls, opMuls);
		OPMAPI(sub, opSub);
		OPMAPI(neg, opNeg);
		OPMAPI(div, opDiv);
		OPMAPI(mod, opMod);
		OPMAPI(sin, opSin);
		OPMAPI(cos, opCos);
		OPMAPI(tan, opTan);
		OPMAPI(atan, opArg);
		OPMAPI(sqrt, opSqrt);
		OPMAPI(pow, opPow);
		OPMAPI(log, opLog);
		OPMAPI(len, opLen);

		OPMAPI(get, opLod);
		OPMAPI(set, opSto);
		OPMAPI(vget, opVlod);
		OPMAPI(vset, opVsto);
		OPMAP(n, opSpps, AttrType::SpecialNumbers);

		OPMAPI(dup, opDup);
		OPMAPI(pop, opDel);
		OPMAPI(clear, opCls);

		OPMAPI(call, opCall);
		OPMAPI(ret, opRet);

		OPMAP(push, opPush, AttrType::Float);

		OPMAPI(dadd, opNsAdd);
		OPMAPI(dsub, opNsSub);
		OPMAPI(dmul, opNsMul);
		OPMAPI(ddiv, opNsDiv);

#undef OPMAP
#undef OPMAPI

	}

	Code Generator::operator ()(const std::string& sig, const std::string& attr) {
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
		}

		return c;
	}

	void Generator::ParseAttrAsInteger(Code& c, const std::string& attr) {
		c.option = std::stoi(attr, nullptr, 0); // 基数自動判別
	}
	void Generator::ParseAttrAsFloat(Code& c, const std::string& attr) {
		c.val = std::stof(attr);
	}
	void Generator::ParseAttrAsComparer(Code& c, const std::string& attr) {
		std::initializer_list<std::string> list = { "==","!=",">",">=","<","<=","and","nand","or","nor","xor","nxor" };
		auto it = std::find(list.begin(), list.end(), attr);
		c.option = std::distance(list.begin(), it);
	}
	void Generator::ParseAttrAsSpecialNumbers(Code& c, const std::string& attr) {
		std::initializer_list<std::string> list = { "zero", "nonzero", "plus", "notplus", "minus", "notminus", "posinf", "notposinf", "neginf", "notneginf", "nan", "notnan" };
		auto it = std::find(list.begin(), list.end(), attr);
		c.option = std::distance(list.begin(), it);
	}

}}

