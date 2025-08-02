#pragma once

#ifndef SCRIPEVALVALUE_H_
#define SCRIPEVALVALUE_H_

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <memory>

#include "ScripException.h"
#include "ScripStringValue.h"

// 型情報
namespace Scrip {

	union ValueStorage {
		nullptr_t nil; // nil型
		double number; // 数値型
		struct {
			char* str_ptr; // 文字列型のポインタ(ヌル終端文字配列)
			size_t str_size; // 文字列のサイズ(文字数)
		} str;
		int64_t integer; // 整数型
		struct {
			void* ptr; // 配列型のポインタ
			size_t size; // 配列のサイズ(要素数)
		} array;
		struct {
			void* ptr; // オブジェクト型のポインタ
			size_t type; // オブジェクトの型情報（例えば、クラスのIDなど）
		} object;
		struct {
			void* ptr0;
			void* ptr1;
		} internal;

		uint8_t bytes[2 * sizeof(void*)]; // 内部データ用のバイト配列
	};

	static_assert(sizeof(ValueStorage) == 2 * sizeof(void*), "ValueStorage must be large enough to hold two pointers");

	enum class TypeIndex {
		Nil,        // nil型
		Number,     // 数値型(double)
		String,     // 文字列型(std::string)
		Integer,    // 整数型(int64_t)
		Array,      // 配列型(std::vector<EvalValue>)
		Object,     // オブジェクト型(void*)
		Unknown     // 未知の型
	};

	/*abstract*/ class TypeInfo {
		~TypeInfo() = default;

		// 型の名前の取得
		virtual const StringName& GetTypeName() const = 0;

		// 型の値を指定の型に暗黙的に変換可能かを返す
		virtual bool CanConvertImplicitlyTo(TypeIndex type_index) const = 0;
		// 型の値を指定の型に明示的に変換可能かを返す
		virtual bool CanConvertExplicitlyTo(TypeIndex type_index) const = 0;

		// 指定の型に変換する
		virtual bool ConvertTo(TypeIndex type_index, ValueStorage& storage) const = 0;
	};

	//class NilTypeInfo : public TypeInfo {};
	//class NumberTypeInfo : public TypeInfo {};
	//class StringTypeInfo : public TypeInfo {};
	//class IntegerTypeInfo : public TypeInfo {};
	//class ArrayTypeInfo : public TypeInfo {};
}

namespace Scrip {
	//using EvalValue = std::variant<double, std::string, intptr_t>;
	struct EvalValue {
		std::variant<
			nullptr_t, // 値なし(nil)
			double,    // 数値
			std::string, // 文字列
			int64_t, // 整数型(boolも兼ねる)
			void* // 内部データポインタ
		> value;
		EvalValue() : value(nullptr) {}
		EvalValue(double v) : value(v) {}
		EvalValue(const std::string& v) : value(v) {}
		EvalValue(int64_t v) : value(v) {}
		EvalValue(bool v) : value(v ? 1LL : 0LL) {} // boolをint64_tに変換
		EvalValue(void* v) : value(v) {}

		bool operator==(const EvalValue& other) const {
			return value == other.value;
		}
		friend std::ostream& operator<<(std::ostream& os, const EvalValue& ev) {
			std::visit([&os](auto&& arg) { os << arg; }, ev.value);
			return os;
		}

		bool IsNil() const {
			return std::holds_alternative<nullptr_t>(value);
		}

		std::string ToString() const {
			return std::visit([](auto&& arg) -> std::string {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, nullptr_t>) {
					return "nil"; // nullptrは"nil"として扱う
				}
				else if constexpr (std::is_same_v<T, std::string>) {
					return arg; // 文字列の場合はそのまま
				}
				else if constexpr (std::is_same_v<T, double>) {
					return std::to_string(arg); // 数値の場合は文字列に変換
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					return std::to_string(arg); // 整数型も文字列に変換
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue conversion to string");
				}
				}, value);
		}

		double ToNumber() const {
			return std::visit([](auto&& arg) -> double {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, nullptr_t>) {
					return 0.0; // nullptrは0.0として扱う
				}
				else if constexpr (std::is_same_v<T, std::string>) {
					return std::stod(arg); // 文字列を数値に変換
				}
				else if constexpr (std::is_same_v<T, double>) {
					return arg; // 数値はそのまま
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					return static_cast<double>(arg); // 整数型も数値として扱う
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue conversion to number");
				}
				}, value);
		}

		int64_t ToInt() const {
			return std::visit([](auto&& arg) -> int64_t {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, nullptr_t>) {
					return 0; // nullptrは0として扱う
				}
				else if constexpr (std::is_same_v<T, std::string>) {
					return std::stoll(arg); // 文字列を整数に変換
				}
				else if constexpr (std::is_same_v<T, double>) {
					return static_cast<int64_t>(arg); // 数値は整数に変換
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					return arg; // 整数はそのまま
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue conversion to int");
				}
				}, value);
		}

		bool IsTrueValue() const {
			return std::visit([](auto&& arg) -> bool {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<decltype(arg), nullptr_t>) {
					return false; // nullptrはfalse
				}
				else if constexpr (std::is_same_v<T, std::string>) {
					return !arg.empty(); // 文字列が空でない場合はtrue
				}
				else if constexpr (std::is_same_v<T, double>) {
					return arg != 0.0; // 数値が0でない場合はtrue
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					return arg != 0; // 整数が0でない場合はtrue
				}
				else if constexpr (std::is_same_v<T, void*>) {
					return arg != nullptr; // ポインタがnullptrでない場合はtrue
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue truthiness check");
				}
				}, value);
		}

		template <typename T>
		EvalValue CastTo() const {
			if constexpr (std::is_same_v<T, EvalValue>) {
				return EvalValue{}; // nulloptを返す
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				return EvalValue(ToString());
			}
			else if constexpr (std::is_same_v<T, double>) {
				return EvalValue(ToNumber());
			}
			else if constexpr (std::is_same_v<T, int64_t>) {
				return EvalValue(ToInt());
			}
			else if constexpr (std::is_same_v<T, void*>) {
				// 変換は許可しない 元の値が void* の時だった場合のみ値を返す
				if (std::holds_alternative<void*>(value)) {
					return EvalValue(std::get<void*>(value));
				}
				else {
					throw std::bad_variant_access(); // void* 以外の型からの変換は許可しない
				}
			}
			else {
				throw RuntimeErrorException("Unsupported type for EvalValue cast");
			}
		}

		/*
		* 演算の方針
		* - 文字列は加算のみ対応。左辺値か右辺値のどちらかが文字列だった場合、文字列に揃えて連結する。
		* - 数値は加算、減算、乗算、除算に対応。どちらか一方がdoubleならばdoubleで、両方が整数なら整数のまま演算する。
		* - null型、ポインタ型はあらゆる演算を許可しない。四則演算が行われた場合はランタイムエラーを発生させる。
		*/

		// 二項演算子の適用（全タイプ対象）
		template <typename Op>
		static EvalValue ApplyBinaryOp(const EvalValue& left, const EvalValue& right) {
			return std::visit([&](auto&& arg1, auto&& arg2) -> EvalValue {
				using T1 = std::decay_t<decltype(arg1)>;
				using T2 = std::decay_t<decltype(arg2)>;
				if constexpr (std::is_same_v<T1, std::string> || std::is_same_v<T2, std::string>) {
					return EvalValue(Op()(left.ToString(), right.ToString()));
				}
				else if constexpr (std::is_same_v<T1, double> || std::is_same_v<T2, double>) {
					return EvalValue(Op()(left.ToNumber(), right.ToNumber()));
				}
				else if constexpr (std::is_same_v<T1, int64_t> && std::is_same_v<T2, int64_t>) {
					return EvalValue(Op()(left.ToInt(), right.ToInt()));
				}
				else if constexpr (std::is_same_v<T1, void*> || std::is_same_v<T2, void*>) {
					throw RuntimeErrorException("Cannot perform operation on void* types");
				}
				else if constexpr (std::is_same_v<T1, nullptr_t> || std::is_same_v<T2, nullptr_t>) {
					throw RuntimeErrorException("Cannot perform operation on nil types");
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue binary operation");
				}
				}, left.value, right.value);
		}

		// 二項演算子の適用（文字列を除外）
		template <typename Op>
		static EvalValue ApplyBinaryOpNs(const EvalValue& left, const EvalValue& right) {
			return std::visit([&](auto&& arg1, auto&& arg2) -> EvalValue {
				using T1 = std::decay_t<decltype(arg1)>;
				using T2 = std::decay_t<decltype(arg2)>;
				if constexpr (std::is_same_v<T1, double> || std::is_same_v<T2, double>) {
					return EvalValue(Op()(left.ToNumber(), right.ToNumber()));
				}
				else if constexpr (std::is_same_v<T1, int64_t> && std::is_same_v<T2, int64_t>) {
					return EvalValue(Op()(left.ToInt(), right.ToInt()));
				}
				else if constexpr (std::is_same_v<T1, void*> || std::is_same_v<T2, void*>) {
					throw RuntimeErrorException("Cannot perform operation on void* types");
				}
				else if constexpr (std::is_same_v<T1, nullptr_t> || std::is_same_v<T2, nullptr_t>) {
					throw RuntimeErrorException("Cannot perform operation on nil types");
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue binary operation without string");
				}
				}, left.value, right.value);
		}

		// 二幸演算子の適用（比較演算子）
		template <typename Op>
		static EvalValue ApplyBinaryOpCmp(const EvalValue& left, const EvalValue& right) {
			return std::visit([&](auto&& arg1, auto&& arg2) -> EvalValue {
				using T1 = std::decay_t<decltype(arg1)>;
				using T2 = std::decay_t<decltype(arg2)>;
				if constexpr (std::is_same_v<T1, std::string> || std::is_same_v<T2, std::string>) {
					return EvalValue(Op()(left.ToString(), right.ToString()) ? 1LL : 0LL);
				}
				else if constexpr (std::is_same_v<T1, double> || std::is_same_v<T2, double>) {
					return EvalValue(Op()(left.ToNumber(), right.ToNumber()) ? 1LL : 0LL);
				}
				else if constexpr (std::is_same_v<T1, int64_t> && std::is_same_v<T2, int64_t>) {
					return EvalValue(Op()(left.ToInt(), right.ToInt()) ? 1LL : 0LL);
				}
				else if constexpr (std::is_same_v<T1, void*> || std::is_same_v<T2, void*>) {
					throw RuntimeErrorException("Cannot perform comparison on void* types");
				}
				else if constexpr (std::is_same_v<T1, nullptr_t> || std::is_same_v<T2, nullptr_t>) {
					throw RuntimeErrorException("Cannot perform comparison on nil types");
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue binary comparison operation");
				}
				}, left.value, right.value);
		}

		// 二項演算子の適用（論理演算子）
		template <typename Op>
		static EvalValue ApplyBinaryOpLogic(const EvalValue& left, const EvalValue& right) {
			return std::visit([&](auto&& arg1, auto&& arg2) -> EvalValue {
				return EvalValue(Op()(left.IsTrueValue(), right.IsTrueValue()) ? 1LL : 0LL);
				}, left.value, right.value);
		}

		// 単項演算子は数が少ないため個別実装する

		// 単項 + 演算子
		static EvalValue ApplyUnaryPosite(const EvalValue& val) {
			return val; // 単項 + は値をそのまま返す
		}

		// 単項 - 演算子
		static EvalValue ApplyUnaryNegate(const EvalValue& val) {
			return std::visit([](auto&& arg) -> EvalValue {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, double>) {
					return EvalValue(-arg);
				}
				else if constexpr (std::is_same_v<T, int64_t>) {
					return EvalValue(-arg);
				}
				else if constexpr (std::is_same_v<T, nullptr_t> || std::is_same_v<T, void*>) {
					throw RuntimeErrorException("Cannot apply unary - to nil or void* types");
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue unary - operation");
				}
				}, val.value);
		}

		// 単項 ! 演算子
		static EvalValue ApplyUnaryNot(const EvalValue& val) {
			return EvalValue(!val.IsTrueValue());
		}


	}; // EvalValue

}

#endif // SCRIPEVALVALUE_H_
