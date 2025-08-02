#pragma once

#ifndef SCRIPUTILITY_H_
#define SCRIPUTILITY_H_

#include <unordered_map>
#include <vector>

namespace Scrip {
	// ユーティリティクラス

	template <typename Fn>
	struct ScopeExit {
		Fn fn;
		ScopeExit(Fn&& fn) : fn(std::forward<Fn>(fn)) {}
		~ScopeExit() { fn(); }
	};

	template <typename Enum>
	bool HasFlag(Enum value, Enum flag) {
		return (static_cast<int>(value) & static_cast<int>(flag)) != 0;
	}

	template <typename T>
	using Dictionary = std::unordered_map<StringName, T>;


	using EvalValueList = std::vector<EvalValue>;

}

#endif // SCRIPUTILITY_H_
