#pragma once
#ifndef SCRIP_H_
#define SCRIP_H_

#include "ScripTypes.h"
#include "ScripException.h"

#include "ScripAst.h"
#include "ScripEnvironment.h"

namespace Scrip {
	using ParseValue = std::variant<AstPtr, StringName, EvalValue>;
}

// Caper出力のパーサをinclude
#include "ScripParser.h"
#include "ScripTokenizer.h"

namespace Scrip {
	/// <summary>
	/// caper セマンティックアクション クラス
	/// </summary>
	class SemanticAction {
		Environment& parent;

	public:

		SemanticAction(Environment& ev) : parent(ev) {}

		void stack_overflow() { throw CompilationStackOverflowException("Compilation Stack overflow"); }
		void syntax_error() { throw SyntaxErrorException("SYntax Error"); }

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
			return std::make_shared<Ast::Program>();
		}
		
		// Program(Decl}
		AstPtr Program(const AstPtr& prog, const AstPtr & decl) {
			std::cout << "<Program>" << std::endl;
			auto prog_typed = std::dynamic_pointer_cast<Ast::Program>(prog);
			if (auto func_decl = std::dynamic_pointer_cast<Ast::Function>(decl)) {
				prog_typed->functions.push_back(func_decl);
			}
			else if (auto var_decl = std::dynamic_pointer_cast<Ast::VariableDeclaration>(decl)) {
				prog_typed->variables.push_back(var_decl);
			}
			else {
				std::cerr << "Unknown declaration type in Program" << std::endl;
			}

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

		AstPtr AssignStatement(const StringName& lhs, const AstPtr& rhs) {
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
			return std::make_shared<Ast::FlowControl>(expr ? Ast::FlowControl::CONTROL_RETURN : Ast::FlowControl::CONTROL_RETURN_VOID, expr);
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
		AstPtr MakeCall(const StringName& name, const AstPtr& rhs) {
			std::cout << "<MakeCall>" << std::endl;
			return std::make_shared<Ast::Call>(name, rhs);
		}

		// MakeMember(Expr, Ident)
		AstPtr MakeMember(const AstPtr& expr, const StringName& member_name) {
			std::cout << "<MakeMember>" << std::endl;
			return std::make_shared<Ast::BinOp>(member_name, expr, Ast::BinOp::OP_MEMBER);
		}

		// Variable(String)
		AstPtr Variable(const StringName& name) {
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

		// EmptyIdentList
		AstPtr EmptyIdentList() {
			std::cout << "<EmptyIdentList>" << std::endl;
			return std::make_shared<Ast::IdentList>();
		}

		// MakeIdentList(std::Sequence<String>)
		template <template <typename> typename Sequence>
		AstPtr MakeIdentList(Sequence<StringName> names) {
			std::cout << "<MakeIdentList>" << std::endl;
			auto ident_list = std::make_shared<Ast::IdentList>();
			for (const auto& name : names) {
				ident_list->add(name);
			}
			return ident_list;
		}

		// FunctionDeclaration(ident, list)
		AstPtr FunctionDeclaration(const StringName& name, const AstPtr& argslist, const AstPtr& body) {
			std::cout << "<FunctionDeclaration>" << std::endl;
			return std::make_shared<Ast::Function>(name, argslist, body);
		}

		// VariableDeclaration(ident)
		AstPtr VariableDeclaration(const StringName& name, const AstPtr& init_value = AstPtr{}) {
			std::cout << "<VariableDeclaration>" << std::endl;
			return std::make_shared<Ast::VariableDeclaration>(name, init_value);
		}

		// VariableDeclaration(ident, value)
		AstPtr VariableDeclaration(const StringName& name, const EvalValue& init_value) {
			std::cout << "<VariableDeclaration>" << std::endl;
			return std::make_shared<Ast::VariableDeclaration>(name, std::make_shared<Ast::Constant>(init_value));
		}

	};


}

#endif // SCRIP_H_
