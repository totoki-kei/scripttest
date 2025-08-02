#pragma once

#ifndef SCRIPAST_H_
#define SCRIPAST_H_

#include "ScripTypes.h"
#include "ScripException.h"
#include "ScripEnvironment.h"

namespace Scrip {
	struct Ast;
	using AstPtr = std::shared_ptr<Ast>;

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



	struct EvalResult {
		// 評価結果
		EvalValue value;
		// フロー制御の状態
		FlowAction flow;

		EvalResult() = default;

		EvalResult(EvalValue value, FlowAction flow = FlowAction::None)
			: value(value)
			, flow(flow)
		{
		}
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

		struct IdentList;

		struct Function;
		struct VariableDeclaration;

		struct Program;

		virtual ~Ast() = default;
		virtual std::string to_string() const = 0;
		virtual EvalResult eval(Environment& env) const = 0;
	};


	// Programクラスの宣言のみ先に記述する 実装は後で行う
	struct Ast::Program : public Ast {
		friend struct Ast;

		std::vector<AstPtr> functions;
		std::vector<AstPtr> variables;

		std::string to_string() const override;

		EvalResult eval(Environment& env) const override;
		EvalResult EvalFunction(const StringName entry_point, EvalValueList args, Environment& env) const;
		bool CallFunction(const StringName entry_point, EvalValueList args, EvalResult& out_result, Environment& env) const;

	private:
		bool StoreProgramPtrToEnvironment(Environment& env) const;
		void ClearProgramPtrFromEnvironment(Environment& env) const;

	public:
		static const Program* FromEnvironment(Environment& env);
	};


	struct Ast::Constant : public Ast {
		EvalValue value;

		Constant(EvalValue value) : value(value) {}

		std::string to_string() const override {
			return "Constant(" + value.ToString() + ")";
		}
		EvalResult eval(Environment& env) const override {
			return value;
		}
	};

	struct Ast::VarRef : public Ast {
		StringName name;

		VarRef(const StringName& name)
			: name(name) {
		}

		std::string to_string() const override {
			return "VarRef(" + name.s + ")";
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
			EvalResult result{ };
			for (const auto& e : list) {
				result = e->eval(env);
				if (result.flow != FlowAction::None) {
					result.flow = FlowAction::None;
					break;
				}
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
				return EvalValue::ApplyUnaryNegate(expr->eval(env).value);
			case OP_NOT:
				return EvalValue::ApplyUnaryNot(expr->eval(env).value);
			}

			return EvalResult{ nullptr };
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
			, op(op) {
		}

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
				return EvalValue::ApplyBinaryOp<std::plus<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_SUB:
				return EvalValue::ApplyBinaryOpNs<std::minus<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_MUL:
				return EvalValue::ApplyBinaryOpNs<std::multiplies<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_DIV:
				return EvalValue::ApplyBinaryOpNs<std::divides<>>(lhs->eval(env).value, rhs->eval(env).value);

			case OP_EQ:
				return EvalValue::ApplyBinaryOpCmp<std::equal_to<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_DIFFER:
				return EvalValue::ApplyBinaryOpCmp<std::not_equal_to<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_LESS:
				return EvalValue::ApplyBinaryOpCmp<std::less<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_LESSEQ:
				return EvalValue::ApplyBinaryOpCmp<std::less_equal<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_GREATER:
				return EvalValue::ApplyBinaryOpCmp<std::greater<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_GREATEREQ:
				return EvalValue::ApplyBinaryOpCmp<std::greater_equal<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_AND:
				return EvalValue::ApplyBinaryOpLogic<std::logical_and<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_OR:
				return EvalValue::ApplyBinaryOpLogic<std::logical_or<>>(lhs->eval(env).value, rhs->eval(env).value);
			}

			throw RuntimeErrorException("Unknown binary operator");
		}

	};

	struct Ast::Call : public Ast {
		StringName name;
		AstPtr args;

		Call(const StringName& name, const AstPtr& args)
			: name(name)
			, args(args) {
		}

		std::string to_string() const override {
			return "Call(" + name.s + "(" + args->to_string() + "))";
		}

		EvalResult eval(Environment& env) const override {
			EvalValueList arg_values;
			if (auto list = std::dynamic_pointer_cast<Ast::List>(args)) {
				for (const auto& e : list->list) {
					arg_values.push_back(e->eval(env).value);
				}
			}

			auto program = Program::FromEnvironment(env);
			if (program) {
				// 該当する関数があれば呼び出す
				auto ret = EvalResult{ {} };
				if (program->CallFunction(name, arg_values, ret, env)) {
					ret.flow = FlowAction::None;
					return ret;
				}
			}

			return env.CallMacro(name, arg_values);
		}
	};

	struct Ast::Assign : public Ast {
		StringName name;
		AstPtr expr;

		Assign(const StringName& name, const AstPtr& expr)
			: name(name)
			, expr(expr) {
		}

		std::string to_string() const override {
			return "Assign(" + name.s + " = " + expr->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			env.SetVariableValue(name, expr->eval(env).value, StoreModeFlags(VAR_MODE_CREATE | VAR_MODE_UPDATE));
			return { nullptr };
		}

	};

	struct Ast::Branch : public Ast {
		AstPtr expr;
		AstPtr true_part;
		AstPtr false_part;

		Branch(const AstPtr& expr, const AstPtr& t_part, const AstPtr& f_part)
			: expr(expr)
			, true_part(t_part)
			, false_part(f_part) {
		}

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
			if (expr->eval(env).value.IsTrueValue()) {
				return true_part ? true_part->eval(env) : EvalResult{};
			}
			else {
				return false_part ? false_part->eval(env) : EvalResult{};
			}
		}
	};

	struct Ast::Loop : public Ast {
		AstPtr expr;
		AstPtr stmt;

		Loop(const AstPtr& expr, const AstPtr& stmt)
			: expr(expr)
			, stmt(stmt) {
		}

		std::string to_string() const override {
			return "Loop(" + expr->to_string() + " : " + stmt->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			EvalResult result{ };
			bool continue_flag = false;

			do {
				continue_flag = false;
				while (expr->eval(env).value.IsTrueValue()) {
					result = stmt->eval(env);
					auto flow_state = result.flow;
					if (flow_state == FlowAction::Break) {
						// loop break;
						result.flow = FlowAction::None; // flow stateをリセット
						return result;
					}
					else if (flow_state == FlowAction::Return) {
						return result; // return値をそのまま返す
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
			, expr(expr) {
		}

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
				return { nullptr, FlowAction::Continue };
			case CONTROL_BREAK:
				return { nullptr, FlowAction::Break };
			case CONTROL_RETURN:
				return { expr ? expr->eval(env).value : nullptr, FlowAction::Return };
			case CONTROL_RETURN_VOID:
				return { nullptr, FlowAction::Return };
			}
			return { nullptr };
		}
	};

	struct Ast::IdentList : public Ast {
		std::vector<StringName> names; // 複数の識別子を保持するリスト
		IdentList() = default;
		void add(const StringName& name) {
			names.push_back(name);
		}
		std::string to_string() const override {
			std::string s = "IdentList(";
			for (const auto& name : names) {
				s += name.s + ",";
			}
			s += ")";
			return s;
		}
		EvalResult eval(Environment& env) const override {
			// IdentListを評価してはいけない
			throw RuntimeErrorException("Invalid AST Call");
		}
	};

	struct Ast::Function : public Ast {
		StringName name;
		std::vector<StringName> arg_names; // 引数名のリスト
		AstPtr body;
		Function(const StringName& name, const AstPtr& argslist, const AstPtr& body)
			: name(name)
			, body(body)
		{
			if (auto list = std::dynamic_pointer_cast<Ast::IdentList>(argslist)) {
				for (const auto& arg_name : list->names) {
					arg_names.push_back(arg_name);
				}
			}
			else {
				throw SyntaxErrorException("Invalid Argument Pointer");
			}
		}
		std::string to_string() const override {
			return "Function(" + name.s + " : " + body->to_string() + ")";
		}
		EvalResult eval(Environment& env) const override {
			return eval(EvalValueList(), env);
		}

		EvalResult eval(const EvalValueList& args, Environment& env) const {
			env.PushFrame();
			for (size_t i = 0; i < arg_names.size(); ++i) {
				if (i < args.size()) {
					env.SetVariableValue(arg_names[i], args[i], VAR_MODE_CREATE);
				}
				else {
					env.SetVariableValue(arg_names[i], nullptr, VAR_MODE_CREATE);
				}
			}

			EvalResult ret{ };
			if (auto body_list = std::dynamic_pointer_cast<List>(body)) {
				bool returned = false;
				for (const auto& stmt : body_list->list) {
					ret = stmt->eval(env);
					if (ret.flow == FlowAction::Return) {
						// フロー制御が発生した場合はそのまま返す
						returned = true;
						break;
					}
				}

				// returnが内部で発生しなかった場合は、戻り値なしとする
				if (!returned) {
					ret.value = nullptr;
				}
			}
			else {
				// 単一のステートメントの場合
				ret = body->eval(env);
			}

			env.PopFrame();
			return EvalResult(ret.value); // FlowActionは無視
		}
	};

	struct Ast::VariableDeclaration : public Ast {
		StringName name;
		AstPtr init_value;
		VariableDeclaration(const StringName& name, const AstPtr& init_value = AstPtr())
			: name(name)
			, init_value(init_value) {
		}
		std::string to_string() const override {
			return "Variable(" + name.s + " = " + (init_value ? init_value->to_string() : "undefined") + ")";
		}
		EvalResult eval(Environment& env) const override {
			EvalValue value = init_value ? init_value->eval(env).value : EvalValue{ nullptr };
			env.SetVariableValue(name, value, VAR_MODE_CREATE);
			return value;
		}
	};


	std::string Ast::Program::to_string() const {
		std::string s = "Program(...)"; // 簡易的な出力のみ
		return s;
	}

	EvalResult Ast::Program::eval(Environment& env) const {
		return EvalFunction("main", EvalValueList(), env);
	}

	EvalResult Ast::Program::EvalFunction(const StringName entry_point, EvalValueList args, Environment& env) const {
		if (!StoreProgramPtrToEnvironment(env)) {
			// 設定に失敗 別のプログラムが動作中
			throw RuntimeErrorException("Invalid Program State");
		}

		// variablesを評価して環境に登録
		for (const auto& var : variables) {
			var->eval(env);
		}

		EvalResult ret;
		bool called = CallFunction(entry_point, args, ret, env);

		ClearProgramPtrFromEnvironment(env);

		return ret;
	}

	bool Ast::Program::CallFunction(const StringName entry_point, EvalValueList args, EvalResult& out_result, Environment& env) const {
		// functionsからエントリーポイントの関数を探して実行
		for (const auto& func : functions) {
			if (auto decl = std::dynamic_pointer_cast<Function>(func)) {
				if (decl->name == entry_point) {
					out_result = decl->eval(args, env);
					return true;
				}
			}
		}
		return false;
	}

	bool Ast::Program::StoreProgramPtrToEnvironment(Environment& env) const {
		return env.SetVariableValue("@@program", { const_cast<void*>(static_cast<const void*>(this)) }, StoreModeFlags(VAR_MODE_CREATE | VAR_MODE_GLOBAL));
	}

	void Ast::Program::ClearProgramPtrFromEnvironment(Environment& env) const {
		env.DeleteVariable("@@program", StoreModeFlags(VAR_MODE_GLOBAL));
	}

	const Ast::Program* Ast::Program::FromEnvironment(Environment& env) {
		// 環境からProgramポインタを取得
		EvalValue prog_value = env.GetVariableValue("@@program");
		return static_cast<const Program*>(std::get<void*>(prog_value.value));
	}


}

#endif // SCRIPAST_H_
