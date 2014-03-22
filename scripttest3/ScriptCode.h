#pragma once

namespace Script {
	class ScriptCode;
}

#include "ScriptOpSource.h"

#include <functional>


namespace Script {
	typedef std::function<ReturnCode (ScriptEnvironment&, ScriptCode&)> ScriptFunction;

	class ScriptCode
	{
	public:
		ScriptCode();
		virtual ~ScriptCode();

		ScriptFunction func;
		ScriptOpSource src[3];
	};

}