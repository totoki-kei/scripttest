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

		TEST_METHOD(OpWait) {
			Code code[] = {
				N(10), P(opWait), N(20), P(opWait, 3), N(30), P(opEnd),
			};
			auto cp = Loader::FromCodeSet(std::vector<Code>(std::begin(code), std::end(code)));
			auto state = cp->CreateState();
			auto thread = state->CreateThread();

			{
				auto ret = thread->Run();
				Assert::AreEqual((size_t)0, thread->CallStackSize());
				Assert::AreEqual((size_t)1, thread->WorkStackSize());
				Assert::AreEqual((int)ReturnState::Wait, (int)ret);
				Assert::AreEqual((int)ErrorType::OK, (int)thread->GetErrorCode());
				Assert::AreEqual(2, thread->GetCodeIndex());
			}

			{
				auto ret = thread->Run();
				Assert::AreEqual((size_t)0, thread->CallStackSize());
				Assert::AreEqual((size_t)2, thread->WorkStackSize());
				Assert::AreEqual((int)ReturnState::Wait, (int)ret);
				Assert::AreEqual((int)ErrorType::OK, (int)thread->GetErrorCode());
				Assert::AreEqual(4, thread->GetCodeIndex());
			}

			{
				auto ret = thread->Run();
				Assert::AreEqual((size_t)0, thread->CallStackSize());
				Assert::AreEqual((size_t)2, thread->WorkStackSize());
				Assert::AreEqual((int)ReturnState::Wait, (int)ret);
				Assert::AreEqual((int)ErrorType::OK, (int)thread->GetErrorCode());
				Assert::AreEqual(4, thread->GetCodeIndex());
			}
			
			{
				thread->Reset(0);
				auto ret = thread->Run(true);
				Assert::AreEqual((size_t)0, thread->CallStackSize());
				Assert::AreEqual((size_t)3, thread->WorkStackSize());
				Assert::AreEqual((int)ReturnState::Finished, (int)ret);
				Assert::AreEqual((int)ErrorType::ScriptHasFinished, (int)thread->GetErrorCode());
				Assert::AreEqual(5, thread->GetCodeIndex());
			}
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

		TEST_METHOD(StringAttribute) {
			std::vector< std::pair<int, const char*> > result;

			std::string source = R"(
st[aaa]
st[bbb]
st["aaa"]
st['ccc"']
st[d d d  ]
/* comment 
  test */
st[ eee ]
st[" f f f"]
/* 日本語 * コメント / テスト */
st[eee]
)";
			Loader::Generator gen;
			gen.map["st"] = { [&result](Thread& th, const Code& c) {
				result.push_back({ c.option, th.GetCodeProvider()->GetString(c.option) });
				return None;
			}, Loader::AttrType::String };
		

			auto cp = Loader::FromString(source, gen);

			Assert::AreEqual((const char*)nullptr, cp->GetString(-1), L"index -1");
			Assert::AreEqual((std::string)"aaa", (std::string)cp->GetString(0), L"index 0");
			Assert::AreEqual((std::string)"bbb", (std::string)cp->GetString(1), L"index 1");
			Assert::AreEqual((std::string)"ccc\"", (std::string)cp->GetString(2), L"index 2");
			Assert::AreEqual((std::string)"d d d", (std::string)cp->GetString(3), L"index 3");
			Assert::AreEqual((std::string)"eee", (std::string)cp->GetString(4), L"index 4");
			Assert::AreEqual((std::string)" f f f", (std::string)cp->GetString(5), L"index 5");
			Assert::AreEqual((const char*)nullptr, cp->GetString(6), L"index 6");

			auto state = cp->CreateState();
			auto thread = state->CreateThread();

			auto ret = thread->Run();

			Assert::AreEqual((size_t)0, thread->CallStackSize(), L"final : callstack");
			Assert::AreEqual((size_t)0, thread->WorkStackSize(), L"final : workstack");
			Assert::AreEqual((int)ReturnState::Finished, (int)ret, L"final : returnstate");
			Assert::AreEqual((int)ErrorType::ScriptHasFinished, (int)thread->GetErrorCode(), L"final : errortype");

			Assert::AreEqual((size_t)8, result.size(), L"Result Size");
			Assert::AreEqual(0, result[0].first, L"Result[0]");
			Assert::AreEqual(1, result[1].first, L"Result[1]");
			Assert::AreEqual(0, result[2].first, L"Result[2]");
			Assert::AreEqual(2, result[3].first, L"Result[3]");
			Assert::AreEqual(3, result[4].first, L"Result[4]");
			Assert::AreEqual(4, result[5].first, L"Result[5]");
			Assert::AreEqual(5, result[6].first, L"Result[6]");
			Assert::AreEqual(4, result[7].first, L"Result[7]");
		}

	};
}