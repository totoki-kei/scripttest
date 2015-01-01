#include "stdafx.h"
#include "CppUnitTest.h"

#include <script.h>
#include <scriptOp.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace scriptUnitTest
{	
	using namespace Script;

	TEST_CLASS(OpcodeTest)
	{
	public:

		static Code N(float value){
			return Code{ opPush, value };
		}

		static Code P(Opcode opcode, int opt = -1) {
			return Code{ opcode, opt };
		}

		static Code P(Opcode opcode, float value) {
			return Code{ opcode, value };
		}

		TEST_METHOD(OpEnd)
		{
			Code code[] = {
				N(10), N(20), P(opEnd), N(30),
			};
			auto cp = Loader::FromCodeSet(std::vector<Code>(std::begin(code), std::end(code)));
			auto state = cp->CreateState();
			auto thread = state->CreateThread();
			auto ret = thread->Run();

			Assert::AreEqual((size_t)0, thread->CallStackSize());
			Assert::AreEqual((size_t)2, thread->WorkStackSize());
			Assert::AreEqual((int)ReturnState::Finished, (int)ret);
			Assert::AreEqual((int)ErrorType::ScriptHasFinished, (int)thread->GetErrorCode());
			Assert::AreEqual(2, thread->GetCodeIndex());
		}

		TEST_METHOD(OpStackFrame) {
			Code code[] = {
				N(10), N(20),   // 10.0     20.0 <
				P(opPushSb, 1), // 10.0 [0] 20.0 <
				P(opMul, 2.0f), // 10.0 [0] 40.0 <
				P(opDup, 8),    // 10.0 [0] 40.0 40.0 40.0 40.0 40.0 40.0 40.0 40.0 <
				P(opAdd, 5.0f), // 10.0 [0] 40.0 40.0 40.0 40.0 40.0 40.0 40.0 45.0 <
				N(100),         // 10.0 [0] 40.0 40.0 40.0 40.0 40.0 40.0 40.0 45.0 100.0 < 
				P(opPopSb, 3),  // 10.0                                   40.0 45.0 100.0 <
				P(opAdds, 3),   // 195.0 <

				P(opEnd),
			};
			auto cp = Loader::FromCodeSet(std::vector<Code>(std::begin(code), std::end(code)));
			auto state = cp->CreateState();
			auto thread = state->CreateThread();

			auto ret = thread->Run();

			Assert::AreEqual((size_t)0, thread->CallStackSize(), L"final : callstack");
			Assert::AreEqual((size_t)1, thread->WorkStackSize(), L"final : workstack");
			Assert::AreEqual((int)ReturnState::Finished, (int)ret, L"final : returnstate");
			Assert::AreEqual((int)ErrorType::ScriptHasFinished, (int)thread->GetErrorCode(), L"final : errortype");
			Assert::AreEqual(195.0f, thread->WorkStackAt(0).float_, L"final : stack[0]");

		}
	};
}