#pragma once

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <regex>

namespace Scrip {
	using String = std::string;
	using EvalValue = double;
	using EvalValueList = std::vector<EvalValue>;

	struct Ast;
	using AstPtr = std::shared_ptr<Ast>;
}

#include "ScripParser.h"

namespace Scrip {

	class Exception { };
	class StackOverflowException : Exception { };
	class SyntaxErrorException : Exception { };

	using TokenValue = std::variant<EvalValue, EvalValueList, String>;
	struct TokenList {
		struct TokenWithValue {
			Token token;
			int index;
		};

		std::vector<TokenWithValue> tokens;
		std::vector<TokenValue> values;
	};

	// *** Mock ***
	//class IEnvironment {
	//	virtual ~IEnvironment() = 0 {};
	//	virtual EvalValue CallMacro(const String& name, const EvalValueList& args) = 0;
	//	virtual EvalValue GetVariableValue(const String& name) = 0;
	//};

	class Environment {
	public:
		using Macro = std::function<EvalValue(const EvalValueList&)>;
		using VariableCallback = std::function<EvalValue(const String&)>;
		using MacroCallback = std::function<EvalValue(const String&, const EvalValueList&)>;

		Environment() = default;
		Environment(const Environment&) = default;
		Environment(Environment&&) = default;

		EvalValue CallMacro(const String& name, const EvalValueList& args) {
			if (auto it = macro_map.find(name); it != macro_map.end()) {
				return it->second(args);
			}
			return std::nan("nan");;
		}

		EvalValue GetVariableValue(const String& name) {
			if (auto it = variable_map.find(name); it != variable_map.end()) {
				return it->second;
			}
			if (variable_callback) {
				return variable_callback(name);
			}
			return std::nan("nan");
		}

		template <typename Fn>
		bool RegisterMacro(const String& name, Fn macro_body, bool overwrite = false) {
			auto it = macro_map.lower_bound(name);
			if (it == macro_map.end() || it->first != name) {
				// insert
				macro_map.insert(it, std::make_pair(name, Macro(macro_body)));
				return true;
			}
			else if (overwrite) {
				it->second = Macro(macro_body);
				return true;
			}
			return false;
		}

		void UnregisterMacro(const String& name) {
			macro_map.erase(name);
		}

		bool SetVariableValue(const String& name, EvalValue value, bool overwrite = false) {
			auto it = variable_map.find(name);
			if (it == variable_map.end() || it->first != name) {
				// insert
				variable_map.insert(std::make_pair(name, value));
				return true;
			}
			else if (overwrite) {
				it->second = value;
				return true;
			}
			return false;
		}

		bool DeleteVariable(const String& name) {
			return variable_map.erase(name);
		}

		template <typename Fn>
		void RegisterMacroCallback(Fn callback) {
			macro_callback = callback;
		}

		template <typename Fn>
		void RegisterVariableCallback(Fn callback) {
			variable_callback = callback;
		}

	private:
		std::unordered_map<String, Macro> macro_map;
		std::unordered_map<String, EvalValue> variable_map;

		MacroCallback macro_callback;
		VariableCallback variable_callback;
	};


	struct Ast {
		struct Constant;
		struct VarRef;
		struct List;
		struct Identity;
		struct UniOp;
		struct BinOp;
		struct Assign;
		struct Branch;
		struct Loop;
		struct FlowControl;
	};

	struct Ast::Constant : public Ast {
		EvalValue value;
	};

	struct Ast::VarRef : public Ast {
		String name;
		size_t name_hash;
	};

	struct Ast::List : public Ast {
		std::vector<AstPtr> list;
	};


	class SemanticAction {
		Environment& parent;

	public:

		SemanticAction(Environment& ev) : parent(ev) {}

		void stack_overflow() { throw StackOverflowException(); }
		void syntax_error() { throw SyntaxErrorException(); }

		template <typename FromT>
		void upcast(TokenValue& to, const FromT& from) {
			to = from;
		}

		template <typename ToT>
		void downcast(ToT& to, const TokenValue& from) {
			to = std::get<ToT>(from);
		}

		template <typename ContainerT>
		AstPtr MakeList(const ContainerT& list) {

		}

		AstPtr Identity(const AstPtr& p) {
			return p;
		}

	};

	TokenList Tokenize(const String& src) {
		TokenList ret;

		using MatchResult = std::match_results<String::const_iterator>;

		struct TokenMap {
			const char* str;
			Token token;
			int (_stdcall *regex_handler)(const MatchResult&, TokenList&);

			static int _stdcall EmptyHandler(const MatchResult&, TokenList&) {
				return -1;
			}
		};
		
		static TokenMap tokens[] = {
			// 2文字演算子が優先
			{ "==", token_op_equal },
			{ "!=", token_op_differ },
			{ "<=", token_op_lesseq },
			{ ">=", token_op_greater },
			{ "<", token_op_less },
			{ ">", token_op_greater },
			{ "+", token_op_add },
			{ "-", token_op_sub },
			{ "*", token_op_mul },
			{ "/", token_op_div },
			{ "=", token_op_assign },

			{ "(", token_paren_open },
			{ ")", token_paren_close },
			{ "{", token_brace_open },
			{ "}", token_brace_close },
			{ "$", token_dollar },
			{ ",", token_comma },
			{ ";", token_semicolon },

			{"^if\\b", token_kwd_if, TokenMap::EmptyHandler},
			{"^while\\b", token_kwd_while, TokenMap::EmptyHandler},
			{"^continue\\b",token_kwd_continue, TokenMap::EmptyHandler},
			{"^break\\b",token_kwd_break, TokenMap::EmptyHandler},
			{"^return\\b",token_kwd_return, TokenMap::EmptyHandler},

			{
				"^[a-zA-Z_][a-zA-Z0-9_]*",
				token_ident,
				[](const MatchResult& match_result, TokenList& ret) -> int {
					int index = (int)ret.values.size();
					ret.values.push_back(match_result.str());
					return index;
				}
			},

			{
				"^([0-9]*[.])?[0-9]+",
				token_number,
				[](const MatchResult& match_result, TokenList& ret) -> int {
					double val = std::stod(match_result.str());
					int index = ret.values.size();
					ret.values.push_back(val);
					return index;
				}
			},
		};

		static std::unordered_map<TokenMap*, std::regex> regex_cache;

		for (auto it = src.begin(); it != src.end(); /* nop */) {

			Token token = token_error;
			size_t token_length = 0;
			int token_index = -1;

			for (auto& pattern : tokens) {
				if (pattern.regex_handler) {
					auto it_r = regex_cache.find(&pattern);
					if (it_r == regex_cache.end()) {
						auto insert_result = regex_cache.insert({ &pattern, std::regex{ pattern.str } });
						it_r = insert_result.first;
					}

					MatchResult match_result;
					if (std::regex_search(it, src.end(), match_result, it_r->second)) {
						token = pattern.token;
						token_length = match_result.length();
						token_index = pattern.regex_handler(match_result, ret);
						break;
					}
				}
				else {
					size_t len = strlen(pattern.str);
					const auto ptr = &*it;
					if (strncmp(ptr, pattern.str, len) == 0) {
						token = pattern.token;
						token_length = len;
						token_index = -1;
						break;
					}
				}
			}

			it += token_length;
			ret.tokens.push_back({ token, token_index });

			if (token_length > 0) {
				// パース失敗の時点で終了する
				break;
			}
		}



		return ret;
	}



	//EvalValue Eval(Environment& env, const TokenList& src) {
	//	SemanticAction sa(env);
	//	Scrip::Parser<TokenValue, SemanticAction> parser(sa);

	//	for (const auto& t : src.tokens) {
	//		if (parser.post(t.token, t.index < 0 ? TokenValue(std::nan("nan")) : src.values[t.index])) break;
	//	}

	//	TokenValue result;
	//	if (parser.accept(result)) {
	//		return std::get<EvalValue>(result);
	//	}

	//	return std::nan("nan");
	//}

	//EvalValue Eval(Environment& env, const String& src) {
	//	return Eval(env, Tokenize(src));
	//}


}
