// scripttest.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


#include "script.h"
#include "scriptOp.h"

namespace S = Script;
namespace SL = Script::Loader;

#include <iostream>

S::ReturnState opPrint(S::Thread& th, const S::Code& c) {
	const char* s = nullptr;
	if (c.attr.int_ != -1) {
		s = th.GetCodeProvider()->GetString(c.attr.int_);
	}
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


static const Script::Code codes[]{
	Script::Code{ &Script::opPush, 0 },
	Script::Code{ &Script::opPush, 1 },
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
	Script::Code{ &Script::opDel, 1 },
	Script::Code{ &Script::opAdd, 1.0f },
	Script::Code{ &Script::opWait, 1 },
	// Script::Code{ &opPrint, -1 },
	Script::Code{ &Script::opGoto, 1 },
	// { &Script::opEnd },
};

#undef A100
#undef A10
#undef A

//static const SL::CodeUnit codes2[]{
//	{ "Front" },
//	{ S::opPush, 0 },
//	{ S::opVsto, 1 },
//};

static class Codes3 {
public:
	SL::Builder builder;

	Codes3() {
		// script.txtと同等のコード
		builder
			.push(0).set(1)
			(nullptr, 1)
			.get(1).wait()
			.push(10).push(20).push(30).call("ppp")
			.push(40).push(50).push(60).call("ppp")
			.push(5).add()(opPrint)
			.adds(3).call("ppp")
			.clear()

			.get(1).add(1.0).set(1)
			.get(1).push(3).jump_lt(1).end().rew(1)

			["ppp"]
			(opPrint, "stack trace")
			.ret()

			["speedtest"]
			.push(1)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
				.add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0).add(1.0)
			(opPrint, "result")
			.end()
		;
	}


} codes3;


#include <Windows.h>

template <typename T>
void printT() {
	std::cout << typeid(T).name() << "(size = " << sizeof(T) << ")" << std::endl;
}

int main(int argc, char* argv[])
{
	// 各データ型のサイズ確認
	printT<S::Code>();
	printT<S::State>();
	printT<S::Thread>();
	printT<std::vector<S::Code>>();


	// 文字列からのコード生成に使うジェネレーター
	SL::Generator gen;
	
	// 命令を追加
	gen.codeMap["print"] = { opPrint, SL::AttrType::String };

	auto prov = SL::Load("script.txt", gen); // スクリプトファイルからコード列を取得
	//auto prov = SL::FromCodeSet(codes, _countof(codes)); // Codeの配列からコード列を取得
	//auto prov = codes3.builder.MakeCodeProvider(); // Builderからコード列を取得

	// 状態とスレッドの作成
	auto st = prov->CreateState();
	auto th = st->CreateThread();

	std::cout << "-------- Start --------" << std::endl;
	auto start = GetTickCount64();
	decltype(start) ticks = 0;
	while (auto stat = th->Run()) {
		switch (stat) {
			case S::Wait:
				//std::cout << "-------- Wait --------" << std::endl;
				break;
			case S::Error:
				std::cout << "======== Error (code = " << th->GetErrorCode() << ") ========" << std::endl;
				break;
			case S::Finished:
				std::cout << "-------- Finished --------" << std::endl;
				break;
		}
		if ((ticks = GetTickCount64() - start) > 60 * 1000) {
			
			break;
		}
		if (stat != S::Wait) break;
	}
	std::cout << "** time : " << ticks << " ms" << std::endl;
	Script::Code dummy;
	opPrint(*th, dummy);

	return 0;
}

