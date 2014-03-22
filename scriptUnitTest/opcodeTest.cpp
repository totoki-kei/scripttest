#include "stdafx.h"
#include "CppUnitTest.h"

#include <script.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace scriptUnitTest
{		
	TEST_CLASS(OpcodeTest)
	{
	public:

		static ScriptCode N(float value){
			ScriptCode c;
			c.val = value;
			return c;
		}

		static ScriptCode P(ScriptOpcode opcode, short opt = 0){
			ScriptCode c;
			c.flag = 0;
			c.exp = 0xFF;
			c.opid = (unsigned short)opcode;
			c.option = opt;
			return c;
		}

		TEST_METHOD(OpEnd)
		{
			ScriptCode code[] = {
				N(10), N(20), P(OpcodeEnd), N(30),
			};
			ScriptState state(8, 8, 8, 8, code);
			auto ret = state.Run();

			Assert::AreEqual(0, state.callstacktop);
			Assert::AreEqual(2, state.workstacktop);
			Assert::AreEqual((int)ScriptReturnState::Finished, (int)ret);
			Assert::AreEqual((int)ScriptError::ScriptHasFinished, (int)state.errorCode);
			Assert::AreEqual(2, state.codeindex);

		}

	};
}