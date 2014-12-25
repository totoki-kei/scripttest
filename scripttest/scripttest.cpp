// scripttest.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


#include "script.h"

namespace S = Script;
namespace SL = Script::Loader;

#include <iostream>

S::ReturnState opPrint(S::Thread& th, const S::Code& c) {
	std::cout << "stack trace start" << std::endl;
	for (auto i = (size_t)0; i < th.workstack.size(); i++) {
		std::cout << "\t#" << i << " : " << th.workstack[i] << std::endl;
	}
	std::cout << "stack trace end" << std::endl;

	return S::ReturnState::None;
}

int main(int argc, char* argv[])
{
	SL::Generator gen;
	
	gen.map["print"] = { opPrint, SL::Generator::AttrType::Integer };

	auto prov = SL::Load("script.txt", gen);
	
	auto st = prov->CreateState();
	auto th = st->CreateThread();

	th->Run();

	return 0;
}

