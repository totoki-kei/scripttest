#pragma once

#include <cstdint>

namespace Script {

	class CodeInterpreter {
		// この処理はInterpreterではなくEnvironmentとかそっちの方に集めた方がいい可能性もあるが
		// 取り急ぎここに実装し、必要に応じて別のところに移し替えることにする

		CodeResult Step(const Codeset& codeset, size_t cursor) {

		}
	};


}
