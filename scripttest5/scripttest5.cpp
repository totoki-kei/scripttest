// scripttest5.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>

#include <unordered_map>

int main()
{
    std::cout << "Hello World!\n";
}

struct TypeInfo;
struct MemberInfo;

struct TypeInfo {
	const char* name;
	size_t size;
	bool is_primitive;
	bool has_pointer;

	std::unordered_map<std::string, MemberInfo> members;
};

struct MemberInfo {
	size_t offset;
	TypeInfo* type;
};


union ValueStorage {
	int64_t int_value;
	double float_value;
	void* pointer_value;
};

struct ResultTrait {
	TypeInfo* type;
	bool is_reference;
	bool is_const;
};

struct Expression {
	// Base class for all expressions
	virtual ~Expression() = default;

	// Virtual function to evaluate the expression
	virtual ValueStorage evaluate() = 0;
	// Virtual function to get the type of the expression
	virtual ResultTrait get_type() = 0;
};


struct Add : Expression {
	Expression* left;
	Expression* right;
};

struct Sub : Expression {
	Expression* left;
	Expression* right;
};

struct Mul : Expression {
	Expression* left;
	Expression* right;
};

struct Div : Expression {
	Expression* left;
	Expression* right;
};

struct Call : Expression {
	Expression* left;
	Expression* right;
};
