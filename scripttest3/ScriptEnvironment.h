#pragma once

#include <map>

namespace Script
{
	class ScriptEnvironment
	{
		float *field;
		int fieldSize;
		std::map<std::wstring, int> fieldName;

		

	public:
		ScriptEnvironment();
		virtual ~ScriptEnvironment();

		
	};


}
