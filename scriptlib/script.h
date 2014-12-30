#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <stdint.h>

namespace Script {
	/**
	* <summery>Codeの実行結果を表す列挙対</summery>
	*/
	enum ReturnState {
		/// <summery>正常に完了</summery>
		None = 0,
		/// <summery>スクリプトを中断する</summery>
		Wait,
		/// <summery>エラーが発生した</summery>
		/// <remark>これを返す場合は、ThreadのerrorCodeメンバにエラー理由を格納すること。</remark>
		Error,
		/// <summery>スクリプトが終了した</summery>
		Finished,
	};

	/**
	* <summery>スクリプトエラーの情報</summery>
	*/
	enum ErrorType {
		OK = 0,
		FileCannotOpen = 0x10,
		FileCannotRead,
		InvalidFileHeader,
		MemAllocationError,

		WorkareaOutOfRange = 0x20,
		CallstackOverflow,
		CallstackUnderflow,
		WorkstackOverflow,
		WorkstackUnderflow,
		CodeindexOutOfRange,
		InvalidOpcode,
		InvalidOperand,

		ScriptHasFinished,
	};

	struct Code;
	struct State;
	struct Thread;

	/// <summery>ワークスタックとワークエリアに格納される値の単位。単精度浮動小数点数と符号付き32ビット整数、型無しポインタの共用体。</summery>
	union Value {
		float float_;
		int32_t int_;
		void* ptr_;

		inline Value() : int_{ -1 } {};
		inline Value(float f) : float_{ f } {};
		inline Value(int32_t i) : int_{ i } {};
		inline Value(void* p) : ptr_{ p } {};

		operator float&() { return float_; }
		operator int32_t&() { return int_; }
		operator void*&() { return ptr_; }
	};

	/// <summery>スクリプトの処理の一単位。</summery>
	typedef std::function<ReturnState(Thread&, const Code&)> Opcode;

	template <typename Fn>
	inline Opcode ToOpcode(Fn fn) {
		return fn;
	}

	/// <summery>実行の最小単位。実処理を行うOpcodeと追加のオプション値からなる。初期化時に何もオプションを指定しなかった場合、整数-1が設定される。</summery>
	struct Code {
		Opcode opcode;

		union {
			float val;
			int32_t option;
		};

		Code() : opcode{ nullptr }, option{ -1 } { }
		Code(const Code&) = default;

		template<typename Fn>
		Code(Fn f) : opcode{ f }, option{ -1 } {}
		template<typename Fn>
		Code(Fn f, float n) : opcode{ f }, val{ n } {}
		template<typename Fn>
		Code(Fn f, int32_t i) : opcode{ f }, option{ i } {}

	};

	/// <summery>一連のコード群を提供するクラス</summery>
	class CodeProvider : public std::enable_shared_from_this<CodeProvider> {
	public:
		typedef std::shared_ptr<CodeProvider> Ptr;

		/// <summery>指定インデックスのCodeを得る</summery>
		virtual const Code& Get(int index) = 0;
		/// <summery>コード群のサイズを得る。</summery>
		virtual int Length() = 0;
		/// <summery>指定された名称に関連付けられたコードインデックスを得る。存在しない場合は-1を返す。</summery>
		virtual int EntryPoint(const char* name) = 0;

		/// <summery>Stateを作成する</summery>
		virtual std::shared_ptr<State> CreateState();
	};

	/// <summery>実行中のグローバルな情報を格納するクラス</summery>
	struct State : public std::enable_shared_from_this<State> {
		typedef std::shared_ptr<State> Ptr;

		std::shared_ptr<CodeProvider> provider;
		std::vector<Value> workarea;

		State(std::shared_ptr<CodeProvider>);
		~State();

		/// <summery>指定のコードインデックスから開始するスレッドを作成する。未指定の場合はインデックス0番から開始する。</summery>
		std::shared_ptr<Thread> CreateThread(int entryPoint = 0);
		/// <summery>指定名称のコードインデックスから開始するスレッドを作成する。</summery>
		std::shared_ptr<Thread> CreateThread(const char* entryPoint);

		/// <summery>ワークエリアを整数値0で初期化する。</summery>
		void Reset();
	};

	struct Thread {
		typedef std::shared_ptr<Thread> Ptr;

		std::shared_ptr<State> state;

		std::vector<Value> workstack;
		std::vector<int> callstack;
		//std::vector<int> stackBase;
		
		int codeindex;
		int waitcount;
		ErrorType errorCode;

		Thread(std::shared_ptr<State>, int);

		/// <summery>スクリプトを実行する。nowaitを指定するとReturnState::Waitで中断されなくなる。</summery>
		ReturnState Run(bool nowait = false);
		/// <summery>スレッドのスタック等を完全に削除し、指定されたコードインデックスから実行するように設定する。未指定の場合はインデックス0番から開始する。</summery>
		void Reset(int ep = 0);

#pragma region Operation Definitions
		static ReturnState opEnd(Thread&, const Code& code);
		static ReturnState opWait(Thread&, const Code& code);

		static ReturnState opGoto(Thread&, const Code& code);
		static ReturnState opJmp(Thread&, const Code& code);
		//static ReturnState opCpt(Thread&, const Code& code);
		static ReturnState opFwd(Thread&, const Code& code);
		static ReturnState opRew(Thread&, const Code& code);
		static ReturnState opJz(Thread&, const Code& code);
		static ReturnState opJnz(Thread&, const Code& code);
		static ReturnState opJpos(Thread&, const Code& code);
		static ReturnState opJneg(Thread&, const Code& code);
		static ReturnState opJeq(Thread&, const Code& code);
		static ReturnState opJne(Thread&, const Code& code);
		static ReturnState opJgt(Thread&, const Code& code);
		static ReturnState opJge(Thread&, const Code& code);
		static ReturnState opJlt(Thread&, const Code& code);
		static ReturnState opJle(Thread&, const Code& code);
		static ReturnState opCmp(Thread&, const Code& code);
		static ReturnState opIs(Thread&, const Code& code);

		static ReturnState opAdd(Thread&, const Code& code);
		static ReturnState opAdds(Thread&, const Code& code);
		static ReturnState opMul(Thread&, const Code& code);
		static ReturnState opMuls(Thread&, const Code& code);
		static ReturnState opSub(Thread&, const Code& code);
		static ReturnState opNeg(Thread&, const Code& code);
		static ReturnState opDiv(Thread&, const Code& code);
		static ReturnState opMod(Thread&, const Code& code);
		static ReturnState opSin(Thread&, const Code& code);
		static ReturnState opCos(Thread&, const Code& code);
		static ReturnState opTan(Thread&, const Code& code);
		static ReturnState opArg(Thread&, const Code& code);
		static ReturnState opSqrt(Thread&, const Code& code);
		static ReturnState opPow(Thread&, const Code& code);
		static ReturnState opLog(Thread&, const Code& code);
		static ReturnState opLog10(Thread&, const Code& code);
		static ReturnState opLen(Thread&, const Code& code);
		static ReturnState opD2r(Thread&, const Code& code);
		static ReturnState opR2d(Thread&, const Code& code);

		static ReturnState opLod(Thread&, const Code& code);
		static ReturnState opSto(Thread&, const Code& code);
		static ReturnState opVlod(Thread&, const Code& code);
		static ReturnState opVsto(Thread&, const Code& code);
		static ReturnState opDup(Thread&, const Code& code);
		static ReturnState opSpps(Thread&, const Code& code);
		static ReturnState opDel(Thread&, const Code& code);
		static ReturnState opCls(Thread&, const Code& code);
		static ReturnState opCall(Thread&, const Code& code);
		static ReturnState opRet(Thread&, const Code& code);

		static ReturnState opPush(Thread&, const Code& code);

		static ReturnState opNsAdd(Thread&, const Code& code);
		static ReturnState opNsSub(Thread&, const Code& code);
		static ReturnState opNsMul(Thread&, const Code& code);
		static ReturnState opNsDiv(Thread&, const Code& code);

		static ReturnState opPushSb(Thread&, const Code&);
		static ReturnState opPopSb(Thread&, const Code&);

		static ReturnState opNull(Thread&, const Code& code);
#pragma endregion

		Value StackPop(int count = 1);
		Value StackPush(Value);
		Value& StackTop();
		unsigned int StackSize();
		void ClearStack();

	};



	namespace Loader {
			enum AttrType {
				Integer,
				Float,
				Comparer,
				SpecialNumbers,
				EntryPointSymbol,
				Property,
			};

		class Generator {
		public:
			struct CodeSkelton {
				Opcode opcode;
				AttrType type;
			};

			std::unordered_map<std::string, CodeSkelton> map;

			Generator();


			Code operator ()(const std::string& sig, const std::string& attr, std::string& bindSymbol);

			bool ParseAttrAsInteger(Code& c, const std::string& attr);
			bool ParseAttrAsFloat(Code& c, const std::string& attr);
			bool ParseAttrAsComparer(Code& c, const std::string& attr);
			bool ParseAttrAsSpecialNumbers(Code& c, const std::string& attr);
			bool ParseAttrAsProperty(Code& c, const std::string& attr);
		};

		std::shared_ptr<CodeProvider> Load(const char* filepath, Generator& gen);
		std::shared_ptr<CodeProvider> FromCodeSet(std::vector<Code>&& codes,
												  std::unordered_map<std::string, int>&& entrypoints);
		std::shared_ptr<CodeProvider> FromCodeSet(const std::vector<Code>& codes,
												  const std::unordered_map<std::string, int>& entrypoints);
		std::shared_ptr<CodeProvider> FromCodeSet(std::vector<Code>&& codes);
		std::shared_ptr<CodeProvider> FromCodeSet(const std::vector<Code>& codes);
	}
}

#endif
