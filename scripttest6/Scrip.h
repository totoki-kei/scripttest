#pragma once

#include <string>
#include <vector>
#include <variant>
#include <functional>
#include <regex>

namespace {
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
}

/*************************************************/
#define DECLARE_EXCEPTION(name, base)             \
	class name : public base {                    \
		public:                                   \
		name(std::string_view msg) : base(msg) {} \
	}                                             \
/*************************************************/

namespace Scrip {
	class Exception : std::runtime_error {
		std::string message;
	public:
		Exception(std::string_view msg) : std::runtime_error(std::string(msg)), message(msg) {}
		const char* what() const noexcept override {
			return message.c_str();
		}
	};


	DECLARE_EXCEPTION(CompileErrorException, Exception);
	DECLARE_EXCEPTION(SyntaxErrorException, CompileErrorException);
	DECLARE_EXCEPTION(CompilationStackOverflowException, CompileErrorException);

	DECLARE_EXCEPTION(RuntimeErrorException, Exception);

}

#undef DECLARE_EXCEPTION

namespace Scrip {
	//using String = std::string;
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

		// std::stringへの暗黙変換を許可
		operator std::string& () {
			return s;
		}

		operator const std::string& () const {
			return s;
		}
	};
	//using EvalValue = std::variant<double, std::string, intptr_t>;
	struct EvalValue{
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
				if constexpr (std::is_same_v<decltype(arg), nullptr_t>) {
					return "nil"; // nullptrは"nil"として扱う
				}
				else if constexpr (std::is_same_v<decltype(arg), std::string>) {
					return arg; // 文字列の場合はそのまま
				}
				else if constexpr (std::is_same_v<decltype(arg), double>) {
					return std::to_string(arg); // 数値の場合は文字列に変換
				}
				else if constexpr (std::is_same_v<decltype(arg), int64_t>) {
					return std::to_string(arg); // 整数型も文字列に変換
				}
				else if constexpr (std::is_same_v<decltype(arg), void*>) {
					return std::to_string(arg); // ポインタ型も数値として扱う
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue conversion to string");
				}
			}, value);
		}

		double ToNumber() const {
			return std::visit([](auto&& arg) -> double {
				if constexpr (std::is_same_v<decltype(arg), nullptr_t>) {
					return 0.0; // nullptrは0.0として扱う
				}
				else if constexpr (std::is_same_v<decltype(arg), std::string>) {
					return std::stod(arg); // 文字列を数値に変換
				}
				else if constexpr (std::is_same_v<decltype(arg), double>) {
					return arg; // 数値はそのまま
				}
				else if constexpr (std::is_same_v<decltype(arg), int64_t>) {
					return static_cast<double>(arg); // 整数型も数値として扱う
				}
				else if constexpr (std::is_same_v<decltype(arg), void*>) {
					return static_cast<double>(arg); // ポインタ型も数値として扱う
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue conversion to number");
				}
			}, value);
		}

		int64_t ToInt() const {
			return std::visit([](auto&& arg) -> int64_t {
				if constexpr (std::is_same_v<decltype(arg), nullptr_t>) {
					return 0; // nullptrは0として扱う
				}
				else if constexpr (std::is_same_v<decltype(arg), std::string>) {
					return std::stoll(arg); // 文字列を整数に変換
				}
				else if constexpr (std::is_same_v<decltype(arg), double>) {
					return static_cast<int64_t>(arg); // 数値は整数に変換
				}
				else if constexpr (std::is_same_v<decltype(arg), int64_t>) {
					return arg; // 整数はそのまま
				}
				else if constexpr (std::is_same_v<decltype(arg), void*>) {
					return reinterpret_cast<int64_t>(arg); // ポインタ型も整数として扱う
				}
				else {
					throw RuntimeErrorException("Unsupported type for EvalValue conversion to int");
				}
			}, value);
		}

		bool IsTrueValue() const {
			return std::visit([](auto&& arg) -> bool {
				if constexpr (std::is_same_v<decltype(arg), nullptr_t>) {
					return false; // nullptrはfalse
				}
				else if constexpr (std::is_same_v<decltype(arg), std::string>) {
					return !arg.empty(); // 文字列が空でない場合はtrue
				}
				else if constexpr (std::is_same_v<decltype(arg), double>) {
					return arg != 0.0; // 数値が0でない場合はtrue
				}
				else if constexpr (std::is_same_v<decltype(arg), int64_t>) {
					return arg != 0; // 整数が0でない場合はtrue
				}
				else if constexpr (std::is_same_v<decltype(arg), void*>) {
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

	using EvalValueList = std::vector<EvalValue>;

	struct Ast;
	using AstPtr = std::shared_ptr<Ast>;

	using ParseValue = std::variant<AstPtr, StringName, EvalValue>;

	template <typename T>
	using Dictionary = std::unordered_map<StringName, T>;
}

namespace std {
	template <>
	struct hash<Scrip::StringName> {
		size_t operator()(const Scrip::StringName& str) const noexcept {
			return str.hash;
		}
	};
}

#include "ScripParser.h"


namespace Scrip {

	using TokenValue = std::variant<EvalValue, EvalValueList, StringName>;
	struct TokenList {
		struct TokenWithValue {
			Token token;
			int index;
		};

		std::vector<TokenWithValue> tokens;
		std::vector<TokenValue> values;
	};

	// *** Mock ***
	//class IEnvironment {
	//	virtual ~IEnvironment() = 0 {};
	//	virtual EvalValue CallMacro(const String& name, const EvalValueList& args) = 0;
	//	virtual EvalValue GetVariableValue(const String& name) = 0;
	//};

	// フロー制御
	enum class FlowAction {
		// フロー制御なし
		None,
		// break(現在のループを終了する)
		Break,
		// continue(現在のループの次のイテレーションに進む)
		Continue,
		// return(現在の関数を終了し、値を返す)
		Return,
	};

	// 変数格納時のモード
	enum StoreModeFlags {
		VAR_MODE_CREATE = 0x01, // 変数が存在しない場合は新規作成する
		VAR_MODE_UPDATE = 0x02, // 変数が存在する場合は上書きする

		VAR_MODE_GLOBAL = 0x10, // グローバル変数として格納する(ローカル変数を参照しない)
		VAR_MODE_LOCAL = 0x20, // ローカル変数として格納する(グローバル変数を参照しない)
		
		VAR_MODE_TOPFRAME = 0x40, // フレームスタックの最上位フレームのみ参照する


		/*
		* VAR_MODE_GLOBAL と VAR_MODE_LOCAL は、一方のみ指定されたときのみ効果を有する。両方指定時とどちらも指定していないときは同じ振る舞いとなる。
		* VAR_MODE_TOPFRAME は、ローカル変数が操作対象である場合にのみ機能する。
		*/
	};

	class StackFrame {
		Dictionary<EvalValue> variable_map;

	public:
		EvalValue GetVariableValue(const StringName& name) {
			if (auto it = variable_map.find(name); it != variable_map.end()) {
				return it->second;
			}
			return { nullptr }; // 変数が見つからない場合はnilを返す
		}

		bool SetVariableValue(const StringName& name, EvalValue value, StoreModeFlags mode) {
			auto it = variable_map.find(name);
			if (it == variable_map.end() || it->first != name) {
				// insert
				if (HasFlag(mode, VAR_MODE_CREATE)) {
					variable_map.insert(std::make_pair(name, value));
					return true;
				}
				else {
					return false;
				}
			}
			else if (HasFlag(mode, VAR_MODE_UPDATE)) {
				it->second = value;
				return true;
			}
			return false;
		}

		bool DeleteVariable(const StringName& name, StoreModeFlags /*mode*/) {
			auto it = variable_map.find(name);
			if (it != variable_map.end()) {
				variable_map.erase(it);
				return true;
			}
			return false;
		}

		bool IsVariableDefined(const StringName& name) const {
			return variable_map.find(name) != variable_map.end();
		}
	};

	class Environment {
	public:
		using Macro = std::function<EvalValue(const EvalValueList&)>;
		using VariableCallback = std::function<EvalValue(const StringName&)>;
		using MacroCallback = std::function<EvalValue(const StringName&, const EvalValueList&)>;

		Environment() = default;
		Environment(const Environment&) = default;
		Environment(Environment&&) = default;

		/// <summary>
		/// 指定された名前と引数リストでマクロを呼び出します。
		/// </summary>
		/// <param name="name">呼び出すマクロの名前。</param>
		/// <param name="args">マクロに渡す引数のリスト。</param>
		/// <returns>マクロが見つかった場合はその評価値。見つからない場合はnilを返します。</returns>
		EvalValue CallMacro(const StringName& name, const EvalValueList& args) {
			std::cout << "CallMacro(" << name << ", [";
			for (const auto& v : args) {
				std::cout << v << ",";
			}
			std::cout << "])" << std::endl;
			if (auto it = macro_map.find(name); it != macro_map.end()) {
				return it->second(args);
			}
			if (macro_callback) {
				auto ret = macro_callback(name, args);
				return ret;
			}
			
			throw RuntimeErrorException("Macro not found: " + std::string(name));
		}

		/// <summary>
		/// 指定された変数名に対応する値を取得します。
		/// </summary>
		/// <param name="name">取得したい変数の名前。</param>
		/// <returns>変数名に対応する値。変数が見つからない場合は、コールバックがあればその結果を返し、どちらもなければnilを返します。</returns>
		EvalValue GetVariableValue(const StringName& name) {
			std::cout << "GetVariableValue(" << name << ")";

			// フレームスタック -> グローバル(環境定義) -> コールバック の順に探索
			for (auto it = frame_stack.rbegin(); it != frame_stack.rend(); ++it) {
				if (auto value = it->GetVariableValue(name); !value.IsNil()) {
					std::cout << " = " << value << std::endl;
					return value;
				}
			}

			if (auto it = variable_map.find(name); it != variable_map.end()) {
				std::cout << " = " << it->second << std::endl;
				return it->second;
			}
			if (variable_callback) {
				auto ret = variable_callback(name);
				std::cout << " = " << ret << std::endl;
				return ret;
			}
			
			return { nullptr }; // 変数が見つからない場合はnilを返す
		}

		/// <summary>
		/// マクロを名前で登録します。
		/// </summary>
		/// <typeparam name="Fn">マクロ本体として使用する関数または関数オブジェクトの型。</typeparam>
		/// <param name="name">登録するマクロの名前。</param>
		/// <param name="macro_body">マクロの本体となる関数または関数オブジェクト。</param>
		template <typename Fn>
		void RegisterMacro(const StringName& name, Fn macro_body) {
			macro_map[name] = Macro(macro_body);
		}

		/// <summary>
		/// 指定されたマクロ名の登録を解除します。
		/// </summary>
		/// <param name="name">登録解除するマクロの名前。</param>
		void UnregisterMacro(const StringName& name) {
			macro_map.erase(name);
		}

		/// <summary>
		/// 指定された変数名に値を設定します。
		/// </summary>
		/// <param name="name">値を設定する変数の名前。</param>
		/// <param name="value">変数に設定する値。</param>
		/// <param name="overwrite">既存の変数の値を上書きするかどうかを指定します。デフォルトは false です。</param>
		/// <returns>値の設定に成功した場合は true、失敗した場合は false を返します。</returns>
		bool SetVariableValue(const StringName& name, EvalValue value, StoreModeFlags mode) {
			std::cout << "SetVariableValue(" << name << "," << value << ")" << std::endl;

			bool to_local = !HasFlag(mode, VAR_MODE_GLOBAL) || HasFlag(mode, VAR_MODE_LOCAL);
			bool to_global = !HasFlag(mode, VAR_MODE_LOCAL) || HasFlag(mode, VAR_MODE_GLOBAL);

			// フレームスタックが一つ以上存在している場合は、フレームスタック内の変数として格納する
			if (to_local) {
				bool search_all_frame = !HasFlag(mode, VAR_MODE_TOPFRAME);
				for (auto it = frame_stack.rbegin(); it != frame_stack.rend(); ++it) {
					if (it->SetVariableValue(name, value, mode)) {
						std::cout << " -> FrameStack" << std::endl;
						return true;
					}

					if (!search_all_frame) {
						// VAR_MODE_TOPFRAME が指定されている場合、最上位フレームのみを対象とする
						break;
					}
				}
			}

			if (to_global) {
				auto it = variable_map.find(name);
				if (it == variable_map.end() || it->first != name) {
					if (HasFlag(mode, VAR_MODE_CREATE)) {
						// 変数が存在しない場合は新規作成
						std::cout << " -> Global Variable" << std::endl;
						variable_map.insert(std::make_pair(name, value));
						return true;
					}
					else {
						std::cout << " -> Variable not found, not created" << std::endl;
						return false; // 変数が存在しない場合は何もしない
					}
				}

				// 変数が存在している
				if (HasFlag(mode, VAR_MODE_UPDATE)) {
					it->second = value;
					return true;
				}
				else {
					std::cout << " -> Variable already exists, not updated" << std::endl;
					return false;
				}
			}

			return false;

		}

		/// <summary>
		/// 指定された名前の変数を削除します。
		/// </summary>
		/// <param name="name">削除する変数の名前。</param>
		/// <returns>変数が削除された場合は true、存在しなかった場合は false を返します。</returns>
		bool DeleteVariable(const StringName& name, StoreModeFlags mode) {

			bool to_local = !HasFlag(mode, VAR_MODE_GLOBAL) || HasFlag(mode, VAR_MODE_LOCAL);
			bool to_global = !HasFlag(mode, VAR_MODE_LOCAL) || HasFlag(mode, VAR_MODE_GLOBAL);

			if (to_local) {
				bool search_all_frame = !HasFlag(mode, VAR_MODE_TOPFRAME);
				for (auto it = frame_stack.rbegin(); it != frame_stack.rend(); ++it) {
					if (it->DeleteVariable(name, mode)) {
						return true;
					}

					if (!search_all_frame) {
						// VAR_MODE_TOPFRAME が指定されている場合、最上位フレームのみを対象とする
						break;
					}
				}
			}

			if (to_global) {
				return variable_map.erase(name);
			}

			return false;
		}

		/// <summary>
		/// マクロコールバック関数を登録します。
		/// </summary>
		/// <typeparam name="Fn">コールバック関数の型。</typeparam>
		/// <param name="callback">登録するコールバック関数。</param>
		/// <remarks>ここで登録されたコールバック関数は、 CallMacro が呼び出されたときに該当するマクロが登録されていない場合に呼び出されます。</remarks>
		template <typename Fn>
		void RegisterMacroCallback(Fn callback) {
			macro_callback = callback;
		}

		/// <summary>
		/// 変数コールバック関数を登録します。
		/// </summary>
		/// <typeparam name="Fn">コールバック関数の型。任意の呼び出し可能オブジェクトを指定できます。</typeparam>
		/// <param name="callback">登録するコールバック関数。変数の変更時などに呼び出されます。</param>
		/// <remarks>ここで登録されたコールバック関数は、 GetVariableValue が呼び出されたときに該当する変数が登録されていない場合に呼び出されます。</remarks>
		template <typename Fn>
		void RegisterVariableCallback(Fn callback) {
			variable_callback = callback;
		}

		size_t GetFrameCount() const {
			return frame_stack.size();
		}

		StackFrame GetCurrentFrame() const {
			if (frame_stack.empty()) {
				throw std::runtime_error("No stack frame available");
			}
			return frame_stack.back();
		}

		void PushFrame() {
			frame_stack.emplace_back();
		}

		void PopFrame() {
			if (frame_stack.empty()) {
				throw std::runtime_error("No stack frame to pop");
			}
			frame_stack.pop_back();
		}

	private:
		Dictionary<Macro> macro_map;
		Dictionary<EvalValue> variable_map;
		std::vector<StackFrame> frame_stack;

		MacroCallback macro_callback;
		VariableCallback variable_callback;
	};

	struct EvalResult {
		// 評価結果
		EvalValue value;
		// フロー制御の状態
		FlowAction flow;

		EvalResult(EvalValue value, FlowAction flow = FlowAction::None)
			: value(value)
			, flow(flow)
		{}
	};

#pragma region Ast

	struct Ast {
		struct Constant;
		struct VarRef;
		struct List;
		struct Identity;
		struct UniOp;
		struct BinOp;
		struct Call;
		struct Assign;

		struct StatementBlock;
		struct Branch;
		struct Loop;

		struct FlowControl;

		struct IdentList;

		struct Function;
		struct VariableDeclaration;

		struct Program;

		virtual ~Ast() = default;
		virtual std::string to_string() const = 0;
		virtual EvalResult eval(Environment& env) const = 0;
	};


	// Programクラスの宣言のみ先に記述する 実装は後で行う
	struct Ast::Program : public Ast {
		friend struct Ast;

		std::vector<AstPtr> functions;
		std::vector<AstPtr> variables;

		std::string to_string() const override;

		EvalResult eval(Environment& env) const override;
		EvalResult EvalFunction(const StringName entry_point, EvalValueList args, Environment& env) const;
		EvalResult CallFunction(const StringName entry_point, EvalValueList args, Environment& env) const;

	private:
		bool StoreProgramPtrToEnvironment(Environment& env) const;
		void ClearProgramPtrFromEnvironment(Environment& env) const;

	public:
		static const Program* FromEnvironment(Environment& env);
	};


	struct Ast::Constant : public Ast {
		EvalValue value;

		Constant(EvalValue value) : value(value) {}

		std::string to_string() const override {
			return "Constant(" + value.ToString() + ")";
		}
		EvalResult eval(Environment& env) const override {
			return value;
		}
	};

	struct Ast::VarRef : public Ast {
		StringName name;

		VarRef(const StringName& name)
			: name(name) {}

		std::string to_string() const override {
			return "VarRef(" + name.s + ")";
		}
		EvalResult eval(Environment& env) const override {
			return env.GetVariableValue(name);
		}
	};

	struct Ast::List : public Ast {
		std::vector<AstPtr> list;

		List() {}

		template <typename Iterator>
		List(Iterator begin, Iterator end) : list() {
			// 上手くいかないのでこうする(Parser::Sequence<>::const_iterator が vectorコンストラクタに渡せない)
			for (auto it = begin; it != end; ++it) {
				list.push_back(*it);
			}
		}

		std::string to_string() const override {
			std::string s = "List(";
			for (const auto& e : list) {
				s += e->to_string() + ",";
			}
			s += ")";
			return s;
		}

		EvalResult eval(Environment& env) const override {
			// Listが直接evalされることは通常ない
			EvalResult result{ 0.0 };
			for (const auto& e : list) {
				result = e->eval(env);
			}
			return result;
		}
	};

	struct Ast::Identity : public Ast {
		AstPtr expr;

		Ast::Identity(const AstPtr& expr) : expr(expr) {}

		std::string to_string() const override {
			return "Identity(" + expr->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			return expr->eval(env);
		}
	};

	struct Ast::UniOp : public Ast {
		AstPtr expr;
		enum Op {
			OP_NEGATE,
			OP_NOT,
		} op;

		UniOp(const AstPtr& expr, Op op) : expr(expr), op(op) {}

		std::string to_string() const override {
			auto op_name = std::string(1, "-!"[op]);
			return "UniOp[" + op_name + "](" + expr->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			switch (op) {
			case OP_NEGATE:
				return EvalValue::ApplyUnaryNegate(expr->eval(env).value);
			case OP_NOT:
				return EvalValue::ApplyUnaryNot(expr->eval(env).value);
			}

			return EvalResult{nullptr};
		}
	};

	struct Ast::BinOp : public Ast {
		AstPtr lhs;
		AstPtr rhs;
		enum Op {
			OP_ADD,
			OP_SUB,
			OP_MUL,
			OP_DIV,

			OP_EQ,
			OP_DIFFER,
			OP_LESS,
			OP_LESSEQ,
			OP_GREATER,
			OP_GREATEREQ,

			OP_AND,
			OP_OR,
		} op;

		BinOp(const AstPtr& lhs, const AstPtr& rhs, Op op)
			: lhs(lhs)
			, rhs(rhs)
			, op(op) {}

		std::string to_string() const override {
			const char* op_name_map[] = {
				"+", "-", "*", "/",
				"==", "!=", "<", "<=", ">", ">=",
				"&&", "||"
			};

			return "BinOp[" + std::string(op_name_map[(int)op]) + "](" + lhs->to_string() + "," + rhs->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			switch (op) {
			case OP_ADD:
				return EvalValue::ApplyBinaryOp<std::plus<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_SUB:
				return EvalValue::ApplyBinaryOpNs<std::minus<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_MUL:
				return EvalValue::ApplyBinaryOpNs<std::multiplies<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_DIV:
				return EvalValue::ApplyBinaryOpNs<std::divides<>>(lhs->eval(env).value, rhs->eval(env).value);

			case OP_EQ:
				return EvalValue::ApplyBinaryOpCmp<std::equal_to<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_DIFFER:
				return EvalValue::ApplyBinaryOpCmp<std::not_equal_to<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_LESS:
				return EvalValue::ApplyBinaryOpCmp<std::less<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_LESSEQ:
				return EvalValue::ApplyBinaryOpCmp<std::less_equal<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_GREATER:
				return EvalValue::ApplyBinaryOpCmp<std::greater<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_GREATEREQ:
				return EvalValue::ApplyBinaryOpCmp<std::greater_equal<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_AND:
				return EvalValue::ApplyBinaryOpLogic<std::logical_and<>>(lhs->eval(env).value, rhs->eval(env).value);
			case OP_OR:
				return EvalValue::ApplyBinaryOpLogic<std::logical_or<>>(lhs->eval(env).value, rhs->eval(env).value);
			}

			throw RuntimeErrorException("Unknown binary operator");
		}

	};

	struct Ast::Call : public Ast {
		StringName name;
		AstPtr args;

		Call(const StringName& name, const AstPtr& args)
			: name(name)
			, args(args) {}

		std::string to_string() const override {
			return "Call(" + name.s + "(" + args->to_string() + "))";
		}

		EvalResult eval(Environment& env) const override {
			EvalValueList arg_values;
			if (auto list = std::dynamic_pointer_cast<Ast::List>(args)) {
				for (const auto& e : list->list) {
					arg_values.push_back(e->eval(env).value);
				}
			}

			auto program = Program::FromEnvironment(env);
			if (program) {
				// 該当する関数があれば呼び出す
				auto ret = program->CallFunction(name, arg_values, env);
				if (!ret.value.IsNil()) {
					return EvalResult(ret.value); // 関数が見つかり、値が返された
				}
			}

			return env.CallMacro(name, arg_values);
		}
	};

	struct Ast::Assign : public Ast {
		StringName name;
		AstPtr expr;

		Assign(const StringName& name, const AstPtr& expr)
			: name(name)
			, expr(expr) {}

		std::string to_string() const override {
			return "Assign(" + name.s + " = " + expr->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			env.SetVariableValue(name, expr->eval(env).value, StoreModeFlags(VAR_MODE_CREATE | VAR_MODE_UPDATE));
			return { nullptr };
		}

	};

	struct Ast::Branch : public Ast {
		AstPtr expr;
		AstPtr true_part;
		AstPtr false_part;

		Branch(const AstPtr& expr, const AstPtr& t_part, const AstPtr& f_part)
			: expr(expr)
			, true_part(t_part)
			, false_part(f_part) {}

		std::string to_string() const override {
			std::string s = "Branch(" + expr->to_string();
			if (true_part) {
				s += " : " + true_part->to_string();
			}
			if (false_part) {
				s += " : " + false_part->to_string();
			}
			s += ")";
			return s;
		}

		EvalResult eval(Environment& env) const override {
			if (expr->eval(env).value != 0.0) {
				return true_part ? true_part->eval(env).value : 0.0;
			}
			else {
				return false_part ? false_part->eval(env).value : 0.0;
			}
		}
	};

	struct Ast::Loop : public Ast {
		AstPtr expr;
		AstPtr stmt;

		Loop(const AstPtr& expr, const AstPtr& stmt)
			: expr(expr)
			, stmt(stmt) {}

		std::string to_string() const override {
			return "Loop(" + expr->to_string() + " : " + stmt->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			EvalResult result{ 0.0 };
			bool continue_flag = false;

			do {
				continue_flag = false;
				while (expr->eval(env).value != 0.0) {
					result = stmt->eval(env);
					auto flow_state = result.flow;
					if (flow_state == FlowAction::Break) {
						return { 0.0 };
					}
					else if (flow_state == FlowAction::Return) {
						return { result.value };
					}
					else if (flow_state == FlowAction::Continue) {
						continue_flag = true;
						break;
					}
				}
			} while (continue_flag);

			return result;
		}
	};


	struct Ast::FlowControl : public Ast {
		enum Control {
			CONTROL_CONTINUE,
			CONTROL_BREAK,
			CONTROL_RETURN,
			CONTROL_RETURN_VOID,
		} control;
		AstPtr expr;

		FlowControl(Control control, const AstPtr& expr = AstPtr{})
			: control(control)
			, expr(expr) {}

		std::string to_string() const override {
			const char* flow_control_names[] = {
				"continue",
				"break",
				"return",
				"return void"
			};

			return "FlowControl[" + std::string(flow_control_names[control]) + "](" + expr->to_string() + ")";
		}

		EvalResult eval(Environment& env) const override {
			switch (control) {
			case CONTROL_CONTINUE:
				return { nullptr, FlowAction::Continue };
			case CONTROL_BREAK:
				return { nullptr, FlowAction::Break };
			case CONTROL_RETURN:
				return { expr ? expr->eval(env).value : nullptr, FlowAction::Return };
			case CONTROL_RETURN_VOID:
				return { nullptr, FlowAction::Return};
			}
			return { nullptr };
		}
	};

	struct Ast::IdentList : public Ast {
		std::vector<StringName> names; // 複数の識別子を保持するリスト
		IdentList() = default;
		void add(const StringName& name) {
			names.push_back(name);
		}
		std::string to_string() const override {
			std::string s = "IdentList(";
			for (const auto& name : names) {
				s += name.s + ",";
			}
			s += ")";
			return s;
		}
		EvalResult eval(Environment& env) const override {
			// IdentListを評価してはいけない
			throw RuntimeErrorException("Invalid AST Call");
		}
	};

	struct Ast::Function : public Ast {
		StringName name;
		std::vector<StringName> arg_names; // 引数名のリスト
		AstPtr body;
		Function(const StringName& name, const AstPtr& argslist, const AstPtr& body)
			: name(name)
			, body(body)
		{
			if (auto list = std::dynamic_pointer_cast<Ast::IdentList>(argslist)) {
				for (const auto& arg_name : list->names) {
					arg_names.push_back(arg_name);
				}
			}
			else {
				throw SyntaxErrorException("Invalid Argument Pointer");
			}
		}
		std::string to_string() const override {
			return "Function(" + name.s + " : " + body->to_string() + ")";
		}
		EvalResult eval(Environment& env) const override {
			return eval(EvalValueList(), env);
		}

		EvalResult eval(const EvalValueList& args, Environment& env) const {
			env.PushFrame();
			for (size_t i = 0; i < arg_names.size(); ++i) {
				if (i < args.size()) {
					// 引数が足りない場合は0.0を設定
					env.SetVariableValue(arg_names[i], args[i], VAR_MODE_CREATE);
				}
				else {
					env.SetVariableValue(arg_names[i], 0.0, VAR_MODE_CREATE);
				}
			}
			
			EvalResult ret{ 0.0 };
			if (auto body_list = std::dynamic_pointer_cast<List>(body)) {
				bool returned = false;
				for (const auto& stmt : body_list->list) {
					ret = stmt->eval(env);
					if (ret.flow != FlowAction::None) {
						// フロー制御が発生した場合はそのまま返す
						returned = true;
						break;
					}
				}

				// returnが内部で発生しなかった場合は、戻り値なしとする
				if (!returned) {
					ret.value = nullptr;
				}
			}
			else {
				// 単一のステートメントの場合
				ret = body->eval(env);
				if (ret.flow == FlowAction::Return) {
					ret.value = ret.value; // return値をそのまま返す
				}
			}

			env.PopFrame();
			return EvalResult(ret.value); // FlowActionは無視
		}
	};

	struct Ast::VariableDeclaration : public Ast {
		StringName name;
		AstPtr init_value;
		VariableDeclaration(const StringName& name, const AstPtr& init_value = AstPtr())
			: name(name)
			, init_value(init_value) {}
		std::string to_string() const override {
			return "Variable(" + name.s + " = " + (init_value ? init_value->to_string() : "undefined") + ")";
		}
		EvalResult eval(Environment& env) const override {
			EvalValue value = init_value ? init_value->eval(env).value : 0.0;
			env.SetVariableValue(name, value, VAR_MODE_CREATE);
			return value;
		}
	};


	std::string Ast::Program::to_string() const {
		std::string s = "Program(...)"; // 簡易的な出力のみ
		return s;
	}

	EvalResult Ast::Program::eval(Environment& env) const {
		return EvalFunction("main", EvalValueList(), env);
	}

	EvalResult Ast::Program::EvalFunction(const StringName entry_point, EvalValueList args, Environment& env) const {
		if (!StoreProgramPtrToEnvironment(env)) {
			// 設定に失敗 別のプログラムが動作中
			throw RuntimeErrorException("Invalid Program State");
		}

		// variablesを評価して環境に登録
		for (const auto& var : variables) {
			var->eval(env);
		}

		auto ret = CallFunction(entry_point, args, env);

		ClearProgramPtrFromEnvironment(env);

		return ret;
	}

	EvalResult Ast::Program::CallFunction(const StringName entry_point, EvalValueList args, Environment& env) const {
		// functionsからエントリーポイントの関数を探して実行
		for (const auto& func : functions) {
			if (auto decl = std::dynamic_pointer_cast<Function>(func)) {
				if (decl->name == entry_point) {
					return decl->eval(args, env);
				}
			}
		}
		// エントリーポイントが見つからない場合はエラー
		throw RuntimeErrorException("Function not found: " + entry_point.s);
	}

	bool Ast::Program::StoreProgramPtrToEnvironment(Environment& env) const {
		return env.SetVariableValue("@@program", { const_cast<void*>(static_cast<const void*>(this)) }, StoreModeFlags(VAR_MODE_CREATE | VAR_MODE_GLOBAL));
	}

	void Ast::Program::ClearProgramPtrFromEnvironment(Environment& env) const {
		env.DeleteVariable("@@program", StoreModeFlags(VAR_MODE_GLOBAL));
	}

	const Ast::Program* Ast::Program::FromEnvironment(Environment& env) {
		// 環境からProgramポインタを取得
		EvalValue prog_value = env.GetVariableValue("@@program");
		return static_cast<const Program*>(std::get<void*>(prog_value.value));
	}

#pragma endregion Ast

	/// <summary>
	/// caper セマンティックアクション クラス
	/// </summary>
	class SemanticAction {
		Environment& parent;

	public:

		SemanticAction(Environment& ev) : parent(ev) {}

		void stack_overflow() { throw CompilationStackOverflowException("Compilation Stack overflow"); }
		void syntax_error() { throw SyntaxErrorException("SYntax Error"); }

		template <typename FromT>
		void upcast(ParseValue& to, const FromT& from) {
			to = from;
		}

		template <typename ToT>
		void downcast(ToT& to, const ParseValue& from) {
			to = std::get<ToT>(from);
		}

		// ProgramStart
		AstPtr ProgramStart() {
			std::cout << "<ProgramStart>" << std::endl;
			return std::make_shared<Ast::Program>();
		}
		
		// Program(Decl}
		AstPtr Program(const AstPtr& prog, const AstPtr & decl) {
			std::cout << "<Program>" << std::endl;
			auto prog_typed = std::dynamic_pointer_cast<Ast::Program>(prog);
			if (auto func_decl = std::dynamic_pointer_cast<Ast::Function>(decl)) {
				prog_typed->functions.push_back(func_decl);
			}
			else if (auto var_decl = std::dynamic_pointer_cast<Ast::VariableDeclaration>(decl)) {
				prog_typed->variables.push_back(var_decl);
			}
			else {
				std::cerr << "Unknown declaration type in Program" << std::endl;
			}

			return prog;
		}

		AstPtr EmptyStatement() {
			std::cout << "<EmptyStatement>" << std::endl;
			return AstPtr();
		}

		AstPtr ExprStatement(const AstPtr& expr) {
			std::cout << "<ExprStatement>" << std::endl;
			return expr;
		}

		AstPtr AssignStatement(const StringName& lhs, const AstPtr& rhs) {
			std::cout << "<AssignStatement>" << std::endl;
			return std::make_shared<Ast::Assign>(lhs, rhs);
		}

		//BlockStatement(std::vector<AstPtr>& stmts);
		template <template <typename> typename Sequence>
		AstPtr BlockStatement(const Sequence<AstPtr>& stmts) {
			std::cout << "<BlockStatement>" << std::endl;
			return std::make_shared<Ast::List>(stmts.begin(), stmts.end());
		}

		//IfStatement(Expr, Stmt);
		AstPtr IfStatement(const AstPtr& expr, const AstPtr& stmt) {
			std::cout << "<IfStatement>" << std::endl;
			return std::make_shared<Ast::Branch>(expr, stmt, AstPtr{});

		}

		//IfElseStatement(Expr, Stmt, Stmt);
		AstPtr IfElseStatement(const AstPtr& expr, const AstPtr& stmt_t, const AstPtr& stmt_f) {
			std::cout << "<IfElseStatement>" << std::endl;
			return std::make_shared<Ast::Branch>(expr, stmt_t, stmt_f);
		}

		////IfStatement(Expr, Stmt, Stmt);
		//template<typename OptionalT>
		//AstPtr IfStatement(const AstPtr& expr, const AstPtr& stmt_t, const OptionalT stmt_f) {
		//	std::cout << "<IfElseStatement>" << std::endl;
		//	return std::make_shared<Ast::Branch>(expr, stmt_t, stmt_f ? *stmt_f : AstPtr{});
		//	//return std::make_shared<Ast::Branch>(expr, stmt_t, AstPtr{});
		//}

		//// ElseStatement(Stmt)
		//AstPtr ElseStatement(const AstPtr& stmt) {
		//	std::cout << "<ElseStatement>" << std::endl;
		//	return stmt;
		//}

		//WhileStatement(Expr, Stmt);
		AstPtr WhileStatement(const AstPtr& expr, const AstPtr& stmt) {
			std::cout << "<WhileStatement>" << std::endl;
			return std::make_shared<Ast::Loop>(expr, stmt);
		}

		//ContinueStatement;
		AstPtr ContinueStatement() {
			std::cout << "<ContinueStatement>" << std::endl;
			return std::make_shared<Ast::FlowControl>(Ast::FlowControl::CONTROL_CONTINUE);
		}

		//BreakStatement;
		AstPtr BreakStatement() {
			std::cout << "<BreakStatement>" << std::endl;
			return std::make_shared<Ast::FlowControl>(Ast::FlowControl::CONTROL_BREAK);
		}

		//ReturnStatement;
		//ReturnStatement(Expr)
		AstPtr ReturnStatement(const AstPtr& expr = AstPtr{}) {
			std::cout << "<ReturnStatement>" << std::endl;
			return std::make_shared<Ast::FlowControl>(expr ? Ast::FlowControl::CONTROL_RETURN : Ast::FlowControl::CONTROL_RETURN_VOID, expr);
		}


		// Identity(Expr)
		AstPtr Identity(const AstPtr& expr) {
			std::cout << "<Identity>" << std::endl;
			return expr;
		}

		// Identity(EvalValue)
		AstPtr Identity(const EvalValue& value) {
			std::cout << "<Identity>" << std::endl;
			return std::make_shared<Ast::Constant>(value);
		}

		// MakeNot(Expr)
		AstPtr MakeNot(const AstPtr& expr) {
			std::cout << "<MakeNot>" << std::endl;
			return std::make_shared<Ast::UniOp>(expr, Ast::UniOp::OP_NOT);
		}

		// MakeEqual(Expr, Expr)
		AstPtr MakeEqual(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeEqual>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_EQ);
		}

		// MakeDiffer(Expr, Expr)
		AstPtr MakeDiffer(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeDiffer>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_DIFFER);
		}

		// MakeLess(Expr, Expr)
		AstPtr MakeLess(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeLess>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_LESS);
		}

		// MakeLessEq(Expr, Expr)
		AstPtr MakeLessEq(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeLessEq>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_LESSEQ);
		}

		// MakeGreater(Expr, Expr)
		AstPtr MakeGreater(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeGreater>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_GREATER);
		}

		// MakeGreaterEq(Expr, Expr)
		AstPtr MakeGreaterEq(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeGreaterEq>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_GREATEREQ);
		}

		// MakeAnd(Expr, Expr)
		AstPtr MakeAnd(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeAnd>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_AND);
		}

		// MakeOr(Expr, Expr)
		AstPtr MakeOr(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeOr>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_OR);
		}

		// MakeAdd(Expr, Expr)
		AstPtr MakeAdd(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeAdd>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_ADD);
		}

		// MakeSub(Expr, Expr)
		AstPtr MakeSub(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeSub>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_SUB);
		}

		// MakeMul(Expr, Expr)
		AstPtr MakeMul(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeMul>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_MUL);
		}

		// MakeDiv(Expr, Expr)
		AstPtr MakeDiv(const AstPtr& lhs, const AstPtr& rhs) {
			std::cout << "<MakeDiv>" << std::endl;
			return std::make_shared<Ast::BinOp>(lhs, rhs, Ast::BinOp::OP_DIV);
		}

		// Negate(Expr)
		AstPtr Negate(const AstPtr& expr) {
			std::cout << "<Negate>" << std::endl;
			return std::make_shared<Ast::UniOp>(expr, Ast::UniOp::OP_NEGATE);
		}

		// MakeCall(Expr, Expr)
		AstPtr MakeCall(const StringName& name, const AstPtr& rhs) {
			std::cout << "<MakeCall>" << std::endl;
			return std::make_shared<Ast::Call>(name, rhs);
		}

		// Variable(String)
		AstPtr Variable(const StringName& name) {
			std::cout << "<Variable>" << std::endl;
			return std::make_shared<Ast::VarRef>(name);
		}

		// EmptyList
		AstPtr EmptyList() {
			std::cout << "<EmptyList>" << std::endl;
			return std::make_shared<Ast::List>();
		}

		// MakeList(std::Sequence<Expr>)
		template <template <typename> typename Sequence>
		AstPtr MakeList(Sequence<AstPtr> exprs) {
			std::cout << "<MakeList>" << std::endl;
			return std::make_shared<Ast::List>(exprs.begin(), exprs.end());
		}

		// EmptyIdentList
		AstPtr EmptyIdentList() {
			std::cout << "<EmptyIdentList>" << std::endl;
			return std::make_shared<Ast::IdentList>();
		}

		// MakeIdentList(std::Sequence<String>)
		template <template <typename> typename Sequence>
		AstPtr MakeIdentList(Sequence<StringName> names) {
			std::cout << "<MakeIdentList>" << std::endl;
			auto ident_list = std::make_shared<Ast::IdentList>();
			for (const auto& name : names) {
				ident_list->add(name);
			}
			return ident_list;
		}

		// FunctionDeclaration(ident, list)
		AstPtr FunctionDeclaration(const StringName& name, const AstPtr& argslist, const AstPtr& body) {
			std::cout << "<FunctionDeclaration>" << std::endl;
			return std::make_shared<Ast::Function>(name, argslist, body);
		}

		// VariableDeclaration(ident)
		AstPtr VariableDeclaration(const StringName& name, const AstPtr& init_value = AstPtr{}) {
			std::cout << "<VariableDeclaration>" << std::endl;
			return std::make_shared<Ast::VariableDeclaration>(name, init_value);
		}

		// VariableDeclaration(ident, value)
		AstPtr VariableDeclaration(const StringName& name, const EvalValue& init_value) {
			std::cout << "<VariableDeclaration>" << std::endl;
			return std::make_shared<Ast::VariableDeclaration>(name, std::make_shared<Ast::Constant>(init_value));
		}

	};

	/// <summary>
	/// トークナイザー クラス
	/// </summary>
	/// <typeparam name="Iterator">入力イテレーター</typeparam>
	template <typename Iterator>
	class Tokenizer {
		using MatchResult = std::match_results<Iterator>;

		Iterator it;
		Iterator end;
		struct TokenMap {
			// トークンの文字列または正規表現
			const char* str;
			// トークンの種類
			Token token;

			// 正規表現ハンドラ
			// この値が設定されている場合、 str は正規表現として扱われる。
			int(_stdcall* regex_handler)(const MatchResult&, std::vector<TokenValue>&);

			// マッチした文字列を評価しない正規表現ハンドラ
			static int _stdcall EmptyHandler(const MatchResult&, std::vector<TokenValue>&) {
				return -1;
			}
		};

		std::vector<TokenMap> tokens;

		std::unordered_map<TokenMap*, std::regex> regex_cache;


		std::vector<TokenValue> token_values;

	public:
		/// <summary>
		/// イテレータ範囲からトークナイザーを初期化します。
		/// </summary>
		/// <param name="begin">トークナイズ対象となる入力範囲の開始イテレータ。</param>
		/// <param name="end">トークナイズ対象となる入力範囲の終端イテレータ。</param>
		Tokenizer(Iterator begin, Iterator end) : it(begin), end(end) {

			// トークン値の初期化
			tokens = {
				// 2文字演算子
				// (他の記号より優先してマッチング)
				{ "==", token_op_equal },
				{ "!=", token_op_differ },
				{ "<=", token_op_lesseq },
				{ ">=", token_op_greater },
				{ "&&", token_op_and_and },
				{ "||", token_op_or_or },

				// 1文字演算子
				{ "!", token_op_not },
				{ "<", token_op_less },
				{ ">", token_op_greater },
				{ "+", token_op_add },
				{ "-", token_op_sub },
				{ "*", token_op_mul },
				{ "/", token_op_div },
				{ "=", token_op_assign },

				// 1文字トークン
				{ "(", token_paren_open },
				{ ")", token_paren_close },
				{ "{", token_brace_open },
				{ "}", token_brace_close },
				{ "$", token_dollar },
				{ ",", token_comma },
				{ ";", token_semicolon },

				// キーワード
				{"^if\\b", token_kwd_if, TokenMap::EmptyHandler},
				{"^else\\b", token_kwd_else, TokenMap::EmptyHandler},
				{"^while\\b", token_kwd_while, TokenMap::EmptyHandler},
				{"^continue\\b",token_kwd_continue, TokenMap::EmptyHandler},
				{"^break\\b",token_kwd_break, TokenMap::EmptyHandler},
				{"^return\\b",token_kwd_return, TokenMap::EmptyHandler},
				{"^func\\b", token_kwd_func, TokenMap::EmptyHandler},
				{"^var\\b", token_kwd_var, TokenMap::EmptyHandler},

				// 識別子
				{
					"^[a-zA-Z_][a-zA-Z0-9_]*",
					token_ident,
					[](const MatchResult& match_result, std::vector<TokenValue>& values) -> int {
						int index = (int)values.size();
						values.push_back(StringName(match_result.str()));
						return index;
					}
				},

				// 数値リテラル
				{
					"^([0-9]*[.])?[0-9]+",
					token_number,
					[](const MatchResult& match_result, std::vector<TokenValue>& values) -> int {
						double val = std::stod(match_result.str());
						int index = (int)values.size();
						values.push_back(val);
						return index;
					}
				},
			};

		}

		template <typename T = TokenValue>
		const T& GetTokenValue(int index) const {
			if constexpr (std::is_same_v<T, TokenValue>) {
				return token_values[index];
			}
			else {
				return std::get<T>(token_values[index]);
			}
		}

		bool Next(Token& out_token, int& out_value_index) {
			while (it != end && isspace(*it)) {
				++it;
			}
			if (it == end) {
				out_token = token_eof;
				out_value_index = -1;
				return false;
			}
			Token token = token_error;
			size_t token_length = 0;
			int token_index = -1;
			for (auto& pattern : tokens) {
				if (pattern.regex_handler) {
					auto it_r = regex_cache.find(&pattern);
					if (it_r == regex_cache.end()) {
						auto insert_result = regex_cache.insert({ &pattern, std::regex{ pattern.str } });
						it_r = insert_result.first;
					}
					MatchResult match_result;
					if (std::regex_search(it, end, match_result, it_r->second, std::regex_constants::match_continuous)) {

						token = pattern.token;
						token_length = match_result.length();
						token_index = pattern.regex_handler(match_result, token_values);
						break;
					}
				}
				else {
					size_t len = strlen(pattern.str);
					const auto ptr = &*it;
					if (strncmp(ptr, pattern.str, len) == 0) {
						token = pattern.token;
						token_length = len;
						token_index = -1;
						break;
					}
				}
			}
			it += token_length;
			out_token = token;
			out_value_index = token_index;
			return true;
		}
	};


}
