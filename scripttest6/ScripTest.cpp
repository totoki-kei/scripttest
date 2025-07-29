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
		void operator()(const Scrip::StringName& value) {
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
func main () {
	print("Start" + " main");
	i = 0;
	while (i < 10) {
		if (i == 2) {
			i = i + 1;
			continue;
		}
		print(i, hoge(i, i+2, i*2));
		i = i + 1;
	}
	print("End main");
}

func hoge(x, y, z) {

	if (!is_zero(x)) {
		print(x);
	}
	if (!is_zero(y)) {
		print(y);
	}
	return x + y + z;
}

)";

	// ソースのトークン化
	Tokenizer<std::string::const_iterator> tokenizer(std::begin(src), std::end(src));

	// 実行環境
	Environment env;
	SemanticAction sa{ env };
	Parser<ParseValue, SemanticAction> parser(sa);

	// マクロの登録
	env.RegisterMacro("is_zero", [](const EvalValueList& args) {
		return args[0] == 0.0;
	});
	env.RegisterMacro("do_something", [](const EvalValueList& args) {
		double sum = 0.0;
		std::cout << "do_something(";
		for (const auto& v : args) {
			std::cout << v << ",";
			sum += v.ToNumber();
		}
		std::cout << ")" << std::endl;
		return sum;
	});
	env.RegisterMacro("print", [](const EvalValueList& args) {
		std::cout << "print(";
		for (const auto& v : args) {
			std::cout << v << ",";
		}
		std::cout << ")" << std::endl;
		return 0.0; // printは値を返さない
	});

	// 変数の登録
	env.SetVariableValue("x", 0.0, Scrip::VAR_MODE_CREATE);
	env.SetVariableValue("y", 5.4, Scrip::VAR_MODE_CREATE);
	env.SetVariableValue("z", 2.25, Scrip::VAR_MODE_CREATE);

	// 解析ループ
	Token t = Token::token_error; int i = -1;
	for (;; ) {
		tokenizer.Next(t, i); // eofも含めて読む
		if (i != -1) {
			// 値付きトークン
			std::cout << t << "(" << token_label(t) << ") :" << tokenizer.GetTokenValue(i) << std::endl;
		}
		else {
			// 値なしトークン
			std::cout << t << "(" << token_label(t) << ")" << std::endl;
		}

		// パーサへpush
		bool accepted = false;
		switch (t) {
			case Token::token_literal:
				accepted = parser.post(t, (tokenizer.GetTokenValue<EvalValue>(i)));
				break;
			case Token::token_ident:
				accepted = parser.post(t, (tokenizer.GetTokenValue<StringName>(i)) );
				break;
			default:
				accepted = parser.post(t, 0.0);
				break;
		}
		// push結果の確認 解析完了時には acceptedがtrueになる
		if (accepted) {
			// 最終結果の取得
			ParseValue result;
			if (parser.accept(result)) {
				std::cout << "Accepted." << std::endl;
				
				std::cout << std::get<AstPtr>(result)->to_string() << std::endl;
			}

			// パーサが返したASTを評価
			std::cout << "********* START EVAL *********" << std::endl;
			auto val = std::get<AstPtr>(result)->eval(env);
			std::cout << "Eval Result: " << val.value << std::endl;
			std::cout << "********* END EVAL *********" << std::endl;
			break;
		}
	}

}
