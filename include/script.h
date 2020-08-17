#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>

namespace Script {

	/// <summary>Codeの実行結果を表す列挙対</summary>
	enum ReturnState {
		/// <summary>正常に完了</summary>
		None = 0,
		/// <summary>スクリプトを中断する</summary>
		Wait,
		/// <summary>エラーが発生した</summary>
		/// <remark>これを返す場合は、ThreadのerrorCodeメンバにエラー理由を格納すること。</remark>
		Error,
		/// <summary>スクリプトが終了した</summary>
		Finished,
	};

	/// <summary>スクリプトエラーの情報</summary>
	enum class ErrorType {
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

	enum class ComparerAttribute : uint32_t {
		Equal,
		NotEqual,
		Greater,
		LessEqual,
		Less,
		GreaterEqual,
		And,
		Nand,
		Or,
		Nor,
		Xor,
		Nxor,

		$NotBit = 0x01,
	};

	enum class NumTypeAttribute : uint32_t {
		Zero,
		NotZero,
		Plus,
		NotPlus,
		Minus,
		NotMinus,
		PosInf,
		NotPosInf,
		NegInf,
		NotNegInf,
		NaN,
		NotNaN,

		$NotBit = 0x01,
	};

	enum class PropertyAttribute : uint32_t {
		Get,
		Set,
	};

	struct Code;
	class State;
	class Thread;

	/// <summary>ワークスタックとワークエリアに格納される値の単位。単精度浮動小数点数と符号付き32ビット整数、型無しポインタの共用体。</summary>
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

	/// <summary>スクリプトの処理の一単位。</summary>
	typedef std::function<ReturnState(Thread&, const Code&)> Opcode;

	/// <summary>実行の最小単位。実処理を行うOpcodeと追加のオプション値からなる。初期化時に何もオプションを指定しなかった場合、整数-1が設定される。</summary>
	struct Code {
		Opcode opcode;

		union Attribute {
			float float_;
			int32_t int_;
			ComparerAttribute cmp_;
			NumTypeAttribute ntype_;
			uint32_t ep_;
			PropertyAttribute prop_;
			int32_t str_;

			Attribute() : int_{ 0 } {}

			Attribute(float f) : float_{ f } {}
			Attribute(int32_t si) : int_{ si } {}
			Attribute(ComparerAttribute c) : cmp_{ c } {}
			Attribute(NumTypeAttribute na) : ntype_{ na } {}
			Attribute(uint32_t ui) : ep_{ ui } {}
			Attribute(PropertyAttribute p) : prop_{ p } {}

		} attr;

		Code() : opcode{ nullptr }, attr{ -1 } {}
		Code(const Code&) = default;

		template<typename Fn>
		Code(Fn f) : opcode{ f }, attr{ -1 } {}

		template<typename Fn, typename Attr>
		Code(Fn f, Attr a) : opcode{ f }, attr{ a } {}
	};

	/// <summary>一連のコード群を提供するクラス</summary>
	class CodeProvider : public std::enable_shared_from_this<CodeProvider> {
	public:
		/// <summary>ポインタ型(shared_ptr)</summary>
		typedef std::shared_ptr<CodeProvider> Ptr;

		/// <summary>指定インデックスのCodeを得る</summary>
		virtual const Code& Get(int index) = 0;
		/// <summary>コード群のサイズを得る。</summary>
		virtual int Length() = 0;
		/// <summary>指定された名称に関連付けられたコードインデックスを得る。存在しない場合は-1を返す。</summary>
		virtual int Label(const char* name) = 0;
		/// <summary>文字列テーブルから文字列を得る。存在しないIDの場合はnullptrを返す。</summary>
		virtual const char* GetString(int id) = 0;

		/// <summary>Stateを作成する</summary>
		virtual std::shared_ptr<State> CreateState();
	};

	/// <summary>実行中のグローバルな情報を格納するクラス</summary>
	class State : public std::enable_shared_from_this<State> {
	public:
		typedef std::shared_ptr<State> Ptr;

	protected:
		std::shared_ptr<CodeProvider> provider;
		std::vector<Value> workarea;
		void* registry;

	public:
		State(std::shared_ptr<CodeProvider>);

		/// <summary>指定のコードインデックスから開始するスレッドを作成する。未指定の場合はインデックス0番から開始する。</summary>
		std::shared_ptr<Thread> CreateThread(int entryPoint = 0);
		/// <summary>指定名称のコードインデックスから開始するスレッドを作成する。</summary>
		std::shared_ptr<Thread> CreateThread(const char* entryPoint);

		CodeProvider* GetCodeProvider() { return provider.get(); }

		Value& At(int index) {
			if (index <= 256 && index >= (int)workarea.size()) {
				workarea.resize(index + 1);
			}
			return workarea[index]; 
		}
		size_t Count() { return workarea.size(); }

		void * GetRegistry() { return registry; }
		void SetRegistry(void* ptr) { registry = ptr; }


		/// <summary>ワークエリアを整数値0で初期化する。</summary>
		void Reset();
	};

	class Thread {
	public:
		typedef std::shared_ptr<Thread> Ptr;

	private:
		std::shared_ptr<State> state;

		std::vector<Value> workstack;
		std::vector<int> callstack;
		size_t stackBase;
		
		int codeindex;
		int waitcount;
		ErrorType errorCode;

	public:

		Thread(std::shared_ptr<State>, int);

		State* GetState() { return state.get(); }
		CodeProvider* GetCodeProvider() { return state->GetCodeProvider(); }

		int GetCodeIndex() { return codeindex; }
		void SetCodeIndex(int newindex) { codeindex = newindex; }
		void AddCodeIndex(int offset) { codeindex += offset; }
		int GetWaitCount() { return waitcount; }
		ReturnState WaitThread(int count) { waitcount = count; return Wait; }

		ErrorType GetErrorCode() { return errorCode; }
		void SetErrorCode(ErrorType e) { errorCode = e; }

		/// <summary>スクリプトを実行する。nowaitを指定するとReturnState::Waitで中断されなくなる。</summary>
		ReturnState Run(bool nowait = false);
		/// <summary>スレッドのスタック等を完全に削除し、指定されたコードインデックスから実行するように設定する。未指定の場合はインデックス0番から開始する。</summary>
		void Reset(int ep = 0);

		ReturnState CheckStack(unsigned int pop, unsigned int push);
		Value StackPop(int count = 1);
		Value StackPush(Value);
		Value& StackTop();
		Value& StackAt(int index);
		unsigned int StackSize();
		void ClearStack();

		ReturnState FramePush(int pass);
		ReturnState FramePop(int pass);

		ReturnState GoSub(int addr);
		ReturnState ReturnSub();

		size_t CallStackSize() { return callstack.size(); }
		size_t WorkStackSize() { return workstack.size(); }
		Value& WorkStackAt(int index) { return workstack[index]; }
	};



	namespace Loader {
		enum AttrType {
			Integer,
			Float,
			Comparer,
			NumType,
			EntryPointSymbol,
			Property,
			String,
		};

		struct CodeSkelton {
			Opcode opcode;
			AttrType type;
		};

		class Generator {
		public:

			std::unordered_map<std::string, CodeSkelton> map;
			std::vector<std::string> stringTable;

			Generator();


			Code operator ()(const std::string& sig, const std::string& attr, std::string& bindSymbol);

			bool ParseAttrAsInteger(Code& c, const std::string& attr);
			bool ParseAttrAsFloat(Code& c, const std::string& attr);
			bool ParseAttrAsComparer(Code& c, const std::string& attr);
			bool ParseAttrAsNumType(Code& c, const std::string& attr);
			bool ParseAttrAsProperty(Code& c, const std::string& attr);
			bool ParseAttrAsString(Code& c, const std::string& attr);
		};

		std::shared_ptr<CodeProvider> Load(const char* filepath, Generator& gen);
		std::shared_ptr<CodeProvider> FromString(const std::string& source, Generator& gen);

		std::shared_ptr<CodeProvider> FromCodeSet(std::vector<Code>&& codes,
												  std::unordered_map<std::string, int>&& entrypoints);
		std::shared_ptr<CodeProvider> FromCodeSet(const std::vector<Code>& codes,
												  const std::unordered_map<std::string, int>& entrypoints);
		std::shared_ptr<CodeProvider> FromCodeSet(std::vector<Code>&& codes);
		std::shared_ptr<CodeProvider> FromCodeSet(const std::vector<Code>& codes);
	}
}

#endif
