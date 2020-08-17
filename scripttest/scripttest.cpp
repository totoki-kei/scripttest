// scripttest.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


#include "script.h"
#include "scriptOp.h"

namespace S = Script;
namespace SL = Script::Loader;

#include <iostream>

S::ReturnState opPrint(S::Thread& th, const S::Code& c) {
	auto s = th.GetCodeProvider()->GetString(c.attr.int_);
	std::cout << "stack trace (" << (s ? s : "(null)") << ") start" << std::endl;
	for (auto i = (size_t)0; i < th.WorkStackSize(); i++) {
		std::cout << "\t#" << i << " : " << th.WorkStackAt(i).float_ << std::endl;
	}
	std::cout << "stack trace (" << (s ? s : "(null)") << ") end" << std::endl;

	return S::ReturnState::None;
}

#define A Script::Code{ &Script::opAdd, 1.0f }
#define A10 A, A, A, A, A, A, A, A, A, A
#define A100 A10, A10, A10, A10, A10, A10, A10, A10, A10, A10


std::vector<Script::Code> codes{
	//Script::Code{ &Script::opPush, 0 },
	//Script::Code{ &Script::opPush, 1 },
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//A100, A100, A100, A100, A100, A100, A100, A100, A100, A100,
	//Script::Code{ &Script::opDel, 1 },
	//Script::Code{ &Script::opAdd, 1.0f },
	//Script::Code{ &Script::opWait, 1 },
	//Script::Code{ &Script::opGoto, 1 },
	Script::Code{ &Script::opEnd },
};

#undef A100
#undef A10
#undef A

#include <Windows.h>

template <typename T>
void printT() {
	std::cout << typeid(T).name() << "(size = " << sizeof(T) << ")" << std::endl;
}

int main(int argc, char* argv[])
{
	printT<S::Code>();
	printT<S::State>();
	printT<S::Thread>();
	printT<std::vector<S::Code>>();

	SL::Generator gen;
	
	gen.map["print"] = { opPrint, SL::AttrType::String };

	auto prov = SL::Load("script.txt", gen);
	//auto prov = SL::FromCodeSet(codes);
	
	auto st = prov->CreateState();
	auto th = st->CreateThread();

	std::cout << "-------- Start --------" << std::endl;
	auto start = GetTickCount();
	decltype(start) ticks;
	while (auto stat = th->Run()) {
		switch (stat) {
			case S::Wait:
				//std::cout << "-------- Wait --------" << std::endl;
				break;
			case S::Error:
				std::cout << "======== Error (code = " << (int)th->GetErrorCode() << ") ========" << std::endl;
				break;
			case S::Finished:
				std::cout << "-------- Finished --------" << std::endl;
				break;
		}
		if ((ticks = GetTickCount() - start) > 60 * 1000) {
			
			break;
		}
		if (stat != S::Wait) break;
	}
	std::cout << "** time : " << ticks << " ms" << std::endl;
	Script::Code dummy;
	opPrint(*th, dummy);

	return 0;
}

