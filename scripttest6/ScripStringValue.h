#pragma once

#ifndef SCRIPSTRINGVALUE_H_
#define SCRIPSTRINGVALUE_H_

#include <string>
#include <unordered_map>
#include <iostream>

namespace Scrip {

	struct StringName {
		std::string s;
		size_t hash;

		StringName() : s(""), hash(0) {}
		StringName(const char* str) : s(str), hash(std::hash<std::string>()(str)) {}
		StringName(const std::string& str) : s(str), hash(std::hash<std::string>()(str)) {}

		bool operator==(const StringName& other) const {
			if (hash != other.hash) return false;
			return s == other.s;
		}

		bool operator < (const StringName& other) const {
			return s < other.s;
		}

		friend std::ostream& operator<<(std::ostream& os, const StringName& str) {
			os << str.s;
			return os;
		}

		// std::string‚Ö‚ÌˆÃ–Ù•ÏŠ·‚ð‹–‰Â
		operator std::string& () {
			return s;
		}

		operator const std::string& () const {
			return s;
		}
	};

}

namespace std {
	template <>
	struct hash<Scrip::StringName> {
		size_t operator()(const Scrip::StringName& str) const noexcept {
			return str.hash;
		}
	};
}

#endif // SCRIPSTRINGVALUE_H_
