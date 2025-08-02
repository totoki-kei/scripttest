#pragma once

#ifndef SCRIPENVIRONMENT_H_
#define SCRIPENVIRONMENT_H_

#include "ScripTypes.h"
#include "ScripException.h"

namespace Scrip {
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
			return EvalValue(); // 変数が見つからない場合はnilを返す
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

			return {}; // 変数が見つからない場合はnilを返す
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


}

#endif // SCRIPENVIRONMENT_H_
