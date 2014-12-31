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

			Assert::AreEqual((size_t)0, thread->callstack.size());
			Assert::AreEqual((size_t)2, thread->workstack.size());
			Assert::AreEqual((int)ReturnState::Finished, (int)ret);
			Assert::AreEqual((int)ErrorType::ScriptHasFinished, (int)thread->errorCode);
			Assert::AreEqual(2, thread->codeindex);
		}

		TEST_METHOD(OpStackFrame) {
			Code code[] = {
				N(10), N(20),
				P(opPushSb, 1),

				P(opMul, 2.0f),
				P(opDup, 8),
				P(opPopSb, 3),
				P(opAdds, 3),

				P(opEnd),
			};
			auto cp = Loader::FromCodeSet(std::vector<Code>(std::begin(code), std::end(code)));
			auto state = cp->CreateState();
			auto thread = state->CreateThread();

			auto ret = thread->Run();

			Assert::AreEqual((size_t)0, thread->callstack.size(), L"final : callstack");
			Assert::AreEqual((size_t)1, thread->workstack.size(), L"final : workstack");
			Assert::AreEqual((int)ReturnState::Finished, (int)ret, L"final : returnstate");
			Assert::AreEqual((int)ErrorType::ScriptHasFinished, (int)thread->errorCode, L"final : errortype");
			Assert::AreEqual(130.0f, thread->workstack[0].float_, L"final : stack[0]");

		}
	};
}