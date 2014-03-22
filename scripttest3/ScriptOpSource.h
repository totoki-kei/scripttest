#pragma once
namespace Script {
	enum class SourceType
	{
		immidiate,
		field,
		stack,
	};

	struct ScriptOpSource{
		SourceType type;

		union {
			float value;
			int address;
		};
	};

}