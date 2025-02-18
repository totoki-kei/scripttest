#pragma once

#include "gdextension_interface.h"

#include <string>
#include <vector>
#include <memory>



namespace ast {
	template <typename T>
	using Ptr = std::shared_ptr<T>;

	/// <summary>
	/// 型情報
	/// </summary>
	struct ValueType {
		// 型ID
		GDExtensionVariantType type_index;
		// クラス名
		std::string class_name;
	};

	struct FieldInfo {
		// フィールドの型ID
		int type_index;
		// フィールドオフセット
		int offset;
	};

	struct MethodInfo {
		// メソッドの戻り値
		ValueType ret_type;

	};

	/// <summary>
	/// Expression戻り値の型情報
	/// </summary>
	struct ExpressionType {
		enum {
			// 値
			ET_VALUE,
			// 変数のサブフィールド
			ET_FIELD,
			// メソッド
			ET_METHOD,
			// プロパティ
			ET_PROPERTY,
			// 定数値
			ET_CONSTANT,
		};

		// 型
		ValueType type;

		// 読み取り可能
		bool is_readable;
		// 書き込み可能
		bool is_writable;


		// 推定された型候補
		std::vector<ExpressionType> estimated_types;
	};

	/// <summary>
	/// Expression定数値
	/// </summary>
	struct ExpressionConstant {
		// 型
		ValueType type;
		// 定数値の文字列表現
		std::string str;
	};

	/// <summary>
	/// 変数情報
	/// </summary>
	struct Variable {
		// 型
		ValueType type;
		// 変数名
		std::string name;
	};




	struct Node {
		virtual ~Node() = 0;
		Ptr<Node> parent;
	};

	struct         ExpressionNode;
	struct  LiteralExpressionNode;
	struct    ConstExpressionNode;
	struct VariableExpressionNode;
	struct OperatorExpressionNode;
	struct     CastExpressionNode;
	struct     CallExpressionNode;
	struct   MemberExpressionNode;

	struct       StatementNode;
	struct  BlockStatementNode;
	struct BranchStatementNode;
	struct   LoopStatementNode;

	struct          DeclNode;
	struct  MetadataDeclNode;
	struct ClassDescDeclNode;
	struct  FunctionDeclNode;
	struct  ConstantDeclNode;
	struct    SignalDeclNode;


#pragma region Expression
	// Expression基底クラス
	struct         ExpressionNode : Node {
		ExpressionType result_type;
	};

	// 型を表すExpression
	// 定数や静的メソッドの参照に使用
	struct     TypeExpressionNode : ExpressionNode {
		// メンバーなし 型自体をresult_typeで表す
	};

	// リテラル値
	struct  LiteralExpressionNode : ExpressionNode {
		ExpressionConstant literal_value;
	};

	// 名前付き定数値
	struct    ConstExpressionNode : ExpressionNode {
		// 定数が属しているクラス名
		std::string class_name;
		// 定数名
		std::string constant_name;
	};

	// メンバー参照
	// ドット(.)による参照全般(フィールド、プロパティ、メソッド参照)
	struct   MemberExpressionNode : ExpressionNode {
		// 値
		Ptr<ExpressionNode> self;
		// メンバー名
		std::string member_name;
	};
	struct OperatorExpressionNode : ExpressionNode {
		Ptr<ExpressionNode> left;
		Ptr<ExpressionNode> right;
		int operator_type;
	};
	struct     CastExpressionNode : ExpressionNode {
		Ptr<ExpressionNode> from;
	};
	struct     CallExpressionNode : ExpressionNode {
		Ptr<ExpressionNode> method;
		std::vector<Ptr<ExpressionNode>> arguments;
	};
#pragma endregion


#pragma region Statement
	struct         StatementNode : Node {
		std::vector< Variable> local_variables;
	};
	struct    BlockStatementNode : StatementNode {};
	struct   BranchStatementNode : StatementNode {
		Ptr<ExpressionNode> condition;
		Ptr<StatementNode> true_statement;
		Ptr<StatementNode> false_statement;
	};
	struct     LoopStatementNode : StatementNode {
		Ptr<ExpressionNode> condition;
		Ptr<StatementNode> inner_statement;
	};
	struct    BreakStatementNode : StatementNode {};
	struct ContinueStatementNode : StatementNode {};
	struct FunctionStatementNode : StatementNode {
		ValueType result_type;
		std::vector<Variable> arguments;
	};
#pragma endregion

#pragma region Decl
	struct          DeclNode : Node {};
	struct  MetadataDeclNode : DeclNode {};
	struct ClassDescDeclNode : DeclNode {
		std::string class_name;
		std::string base_class_name;
	};
	struct  FunctionDeclNode : DeclNode {
		std::string name;
		Ptr<FunctionStatementNode> statement;
	};
	struct  ConstantDeclNode : DeclNode {
		std::string name;
		ExpressionConstant value;
	};
	struct    SignalDeclNode : DeclNode {
		std::string name;
		ValueType result_type;
		std::vector<Variable> arguments;
	};
#pragma endregion

	struct RootNode : Node {
		std::vector<Ptr<DeclNode>> declarations;
	};

}
