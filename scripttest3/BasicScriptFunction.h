#pragma once

namespace Script {
	class BasicScriptFunction;
}

#include "ScriptCode.h"
#include "ScriptReturnCode.h"

namespace Script {
	class BasicScriptFunction
	{
		int index;
	public:
		BasicScriptFunction(int index);
		virtual ~BasicScriptFunction();

		ReturnCode operator () (ScriptEnvironment&, ScriptCode&);
	};
}
