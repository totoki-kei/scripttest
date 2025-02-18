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

	using ParseValue = std::variant<AstPtr, String, EvalValue>;
}

#include "ScripParser.h"

namespace Scrip {

	class Exception {};
	class StackOverflowException : Exception {};
	class SyntaxErrorException : Exception {};

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

		enum FlowControl {
			FLOW_NONE,
			FLOW_BREAK,
			FLOW_CONTINUE,
			FLOW_RETURN,
		};


		Environment() = default;
		Environment(const Environment&) = default;
		Environment(Environment&&) = default;

		EvalValue CallMacro(const String& name, const EvalValueList& args) {
			std::cout << "CallMacro(" << name << ", [";
			for (const auto& v : args) {
				std::cout << v << ",";
			}
			std::cout << "])" << std::endl;
			if (auto it = macro_map.find(name); it != macro_map.end()) {
				return it->second(args);
			}
			return std::nan("nan");;
		}

		EvalValue GetVariableValue(const String& name) {
			std::cout << "GetVariableValue(" << name << ")";
			if (auto it = variable_map.find(name); it != variable_map.end()) {
				std::cout << " = " << it->second << std::endl;
				return it->second;
			}
			if (variable_callback) {
				auto ret = variable_callback(name);
				std::cout << " = " << ret << std::endl;
				return ret;
			}
			std::cout << " = nan" << std::endl;
			return std::nan("nan");
		}

		template <typename Fn>
		bool RegisterMacro(const String& name, Fn macro_body, bool overwrite = false) {
			auto it = macro_map.find(name);
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
			std::cout << "SetVariableValue(" << name << "," << value << ")" << std::endl;
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

		void SetFlowControlState(FlowControl flow) {
			flow_control = flow;
		}

		FlowControl GetFlowControlState() const {
			return flow_control;
		}

		void ClearFlowControlState() {
			flow_control = FLOW_NONE;
		}

	private:
		std::unordered_map<String, Macro> macro_map;
		std::unordered_map<String, EvalValue> variable_map;

		MacroCallback macro_callback;
		VariableCallback variable_callback;
		FlowControl flow_control = FLOW_NONE;
	};


	struct Ast {
		struct Constant;
		struct VarRef;
		struct List;
		struct Identity;
		struct UniOp;
		struct BinOp;
		struct Call;
		struct Assign;


		struct StatementBlock;
		struct Branch;
		struct Loop;


		struct FlowControl;

		virtual ~Ast() = default;
		virtual std::string to_string() const = 0;
		virtual EvalValue eval(Environment& env) const = 0;
	};

	struct Ast::Constant : public Ast {
		EvalValue value;

		Constant(EvalValue value) : value(value) {}

		std::string to_string() const override {
			return "Constant(" + std::to_string(value) + ")";
		}
		EvalValue eval(Environment& env) const override {
			return value;
		}
	};

	struct Ast::VarRef : public Ast {
		String name;
		size_t name_hash;

		VarRef(const String& name)
			: name(name)
			, name_hash(std::hash<String>()(name)) {}

		std::string to_string() const override {
			return "VarRef(" + name + ")";
		}
		EvalValue eval(Environment& env) const override {
			return env.GetVariableValue(name);
		}
	};

	struct Ast::List : public Ast {
		std::vector<AstPtr> list;

		List() {}

		template <typename Iterator>
		List(Iterator begin, Iterator end) : list() {
			// è„éËÇ≠Ç¢Ç©Ç»Ç¢ÇÃÇ≈Ç±Ç§Ç∑ÇÈ(Parser::Sequence<>::const_iterator Ç™ vectorÉRÉìÉXÉgÉâÉNÉ^Ç…ìnÇπÇ»Ç¢)
			for (auto it = begin; it != end; ++it) {
				list.push_back(*it);
			}
		}

		std::string to_string() const override {
			std::string s = "List(";
			for (const auto& e : list) {
				s += e->to_string() + ",";
			}
			s += ")";
			return s;
		}

		EvalValue eval(Environment& env) const override {
			// ListÇ™íºê⁄evalÇ≥ÇÍÇÈÇ±Ç∆ÇÕí èÌÇ»Ç¢
			EvalValue result = 0.0;
			for (const auto& e : list) {
				result = e->eval(env);
			}
			return result;
		}
	};

	struct Ast::Identity : public Ast {
		AstPtr expr;

		Ast::Identity(const AstPtr& expr) : expr(expr) {}

		std::string to_string() const override {
			return "Identity(" + expr->to_string() + ")";
		}

		EvalValue eval(Environment& env) const override {
			return expr->eval(env);
		}
	};

	struct Ast::UniOp : public Ast {
		AstPtr expr;
		enum Op {
			OP_NEGATE,
		} op;

		UniOp(const AstPtr& expr, Op op) : expr(expr), op(op) {}

		std::string to_string() const override {
			return "UniOp(" + expr->to_string() + ")";
		}

		EvalValue eval(Environment& env) const override {
			switch (op) {
			case OP_NEGATE:
				return -expr->eval(env);
			}

			return nan("nan");
		}
	};

	struct Ast::BinOp : public Ast {
		AstPtr lhs;
		AstPtr rhs;
		enum Op {
			OP_ADD,
			OP_SUB,
			OP_MUL,
			OP_DIV,
		} op;

		BinOp(const AstPtr& lhs, const AstPtr& rhs, Op op)
			: lhs(lhs)
			, rhs(rhs)
			, op(op) {}

		std::string to_string() const override {
			auto op_name = std::string(1, "+-*/"[op]);

			return "BinOp[" + op_name + "](" + lhs->to_string() + "," + rhs->to_string() + ")";
		}

		EvalValue eval(Environment& env) const override {
			switch (op) {
			case OP_ADD:
				return lhs->eval(env) + rhs->eval(env);
			case OP_SUB:
				return lhs->eval(env) - rhs->eval(env);
			case OP_MUL:
				return lhs->eval(env) * rhs->eval(env);
			case OP_DIV:
				return lhs->eval(env) / rhs->eval(env);
			}

			return nan("nan");
		}

	};

	struct Ast::Call : public Ast {
		String name;
		size_t name_hash;
		AstPtr args;

		Call(const String& name, const AstPtr& args)
			: name(name)
			, name_hash(std::hash<String>()(name))
			, args(args) {}

		std::string to_string() const override {
			return "Call(" + name + "(" + args->to_string() + "))";
		}

		EvalValue eval(Environment& env) const override {
			EvalValueList arg_values;
			if (auto list = std::dynamic_pointer_cast<Ast::List>(args)) {
				for (const auto& e : list->list) {
					arg_values.push_back(e->eval(env));
				}
			}
			return env.CallMacro(name, arg_values);
		}
	};

	struct Ast::Assign : public Ast {
		String name;
		AstPtr expr;

		Assign(const String& name, const AstPtr& expr)
			: name(name)
			, expr(expr) {}

		std::string to_string() const override {
			return "Assign(" + name + " = " + expr->to_string() + ")";
		}

		EvalValue eval(Environment& env) const override {
			return env.SetVariableValue(name, expr->eval(env), true);
		}

	};

	struct Ast::Branch : public Ast {
		AstPtr expr;
		AstPtr true_part;
		AstPtr false_part;

		Branch(const AstPtr& expr, const AstPtr& t_part, const AstPtr& f_part)
			: expr(expr)
			, true_part(t_part)
			, false_part(f_part) {}

		std::string to_string() const override {
			std::string s = "Branch(" + expr->to_string();
			if (true_part) {
				s += " : " + true_part->to_string();
			}
			if (false_part) {
				s += " : " + false_part->to_string();
			}
			s += ")";
			return s;
		}

		EvalValue eval(Environment& env) const override {
			if (expr->eval(env) != 0.0) {
				return true_part ? true_part->eval(env) : 0.0;
			}
			else {
				return false_part ? false_part->eval(env) : 0.0;
			}
		}
	};

	struct Ast::Loop : public Ast {
		AstPtr expr;
		AstPtr stmt;

		Loop(const AstPtr& expr, const AstPtr& stmt)
			: expr(expr)
			, stmt(stmt) {}

		std::string to_string() const override {
			return "Loop(" + expr->to_string() + " : " + stmt->to_string() + ")";
		}

		EvalValue eval(Environment& env) const override {
			EvalValue result = 0.0;

			auto flow_state = Environment::FLOW_NONE;
			do {
				while (expr->eval(env) != 0.0) {
					result = stmt->eval(env);
					flow_state = env.GetFlowControlState();
					if (flow_state == Environment::FLOW_BREAK) {
						env.ClearFlowControlState();
						return 0.0;
					}
					else if (flow_state == Environment::FLOW_RETURN) {
						env.ClearFlowControlState();
						return result;
					}
					else if (flow_state == Environment::FLOW_CONTINUE) {
						env.ClearFlowControlState();
					}
				}
			} while (flow_state == Environment::FLOW_CONTINUE);

			return result;
		}
	};


	struct Ast::FlowControl : public Ast {
		enum Control {
			CONTROL_CONTINUE,
			CONTROL_BREAK,
			CONTROL_RETURN,
			CONTROL_RETURN_VOID,
		} control;
		AstPtr expr;

		FlowControl(Control control, const AstPtr& expr = AstPtr{})
			: control(control)
			, expr(expr) {}

		std::string to_string() const override {
			const char* flow_control_names[] = {
				"continue",
				"break",
				"return",
				"return void"
			};

			return "FlowControl[" + std::string(flow_control_names[control]) + "](" + expr->to_string() + ")";
		}

		EvalValue eval(Environment& env) const override {
			switch (control) {
			case CONTROL_CONTINUE:
				env.SetFlowControlState(Environment::FLOW_CONTINUE);
				break;
			case CONTROL_BREAK:
				env.SetFlowControlState(Environment::FLOW_BREAK);
				break;
			case CONTROL_RETURN:
				env.SetFlowControlState(Environment::FLOW_RETURN);
				return expr ? expr->eval(env) : 0.0;
			case CONTROL_RETURN_VOID:
				env.SetFlowControlState(Environment::FLOW_RETURN);
				break;
			}
			return 0.0;
		}
	};


	class SemanticAction {
		Environment& parent;

	public:

		SemanticAction(Environment& ev) : parent(ev) {}

		void stack_overflow() { throw StackOverflowException(); }
		void syntax_error() { throw SyntaxErrorException(); }

		template <typename FromT>
		void upcast(ParseValue& to, const FromT& from) {
			to = from;
		}

		template <typename ToT>
		void downcast(ToT& to, const ParseValue& from) {
			to = std::get<ToT>(from);
		}

		AstPtr EmptyStatement() {
			std::cout << "<EmptyStatement>" << std::endl;
			return AstPtr();
		}

		AstPtr ExprStatement(const AstPtr& expr) {
			std::cout << "<ExprStatement>" << std::endl;
			return expr;
		}

		AstPtr AssignStatement(const String& lhs, const AstPtr& rhs) {
			std::cout << "<AssignStatement>" << std::endl;
			return std::make_shared<Ast::Assign>(lhs, rhs);
		}

		//BlockStatement(std::vector<AstPtr>& stmts);
		template <template <typename> typename Sequence>
		AstPtr BlockStatement(const Sequence<AstPtr>& stmts) {
			std::cout << "<BlockStatement>" << std::endl;
			return std::make_shared<Ast::List>(stmts.begin(), stmts.end());
		}

		//IfStatement(Expr, Stmt);
		AstPtr IfStatement(const AstPtr& expr, const AstPtr& stmt) {
			std::cout << "<IfStatement>" << std::endl;
			return std::make_shared<Ast::Branch>(expr, stmt, AstPtr{});

		}

		//IfElseStatement(Expr, Stmt, Stmt);
		AstPtr IfElseStatement(const AstPtr& expr, const AstPtr& stmt_t, const AstPtr& stmt_f) {
			std::cout << "<IfElseStatement>" << std::endl;
			return std::make_shared<Ast::Branch>(expr, stmt_t, stmt_f);
		}

		////IfStatement(Expr, Stmt, Stmt);
		//template<typename OptionalT>
		//AstPtr IfStatement(const AstPtr& expr, const AstPtr& stmt_t, const OptionalT stmt_f) {
		//	std::cout << "<IfElseStatement>" << std::endl;
		//	return std::make_shared<Ast::Branch>(expr, stmt_t, stmt_f ? *stmt_f : AstPtr{});
		//	//return std::make_shared<Ast::Branch>(expr, stmt_t, AstPtr{});
		//}

		//// ElseStatement(Stmt)
		//AstPtr ElseStatement(const AstPtr& stmt) {
		//	std::cout << "<ElseStatement>" << std::endl;
		//	return stmt;
		//}

		//WhileStatement(Expr, Stmt);
		AstPtr WhileStatement(const AstPtr& expr, const AstPtr& stmt) {
			std::cout << "<WhileStatement>" << std::endl;
			return std::make_shared<Ast::Loop>(expr, stmt);
		}

		//ContinueStatement;
		AstPtr ContinueStatement() {
			std::cout << "<ContinueStatement>" << std::endl;
			return std::make_shared<Ast::FlowControl>(Ast::FlowControl::CONTROL_CONTINUE);
		}

		//BreakStatement;
		AstPtr BreakStatement() {
			std::cout << "<BreakStatement>" << std::endl;
			return std::make_shared<Ast::FlowControl>(Ast::FlowControl::CONTROL_BREAK);
		}

		//ReturnStatement;
		//ReturnStatement(Expr)
		AstPtr ReturnStatement(const AstPtr& expr = AstPtr{}) {
			std::cout << "<ReturnStatement>" << std::endl;
			return std::make_shared<Ast::FlowControl>(expr ? Ast::FlowControl::CONTROL_RETURN : Ast::FlowControl::CONTROL_RETURN_VOID);
		}


		// Identity(Expr)
		AstPtr Identity(const AstPtr& expr) {
			std::cout << "<Identity>" << std::endl;
			return expr;
		}

		// Identity(EvalValue)
		AstPtr Identity(const EvalValue& value) {
			std::cout << "<Identity>" << std::endl;
			return std::make_shared<Ast::Constant>(value);
		}

		// MakeAdd(Expr, Expr)
		AstPtr MakeAdd(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeAdd>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_ADD);
		}

		// MakeSub(Expr, Expr)
		AstPtr MakeSub(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeSub>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_SUB);
		}

		// MakeMul(Expr, Expr)
		AstPtr MakeMul(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeMul>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_MUL);
		}

		// MakeDiv(Expr, Expr)
		AstPtr MakeDiv(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeDiv>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_DIV);
		}

		// Negate(Expr)
		AstPtr Negate(const AstPtr& expr) {
			std::cout << "<Negate>" << std::endl;
			return std::make_shared<Ast::UniOp>(expr, Ast::UniOp::OP_NEGATE);
		}

		// MakeCall(Expr, Expr)
		AstPtr MakeCall(const String& name, const AstPtr& rhs) {
			std::cout << "<MakeCall>" << std::endl;
			return std::make_shared<Ast::Call>(name, rhs);
		}

		// Variable(String)
		AstPtr Variable(const String& name) {
			std::cout << "<Variable>" << std::endl;
			return std::make_shared<Ast::VarRef>(name);
		}

		// EmptyList
		AstPtr EmptyList() {
			std::cout << "<EmptyList>" << std::endl;
			return std::make_shared<Ast::List>();
		}

		// MakeList(std::Sequence<Expr>)
		template <template <typename> typename Sequence>
		AstPtr MakeList(Sequence<AstPtr> exprs) {
			std::cout << "<MakeList>" << std::endl;
			return std::make_shared<Ast::List>(exprs.begin(), exprs.end());
		}

	};
#if 0
	TokenList Tokenize(const String& src) {
		TokenList ret;

		using MatchResult = std::match_results<String::const_iterator>;

		struct TokenMap {
			const char* str;
			Token token;
			int(_stdcall* regex_handler)(const MatchResult&, TokenList&);

			static int _stdcall EmptyHandler(const MatchResult&, TokenList&) {
				return -1;
			}
		};

		static TokenMap tokens[] = {
			// 2ï∂éöââéZéqÇ™óDêÊ
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

			if (isspace(*it)) {
				++it;
				continue;
			}

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

			if (token_length == 0) {
				// ÉpÅ[ÉXé∏îsÇÃéûì_Ç≈èIóπÇ∑ÇÈ
				break;
			}
		}



		return ret;
	}
#endif
	template <typename Iterator>
	class Tokenizer {
		using MatchResult = std::match_results<Iterator>;

		Iterator it;
		Iterator end;
		struct TokenMap {
			const char* str;
			Token token;
			int(_stdcall* regex_handler)(const MatchResult&, std::vector<TokenValue>&);
			static int _stdcall EmptyHandler(const MatchResult&, std::vector<TokenValue>&) {
				return -1;
			}
		};

		std::vector<TokenMap> tokens;

		std::unordered_map<TokenMap*, std::regex> regex_cache;


		std::vector<TokenValue> token_values;

	public:
		Tokenizer(Iterator begin, Iterator end) : it(begin), end(end) {

			tokens = {
				// 2ï∂éöââéZéqÇ™óDêÊ
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
				{"^else\\b", token_kwd_else, TokenMap::EmptyHandler},
				{"^while\\b", token_kwd_while, TokenMap::EmptyHandler},
				{"^continue\\b",token_kwd_continue, TokenMap::EmptyHandler},
				{"^break\\b",token_kwd_break, TokenMap::EmptyHandler},
				{"^return\\b",token_kwd_return, TokenMap::EmptyHandler},

				{
					"^[a-zA-Z_][a-zA-Z0-9_]*",
					token_ident,
					[](const MatchResult& match_result, std::vector<TokenValue>& values) -> int {
						int index = (int)values.size();
						values.push_back(match_result.str());
						return index;
					}
				},

				{
					"^([0-9]*[.])?[0-9]+",
					token_number,
					[](const MatchResult& match_result, std::vector<TokenValue>& values) -> int {
						double val = std::stod(match_result.str());
						int index = (int)values.size();
						values.push_back(val);
						return index;
					}
				},
			};

		}

		const TokenValue& get_value(int index) const {
			return token_values[index];
		}

		bool Next(Token& out_token, int& out_value_index) {
			while (it != end && isspace(*it)) {
				++it;
			}
			if (it == end) {
				out_token = token_eof;
				out_value_index = -1;
				return false;
			}
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
					if (std::regex_search(it, end, match_result, it_r->second, std::regex_constants::match_continuous)) {

						token = pattern.token;
						token_length = match_result.length();
						token_index = pattern.regex_handler(match_result, token_values);
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
			out_token = token;
			out_value_index = token_index;
			return true;
		}
	};


}
