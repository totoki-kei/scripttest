// © Totoki Kei, 2014

#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <array>
#include <stdexcept>

#include <boost/lambda/lambda.hpp>
#include <boost/function.hpp>

namespace Lambs {

	//
	// 変数名、ラベル名の解決に失敗したことを表す例外(domain_error)
	struct name_not_found : public std::domain_error {
		name_not_found(const std::string& name) : std::domain_error("Name \"" + name + "\" not found.") {}
	};

#pragma region State

	// ステートのインターフェースを表すクラス
	// 必ずしもこれを継承する必要は無い
	template <typename Value>
	class State {
	protected:
		int ip;
	public:
		typedef Value ValueType;

		State() : ip{ 0 } = default;
		State(const State<Value>& v) : ip{ v.ip } {}

		int Ip() const { return ip; }
		void Next() { ip++; }
		void SetIp(int n) { ip = n; }

		//virtual ValueType& operator[](int) = 0;
		//virtual const ValueType& operator[](int) const = 0;
	};

	// 可変長サイズステート
	template <typename Value = double>
	class VectorState : public State<Value> {
	public:
		std::vector<Value> var;

		VectorState() = default;
		VectorState(size_t varSize) : var(varSize) {}

		VectorState(const VectorState<Value>& v)
			: State<Value>((const State<Value>&)v)
			, var(v.var) {}

		VectorState(VectorState<Value>&& v)
			: State<Value>((const State<Value>&)v)
			, var(std::move(v.var)) {}


		Value& operator[] (int index) {
			if (index >= (int)var.size()) var.resize(index + 1);
			return var[index];
		}

		const Value& operator[] (int index) const {
			return var[index];
		}
	};

	// 固定長サイズステート
	template <size_t Size, typename Value = double>
	class FixedSizeState : public State<Value> {
	public:
		std::array<Value, Size> var;

		FixedSizeState() = default;

		FixedSizeState(const FixedSizeState<Size, Value>& v)
			: State<Value>((const State<Value>&)v)
			, var(v.var) {}

		FixedSizeState(FixedSizeState<Size, Value>&& v)
			: State<Value>((const State<Value>&)v)
			, var(std::move(v.var)) {}

		Value& operator[] (int index) {
			return var[index];
		}
		const Value& operator[] (int index) const {
			return var[index];
		}
	};

#pragma endregion

	template <typename State>
	using Fn = boost::function<bool(State&)>;

	template <typename State , typename Value = typename State::ValueType>
	class FnList {
	public:
		typedef FnList<Value, State> ThisType;
		typedef Value ValueType;
		typedef State StateType;
		typedef Fn<StateType> FuncType;


	private:
		std::vector<FuncType> list;
		std::map<std::string, int> fnLabels;
		std::map<std::string, int> varLabels;

	public:
		FnList() = default;
		FnList(const ThisType& fl)
			: list(fl.list)
			, fnLabels(fl.fnLabels)
			, varLabels(fl.varLabels) {}
		FnList(ThisType&& fl)
			: list(std::move(fl.list))
			, fnLabels(std::move(fl.fnLabels))
			, varLabels(std::move(fl.varLabels)) {}

#pragma region ListConstructor

		//　ラベルを追加する
		FnList& label(const std::string& label) {
			fnLabels.insert(std::make_pair(label, list.size()));
			return *this;
		}
		// ラベルを追加する
		FnList& operator [](const std::string& label) {
			return this->label(label);
		}

		// メソッドリストに関数を追加する
		FnList& push(const FuncType& f) {
			list.push_back(f);
			return *this;
		}

		// メソッドリストに関数を追加する
		FnList& operator ()(const FuncType& f) {
			return this->push(f);
		}
#pragma endregion

#pragma region Reference
		struct RefTag {
			int index;

			RefTag() = delete;

			RefTag(int i) : index{ i } {}

			Value& operator() (State& s) const {
				return s[index];
			}

			// boost::lambda用 戻り値定義
			template <typename Sig>
			struct sig {
				typedef Value& type;
			};

			// boost::lambda用 関数呼び出し構文
			template <typename Ret, typename A, typename B, typename C, typename Env>
			inline Ret call(A& a, const B&, const C&, const Env&) const { return (*this)(a); }

		};

		// 変数への参照を得るラムダ要素を返す
		static boost::lambda::lambda_functor<RefTag> r(int index) {
			return RefTag{ index };
		}

		// 変数への参照を得るラムダ要素を返す
		boost::lambda::lambda_functor<RefTag> r(const std::string& name) {
			auto iter = varLabels.find(name);
			if (iter == varLabels.end()) throw name_not_found(name);
			return RefTag{ GetVarIndex(name) };
		}
#pragma endregion

#pragma region NamedReference
		struct NameRefTag {
			const ThisType& fn;
			const std::string name;

			NameRefTag() = delete;

			NameRefTag(const ThisType& f, const std::string& n) : fn{ f }, name(n) {}

			Value& operator() (State& s) const {
				return fn.GetVar(s, name);
			}

			// boost::lambda用 戻り値定義
			template <typename Sig>
			struct sig {
				typedef Value& type;
			};

			// boost::lambda用 関数呼び出し構文
			template <typename Ret, typename A, typename B, typename C, typename Env>
			inline Ret call(A& a, const B&, const C&, const Env&) const { return (*this)(a); }
		};

		// 実行時名前解決で変数への参照を取得するラムダ要素を返す
		boost::lambda::lambda_functor<NameRefTag> rn(const std::string& name) {
			return NameRefTag(*this, name);
		}

#pragma endregion

#pragma region Variable

		struct VarTag {
			int index;

			VarTag() = delete;

			VarTag(int i) : index{ i } {}

			Value operator() (State& s) const {
				return s[index];
			}

			// boost::lambda用 戻り値定義
			template <typename Sig>
			struct sig {
				typedef Value type;
			};

			// boost::lambda用 関数呼び出し構文
			template <typename Ret, typename A, typename B, typename C, typename Env>
			inline Ret call(A& a, const B&, const C&, const Env&) const { return (*this)(a); }

		};

		// 変数の値を得るラムダ要素を返す
		static boost::lambda::lambda_functor<VarTag> v(int index) {
			return VarTag{ index };
		}
		// 変数の値を得るラムダ要素を返す
		boost::lambda::lambda_functor<VarTag> v(const std::string& name) {
			return VarTag{ GetVarIndex(name) };
		}
#pragma endregion

#pragma region NamedVariable
		struct NameVarTag {
			const ThisType& fn;
			const std::string name;

			NameVarTag() = delete;

			NameVarTag(const ThisType& f, const std::string& n) : fn{ f }, name(n) {}

			Value& operator() (State& s) const {
				return fn.GetVar(s, name);
			}

			// boost::lambda用 戻り値定義
			template <typename Sig>
			struct sig {
				typedef Value type;
			};

			// boost::lambda用 関数呼び出し構文
			template <typename Ret, typename A, typename B, typename C, typename Env>
			inline Ret call(A& a, const B&, const C&, const Env&) const { return (*this)(a); }
		};
		// 実行時名前解決で変数の値を得るラムダ要素を返す
		boost::lambda::lambda_functor<NameVarTag> vn(const std::string& name) {
			return NameVarTag{ *this, name };
		}

#pragma endregion

#pragma region Jump
		struct JumpTag {
			int index;

			JumpTag(int i) : index{ i } {}

			void operator() (State& s) const {
				s.SetIp(index);
			}

			// boost::lambda用 戻り値定義
			template <typename Sig>
			struct sig {
				typedef void type;
			};

			// boost::lambda用 関数呼び出し構文
			template <typename Ret, typename A, typename B, typename C, typename Env>
			inline Ret call(A& a, const B&, const C&, const Env&) const { (*this)(a); }
		};
		// 特定行へのジャンプを行うラムダ要素を返す
		static boost::lambda::lambda_functor<JumpTag> jmp(int index) {
			return JumpTag{ index };
		}
#pragma endregion

#pragma region LabelJump
		struct LabelJumpTag {
			const std::string name;
			const ThisType& fn;

			LabelJumpTag(const ThisType& f, const std::string& n) : fn{ f }, name(n) {}

			void operator() (State& s) const {
				auto iter = fn.fnLabels.find(name);
				if (iter == fn.fnLabels.end()) throw name_not_found(name);
				s.SetIp(iter->second);
			}

			// boost::lambda用 戻り値定義
			template <typename Sig>
			struct sig {
				typedef void type;
			};

			// boost::lambda用 関数呼び出し構文
			template <typename Ret, typename A, typename B, typename C, typename Env>
			inline Ret call(A& a, const B&, const C&, const Env&) const { (*this)(a); }
		};
		// 実行時名前解決でラベルへのジャンプを行うラムダ要素を返す
		boost::lambda::lambda_functor<LabelJumpTag> jmp(const std::string& name) {
			return LabelJumpTag{ *this, name };
		}

#pragma endregion

		// 変数へ名前を付ける
		void NameToVar(int index, const std::string& name) {
			varLabels.insert(make_pair(name, index));
		}

		// 名前から変数の値を得る
		const Value& GetVar(const State& s, const std::string& name) const {
			return s[GetVarIndex(name)];
		}

		// 名前から変数の値を得る
		Value& GetVar(State& s, const std::string& name) const {
			return s[GetVarIndex(name)];
		}

		// 名前から変数のインデックスを得る
		// 見つからない場合は-1を返す
		int GetVarIndex(const std::string& name) const {
			auto iter = varLabels.find(name);
			if (iter == varLabels.end()) return -1;
			return iter->second;
		}

		// 指定の名前のラベルが存在する場合はそのインデックスを返す
		// 見つからない場合は-1を返す
		int GetLabel(const std::string name) const {
			auto iter = fnLabels.find(name);
			if (iter == fnLabels.end()) return -1;
			return iter->second;
		}

		// ステップ実行する
		// 再開可能である場合はtrueを、処理が終了している場合はfalseを返す
		bool Run(State& s) const {
			while (Check(s)) {
				auto old_ip = s.Ip();
				bool br = list[s.Ip()](boost::ref(s));
				if (s.Ip() == old_ip) {
					s.Next();
				}
				if (!br) return true;
			}
			return false;
		}

		// 指定のステートが再開可能である場合はtrueを返す
		bool Check(State& s) const {
			return 0 <= s.Ip() && s.Ip() < (int)list.size();
		}
	};


}

