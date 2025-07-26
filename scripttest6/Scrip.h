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

	// フロー制御
	enum class FlowAction {
		// フロー制御なし
		None,
		// break(現在のループを終了する)
		Break,
		// continue(現在のループの次のイテレーションに進む)
		Continue,
		// return(現在の関数を終了し、値を返す)
		Return,
	};



	class Environment {
	public:
		using Macro = std::function<EvalValue(const EvalValueList&)>;
		using VariableCallback = std::function<EvalValue(const String&)>;
		using MacroCallback = std::function<EvalValue(const String&, const EvalValueList&)>;


		Environment() = default;
		Environment(const Environment&) = default;
		Environment(Environment&&) = default;

		/// <summary>
		/// 指定された名前と引数リストでマクロを呼び出します。
		/// </summary>
		/// <param name="name">呼び出すマクロの名前。</param>
		/// <param name="args">マクロに渡す引数のリスト。</param>
		/// <returns>マクロが見つかった場合はその評価値。見つからない場合はNaNを返します。</returns>
		EvalValue CallMacro(const String& name, const EvalValueList& args) {
			std::cout << "CallMacro(" << name << ", [";
			for (const auto& v : args) {
				std::cout << v << ",";
			}
			std::cout << "])" << std::endl;
			if (auto it = macro_map.find(name); it != macro_map.end()) {
				return it->second(args);
			}
			if (macro_callback) {
				auto ret = macro_callback(name, args);
				return ret;
			}
			return std::nan("nan");;
		}

		/// <summary>
		/// 指定された変数名に対応する値を取得します。
		/// </summary>
		/// <param name="name">取得したい変数の名前。</param>
		/// <returns>変数名に対応する値。変数が見つからない場合は、コールバックがあればその結果を返し、どちらもなければNaNを返します。</returns>
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

		/// <summary>
		/// マクロを名前で登録します。
		/// </summary>
		/// <typeparam name="Fn">マクロ本体として使用する関数または関数オブジェクトの型。</typeparam>
		/// <param name="name">登録するマクロの名前。</param>
		/// <param name="macro_body">マクロの本体となる関数または関数オブジェクト。</param>
		/// <param name="overwrite">既存のマクロを上書きするかどうか（デフォルトはfalse）。</param>
		/// <returns>マクロの登録に成功した場合はtrue、失敗した場合はfalseを返します。</returns>
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

		/// <summary>
		/// 指定されたマクロ名の登録を解除します。
		/// </summary>
		/// <param name="name">登録解除するマクロの名前。</param>
		void UnregisterMacro(const String& name) {
			macro_map.erase(name);
		}

		/// <summary>
		/// 指定された変数名に値を設定します。
		/// </summary>
		/// <param name="name">値を設定する変数の名前。</param>
		/// <param name="value">変数に設定する値。</param>
		/// <param name="overwrite">既存の変数の値を上書きするかどうかを指定します。デフォルトは false です。</param>
		/// <returns>値の設定に成功した場合は true、失敗した場合は false を返します。</returns>
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

		/// <summary>
		/// 指定された名前の変数を削除します。
		/// </summary>
		/// <param name="name">削除する変数の名前。</param>
		/// <returns>変数が削除された場合は true、存在しなかった場合は false を返します。</returns>
		bool DeleteVariable(const String& name) {
			return variable_map.erase(name);
		}

		/// <summary>
		/// マクロコールバック関数を登録します。
		/// </summary>
		/// <typeparam name="Fn">コールバック関数の型。</typeparam>
		/// <param name="callback">登録するコールバック関数。</param>
		/// <remarks>ここで登録されたコールバック関数は、 CallMacro が呼び出されたときに該当するマクロが登録されていない場合に呼び出されます。</remarks>
		template <typename Fn>
		void RegisterMacroCallback(Fn callback) {
			macro_callback = callback;
		}

		/// <summary>
		/// 変数コールバック関数を登録します。
		/// </summary>
		/// <typeparam name="Fn">コールバック関数の型。任意の呼び出し可能オブジェクトを指定できます。</typeparam>
		/// <param name="callback">登録するコールバック関数。変数の変更時などに呼び出されます。</param>
		/// <remarks>ここで登録されたコールバック関数は、 GetVariableValue が呼び出されたときに該当する変数が登録されていない場合に呼び出されます。</remarks>
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

	struct EvalResult {
		// 評価結果
		EvalValue value;
		// フロー制御の状態
		FlowAction flow;

		EvalResult(EvalValue value, FlowAction flow = FlowAction::None)
			: value(value)
			, flow(flow)
		{}
	};

#pragma region Ast

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
		virtual EvalResult eval(Environment& env) const = 0;
	};

	struct Ast::Constant : public Ast {
		EvalValue value;

		Constant(EvalValue value) : value(value) {}

		std::string to_string() const override {
			return "Constant(" + std::to_string(value) + ")";
		}
		EvalResult eval(Environment& env) const override {
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
		EvalResult eval(Environment& env) const override {
			return env.GetVariableValue(name);
		}
	};

	struct Ast::List : public Ast {
		std::vector<AstPtr> list;

		List() {}

		template <typename Iterator>
		List(Iterator begin, Iterator end) : list() {
			// 上手くいかないのでこうする(Parser::Sequence<>::const_iterator が vectorコンストラクタに渡せない)
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

		EvalResult eval(Environment& env) const override {
			// Listが直接evalされることは通常ない
			EvalResult result = 0.0;
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

		EvalResult eval(Environment& env) const override {
			return expr->eval(env);
		}
	};

	struct Ast::UniOp : public Ast {
		AstPtr expr;
		enum Op {
			OP_NEGATE,
			OP_NOT,
		} op;

		UniOp(const AstPtr& expr, Op op) : expr(expr), op(op) {}

		std::string to_string() const override {
			auto op_name = std::string(1, "-!"[op]);
			return "UniOp[" + op_name + "](" + expr->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			switch (op) {
			case OP_NEGATE:
				return { -expr->eval(env).value };
			case OP_NOT:
				return expr->eval(env).value == 0.0 ? 1.0 : 0.0;
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

			OP_EQ,
			OP_DIFFER,
			OP_LESS,
			OP_LESSEQ,
			OP_GREATER,
			OP_GREATEREQ,

			OP_AND,
			OP_OR,
		} op;

		BinOp(const AstPtr& lhs, const AstPtr& rhs, Op op)
			: lhs(lhs)
			, rhs(rhs)
			, op(op) {}

		std::string to_string() const override {
			const char* op_name_map[] = {
				"+", "-", "*", "/",
				"==", "!=", "<", "<=", ">", ">=",
				"&&", "||"
			};

			return "BinOp[" + std::string(op_name_map[(int)op]) + "](" + lhs->to_string() + "," + rhs->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			switch (op) {
			case OP_ADD:
				return lhs->eval(env).value + rhs->eval(env).value;
			case OP_SUB:
				return lhs->eval(env).value - rhs->eval(env).value;
			case OP_MUL:
				return lhs->eval(env).value * rhs->eval(env).value;
			case OP_DIV:
				return lhs->eval(env).value / rhs->eval(env).value;

			case OP_EQ:
				return lhs->eval(env).value == rhs->eval(env).value ? 1.0 : 0.0;
			case OP_DIFFER:
				return lhs->eval(env).value != rhs->eval(env).value ? 1.0 : 0.0;
			case OP_LESS:
				return lhs->eval(env).value < rhs->eval(env).value ? 1.0 : 0.0;
			case OP_LESSEQ:
				return lhs->eval(env).value <= rhs->eval(env).value ? 1.0 : 0.0;
			case OP_GREATER:
				return lhs->eval(env).value > rhs->eval(env).value ? 1.0 : 0.0;
			case OP_GREATEREQ:
				return lhs->eval(env).value >= rhs->eval(env).value ? 1.0 : 0.0;

			case OP_AND:
				return lhs->eval(env).value != 0.0 && rhs->eval(env).value != 0.0 ? 1.0 : 0.0;
			case OP_OR:
				return lhs->eval(env).value != 0.0 || rhs->eval(env).value != 0.0 ? 1.0 : 0.0;

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

		EvalResult eval(Environment& env) const override {
			EvalValueList arg_values;
			if (auto list = std::dynamic_pointer_cast<Ast::List>(args)) {
				for (const auto& e : list->list) {
					arg_values.push_back(e->eval(env).value);
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

		EvalResult eval(Environment& env) const override {
			return env.SetVariableValue(name, expr->eval(env).value, true);
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

		EvalResult eval(Environment& env) const override {
			if (expr->eval(env).value != 0.0) {
				return true_part ? true_part->eval(env).value : 0.0;
			}
			else {
				return false_part ? false_part->eval(env).value : 0.0;
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

		EvalResult eval(Environment& env) const override {
			EvalResult result = 0.0;
			bool continue_flag = false;

			do {
				continue_flag = false;
				while (expr->eval(env).value != 0.0) {
					result = stmt->eval(env);
					auto flow_state = result.flow;
					if (flow_state == FlowAction::Break) {
						return 0.0;
					}
					else if (flow_state == FlowAction::Return) {
						return { result.value };
					}
					else if (flow_state == FlowAction::Continue) {
						continue_flag = true;
						break;
					}
				}
			} while (continue_flag);

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

		EvalResult eval(Environment& env) const override {
			switch (control) {
			case CONTROL_CONTINUE:
				return { 0.0, FlowAction::Continue };
			case CONTROL_BREAK:
				return { 0.0, FlowAction::Break };
			case CONTROL_RETURN:
				return { expr ? expr->eval(env).value : 0.0, FlowAction::Return };
			case CONTROL_RETURN_VOID:
				return { 0.0, FlowAction::Return };
			}
			return 0.0;
		}
	};

#pragma endregion Ast

	/// <summary>
	/// caper セマンティックアクション クラス
	/// </summary>
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

		// ProgramStart
		AstPtr ProgramStart() {
			std::cout << "<ProgramStart>" << std::endl;
			return std::make_shared<Ast::List>();
		}
		
		// Program(Stmt}
		AstPtr Program(const AstPtr& prog, const AstPtr & stmt) {
			std::cout << "<Program>" << std::endl;
			std::dynamic_pointer_cast<Ast::List>(prog)->list.push_back(stmt);
			return prog;
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

		// MakeNot(Expr)
		AstPtr MakeNot(const AstPtr& expr) {
			std::cout << "<MakeNot>" << std::endl;
			return std::make_shared<Ast::UniOp>(expr, Ast::UniOp::OP_NOT);
		}

		// MakeEqual(Expr, Expr)
		AstPtr MakeEqual(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeEqual>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_EQ);
		}

		// MakeDiffer(Expr, Expr)
		AstPtr MakeDiffer(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeDiffer>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_DIFFER);
		}

		// MakeLess(Expr, Expr)
		AstPtr MakeLess(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeLess>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_LESS);
		}

		// MakeLessEq(Expr, Expr)
		AstPtr MakeLessEq(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeLessEq>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_LESSEQ);
		}

		// MakeGreater(Expr, Expr)
		AstPtr MakeGreater(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeGreater>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_GREATER);
		}

		// MakeGreaterEq(Expr, Expr)
		AstPtr MakeGreaterEq(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeGreaterEq>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_GREATEREQ);
		}

		// MakeAnd(Expr, Expr)
		AstPtr MakeAnd(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeAnd>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_AND);
		}

		// MakeOr(Expr, Expr)
		AstPtr MakeOr(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeOr>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_OR);
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

	/// <summary>
	/// トークナイザー クラス
	/// </summary>
	/// <typeparam name="Iterator">入力イテレーター</typeparam>
	template <typename Iterator>
	class Tokenizer {
		using MatchResult = std::match_results<Iterator>;

		Iterator it;
		Iterator end;
		struct TokenMap {
			// トークンの文字列または正規表現
			const char* str;
			// トークンの種類
			Token token;

			// 正規表現ハンドラ
			// この値が設定されている場合、 str は正規表現として扱われる。
			int(_stdcall* regex_handler)(const MatchResult&, std::vector<TokenValue>&);

			// マッチした文字列を評価しない正規表現ハンドラ
			static int _stdcall EmptyHandler(const MatchResult&, std::vector<TokenValue>&) {
				return -1;
			}
		};

		std::vector<TokenMap> tokens;

		std::unordered_map<TokenMap*, std::regex> regex_cache;


		std::vector<TokenValue> token_values;

	public:
		/// <summary>
		/// イテレータ範囲からトークナイザーを初期化します。
		/// </summary>
		/// <param name="begin">トークナイズ対象となる入力範囲の開始イテレータ。</param>
		/// <param name="end">トークナイズ対象となる入力範囲の終端イテレータ。</param>
		Tokenizer(Iterator begin, Iterator end) : it(begin), end(end) {

			// トークン値の初期化
			tokens = {
				// 2文字演算子
				// (他の記号より優先してマッチング)
				{ "==", token_op_equal },
				{ "!=", token_op_differ },
				{ "<=", token_op_lesseq },
				{ ">=", token_op_greater },
				{ "&&", token_op_and_and },
				{ "||", token_op_or_or },

				// 1文字演算子
				{ "!", token_op_not },
				{ "<", token_op_less },
				{ ">", token_op_greater },
				{ "+", token_op_add },
				{ "-", token_op_sub },
				{ "*", token_op_mul },
				{ "/", token_op_div },
				{ "=", token_op_assign },

				// 1文字トークン
				{ "(", token_paren_open },
				{ ")", token_paren_close },
				{ "{", token_brace_open },
				{ "}", token_brace_close },
				{ "$", token_dollar },
				{ ",", token_comma },
				{ ";", token_semicolon },

				// キーワード
				{"^if\\b", token_kwd_if, TokenMap::EmptyHandler},
				{"^else\\b", token_kwd_else, TokenMap::EmptyHandler},
				{"^while\\b", token_kwd_while, TokenMap::EmptyHandler},
				{"^continue\\b",token_kwd_continue, TokenMap::EmptyHandler},
				{"^break\\b",token_kwd_break, TokenMap::EmptyHandler},
				{"^return\\b",token_kwd_return, TokenMap::EmptyHandler},

				// 識別子
				{
					"^[a-zA-Z_][a-zA-Z0-9_]*",
					token_ident,
					[](const MatchResult& match_result, std::vector<TokenValue>& values) -> int {
						int index = (int)values.size();
						values.push_back(match_result.str());
						return index;
					}
				},

				// 数値リテラル
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

		template <typename T = TokenValue>
		const T& GetTokenValue(int index) const {
			if constexpr (std::is_same_v<T, TokenValue>) {
				return token_values[index];
			}
			else {
				return std::get<T>(token_values[index]);
			}
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
