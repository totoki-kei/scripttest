// ScripTest.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>

#include "Scrip.h"



std::ostream& operator<<(std::ostream& os, const Scrip::TokenValue& value) {
	class CoutVisitor {
	public:
		void operator()(const Scrip::EvalValue& value) {
			std::cout << value;
		}
		void operator()(const Scrip::EvalValueList& value) {
			std::cout << "[";
			for (const auto& v : value) {
				std::cout << v << ",";
			}
			std::cout << "]";
		}
		void operator()(const Scrip::String& value) {
			std::cout << "'" << value << "'";
		}
	};

	std::visit(CoutVisitor(), value);
	return os;
};


int main()
{
	using namespace Scrip;

	std::cout << "Hello World!\n";

	//auto t = Tokenize(R"(if (x == 1) { do_something(); })");

	std::string src = R"(
x = 250;
if (x == 0.0) do_something(x + 1, y, z, 20);
else do_something(x, y, z, 1000);
)";

	Tokenizer<std::string::const_iterator> tokenizer(std::begin(src), std::end(src));

	Environment env;
	SemanticAction sa{ env };
	Parser<ParseValue, SemanticAction> parser(sa);

	env.RegisterMacro("is_zero", [](const EvalValueList& args) {
		return args[0] == 0.0;
	});
	env.RegisterMacro("do_something", [](const EvalValueList& args) {
		double sum = 0.0;
		std::cout << "do_something(";
		for (const auto& v : args) {
			std::cout << v << ",";
			sum += v;
		}
		std::cout << ")" << std::endl;
		return sum;
	});
	env.SetVariableValue("x", 0);
	env.SetVariableValue("y", 5);
	env.SetVariableValue("z", 2);

	Token t = Token::token_error; int i = -1;
	for (;; ) {
		tokenizer.Next(t, i); // eofも含めて読む
		if (i != -1) {
			std::cout << t << "(" << token_label(t) << ") :" << tokenizer.get_value(i) << std::endl;
		}
		else {
			std::cout << t << "(" << token_label(t) << ")" << std::endl;
		}

		bool accepted = false;
		switch (t) {
			case Token::token_number:
				accepted = parser.post(t, std::get<EvalValue>(tokenizer.get_value(i)));
				break;
			case Token::token_ident:
				accepted = parser.post(t, std::get<String>(tokenizer.get_value(i)) );
				break;
			default:
				accepted = parser.post(t, 0.0);
				break;
		}
		if (accepted) {
			ParseValue result;
			if (parser.accept(result)) {
				std::cout << "Accepted." << std::endl;
				
				std::cout << std::get<AstPtr>(result)->to_string() << std::endl;
			}

			auto val = std::get<AstPtr>(result)->eval(env);
			std::cout << "Eval Result: " << val.value << std::endl;
			break;
		}
	}

}
