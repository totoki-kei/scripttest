// scripttest.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


#include "script.h"

namespace S = Script;
namespace SL = Script::Loader;

#include <iostream>

S::ReturnState opPrint(S::Thread& th, const S::Code& c) {
	std::cout << "stack trace start" << std::endl;
	for (auto i = (size_t)0; i < th.workstack.size(); i++) {
		std::cout << "\t#" << i << " : " << th.workstack[i].float_ << std::endl;
	}
	std::cout << "stack trace end" << std::endl;

	return S::ReturnState::None;
}

#define A Script::Code{ &Script::Thread::opAdd, 1.0f }
#define A10 A, A, A, A, A, A, A, A, A, A
#define A100 A10, A10, A10, A10, A10, A10, A10, A10, A10, A10


std::vector<Script::Code> codes{
	Script::Code{ &Script::Thread::opPush, 0 },
	Script::Code{ &Script::Thread::opPush, 1 },
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	Script::Code{ &Script::Thread::opDel, 1 },
	Script::Code{ &Script::Thread::opAdd, 1.0f },
	Script::Code{ &Script::Thread::opWait, 1 },
	Script::Code{ &Script::Thread::opGoto, 1 },
	Script::Code{ &Script::Thread::opEnd },
};

#undef A100
#undef A10
#undef A

#include <Windows.h>

int main(int argc, char* argv[])
{
	SL::Generator gen;
	
	gen.map["print"] = { opPrint, SL::AttrType::Integer };

	//auto prov = SL::Load("script.txt", gen);
	auto prov = SL::FromCodeSet(codes);
	
	auto st = prov->CreateState();
	auto th = st->CreateThread();

	auto start = GetTickCount();
	while (auto stat = th->Run()) {
		switch (stat) {
			case S::Wait:
				//std::cout << "-------- Wait --------" << std::endl;
				break;
			case S::Error:
				std::cout << "======== Error (code = " << th->errorCode << ") ========" << std::endl;
				break;
			case S::Finished:
				std::cout << "-------- Finished --------" << std::endl;
				break;
		}
		if (GetTickCount() - start > 60 * 1000) break;
		if (stat != S::Wait) break;
	}
	Script::Code dummy;
	opPrint(*th, dummy);

	return 0;
}

