#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <stdint.h>

namespace Script {
	enum ReturnState {
		None = 0,
		Wait,
		Error,
		Finished,
	};

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

	typedef std::function<ReturnState(Thread&, const Code&)> Opcode;
	inline Opcode ToOpcode(ReturnState(Thread::*memfn)(const Code&)) {
		return std::bind(memfn, std::placeholders::_1, std::placeholders::_2);
	}

	struct Code {
		int label;
		Opcode opcode;

		union {
			float val;
			int32_t option;
		};

		Code() : label{ 0 }, opcode{ nullptr }, option{ -1 } { }
		Code(const Code&) = default;

		template<typename Fn>
		Code(Fn f) : label{ 0 }, opcode{ f }, option{ -1 } {}
		template<typename Fn>
		Code(Fn f, float n) : label{ 0 }, opcode{ f }, val{ n } {}
		template<typename Fn>
		Code(Fn f, int32_t i) : label{ 0 }, opcode{ f }, option{ i } {}

		Code(ReturnState(Thread::*memfn)(const Code&))
			: label{ 0 }
			, opcode{ ToOpcode(memfn) }
			, option{ -1 } {}
		Code(ReturnState(Thread::*memfn)(const Code&), float n)
			: label{ 0 }
			, opcode{ ToOpcode(memfn) }
			, val{ n } {}
		Code(ReturnState(Thread::*memfn)(const Code&), int32_t i)
			: label{ 0 }
			, opcode{ ToOpcode(memfn) }
			, option{ i } {}
	};

	//typedef ScriptReturnState (*ScriptExternOp) (ScriptState* state,short opt);
	//typedef std::function<ReturnState(State*, short)> ExternOp;

	class CodeProvider : public std::enable_shared_from_this<CodeProvider> {
	public:
		typedef std::shared_ptr<CodeProvider> Ptr;

		virtual ~CodeProvider();

		virtual const Code& Get(int index) = 0;
		virtual int Length() = 0;
		virtual int EntryPoint(const char* name) = 0;

		virtual std::shared_ptr<State> CreateState();
	};

	struct State : public std::enable_shared_from_this<State> {
		typedef std::shared_ptr<State> Ptr;

		std::shared_ptr<CodeProvider> provider;
		std::vector<Value> workarea;

		State(std::shared_ptr<CodeProvider>);
		~State();

		std::shared_ptr<Thread> CreateThread(int entryPoint = 0);
		std::shared_ptr<Thread> CreateThread(const char* entryPoint);

		void Reset();
	};

	struct Thread {
		typedef std::shared_ptr<Thread> Ptr;

		std::shared_ptr<State> state;

		std::vector<Value> workstack;
		std::vector<int> callstack;
		
		int codeindex;
		int waitcount;
		ErrorType errorCode;

		Thread(std::shared_ptr<State>, int);

		ReturnState Run();

#pragma region Operation Definitions
		ReturnState opEnd(const Code& code);
		ReturnState opWait(const Code& code);

		ReturnState opGoto(const Code& code);
		ReturnState opJmp(const Code& code);
		//ReturnState opCpt(const Code& code);
		ReturnState opFwd(const Code& code);
		ReturnState opRew(const Code& code);
		ReturnState opJz(const Code& code);
		ReturnState opJnz(const Code& code);
		ReturnState opJpos(const Code& code);
		ReturnState opJneg(const Code& code);
		ReturnState opJeq(const Code& code);
		ReturnState opJne(const Code& code);
		ReturnState opJgt(const Code& code);
		ReturnState opJge(const Code& code);
		ReturnState opJlt(const Code& code);
		ReturnState opJle(const Code& code);
		ReturnState opCmp(const Code& code);
		ReturnState opIs(const Code& code);

		ReturnState opAdd(const Code& code);
		ReturnState opAdds(const Code& code);
		ReturnState opMul(const Code& code);
		ReturnState opMuls(const Code& code);
		ReturnState opSub(const Code& code);
		ReturnState opNeg(const Code& code);
		ReturnState opDiv(const Code& code);
		ReturnState opMod(const Code& code);
		ReturnState opSin(const Code& code);
		ReturnState opCos(const Code& code);
		ReturnState opTan(const Code& code);
		ReturnState opArg(const Code& code);
		ReturnState opSqrt(const Code& code);
		ReturnState opPow(const Code& code);
		ReturnState opLog(const Code& code);
		ReturnState opLog10(const Code& code);
		ReturnState opLen(const Code& code);

		ReturnState opLod(const Code& code);
		ReturnState opSto(const Code& code);
		ReturnState opVlod(const Code& code);
		ReturnState opVsto(const Code& code);
		ReturnState opDup(const Code& code);
		ReturnState opSpps(const Code& code);
		ReturnState opDel(const Code& code);
		ReturnState opCls(const Code& code);
		ReturnState opCall(const Code& code);
		ReturnState opRet(const Code& code);

		ReturnState opPush(const Code& code);

		ReturnState opNsAdd(const Code& code);
		ReturnState opNsSub(const Code& code);
		ReturnState opNsMul(const Code& code);
		ReturnState opNsDiv(const Code& code);

		ReturnState opNull(const Code& code);
#pragma endregion

		Value StackPop();
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
	}
}

#endif
