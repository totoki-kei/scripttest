#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>

namespace Script {
	/**
	* <summery>Code�̎��s���ʂ�\���񋓑�</summery>
	*/
	enum ReturnState {
		/// <summery>����Ɋ���</summery>
		None = 0,
		/// <summery>�X�N���v�g�𒆒f����</summery>
		Wait,
		/// <summery>�G���[����������</summery>
		/// <remark>�����Ԃ��ꍇ�́AThread��errorCode�����o�ɃG���[���R���i�[���邱�ƁB</remark>
		Error,
		/// <summery>�X�N���v�g���I������</summery>
		Finished,
	};

	/**
	* <summery>�X�N���v�g�G���[�̏��</summery>
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
	class State;
	class Thread;

	/// <summery>���[�N�X�^�b�N�ƃ��[�N�G���A�Ɋi�[�����l�̒P�ʁB�P���x���������_���ƕ����t��32�r�b�g�����A�^�����|�C���^�̋��p�́B</summery>
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

	/// <summery>�X�N���v�g�̏����̈�P�ʁB</summery>
	typedef std::function<ReturnState(Thread&, const Code&)> Opcode;

	/// <summery>���s�̍ŏ��P�ʁB���������s��Opcode�ƒǉ��̃I�v�V�����l����Ȃ�B���������ɉ����I�v�V�������w�肵�Ȃ������ꍇ�A����-1���ݒ肳���B</summery>
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
		template<typename Fn>
		Code(Fn f, const char* s) : opcode{ f }, str{ s } {}

	};

	/// <summery>��A�̃R�[�h�Q��񋟂���N���X</summery>
	class CodeProvider : public std::enable_shared_from_this<CodeProvider> {
	public:
		typedef std::shared_ptr<CodeProvider> Ptr;

		/// <summery>�w��C���f�b�N�X��Code�𓾂�</summery>
		virtual const Code& Get(int index) = 0;
		/// <summery>�R�[�h�Q�̃T�C�Y�𓾂�B</summery>
		virtual int Length() = 0;
		/// <summery>�w�肳�ꂽ���̂Ɋ֘A�t����ꂽ�R�[�h�C���f�b�N�X�𓾂�B���݂��Ȃ��ꍇ��-1��Ԃ��B</summery>
		virtual int Label(const char* name) = 0;
		/// <summery>������e�[�u�����當����𓾂�B���݂��Ȃ�ID�̏ꍇ��nullptr��Ԃ��B</summery>
		virtual const char* GetString(int id) = 0;

		/// <summery>State���쐬����</summery>
		virtual std::shared_ptr<State> CreateState();
	};

	/// <summery>���s���̃O���[�o���ȏ����i�[����N���X</summery>
	class State : public std::enable_shared_from_this<State> {
	public:
		typedef std::shared_ptr<State> Ptr;

	protected:
		std::shared_ptr<CodeProvider> provider;
		std::vector<Value> workarea;
		void* registry;

	public:
		State(std::shared_ptr<CodeProvider>);

		/// <summery>�w��̃R�[�h�C���f�b�N�X����J�n����X���b�h���쐬����B���w��̏ꍇ�̓C���f�b�N�X0�Ԃ���J�n����B</summery>
		std::shared_ptr<Thread> CreateThread(int entryPoint = 0);
		/// <summery>�w�薼�̂̃R�[�h�C���f�b�N�X����J�n����X���b�h���쐬����B</summery>
		std::shared_ptr<Thread> CreateThread(const char* entryPoint);

		CodeProvider* GetCodeProvider() { return provider.get(); }

		Value& At(int index) { return workarea[index]; }
		size_t Count() { return workarea.size(); }

		void * GetRegistry() { return registry; }
		void SetRegistry(void* ptr) { registry = ptr; }


		/// <summery>���[�N�G���A�𐮐��l0�ŏ���������B</summery>
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

		/// <summery>�X�N���v�g�����s����Bnowait���w�肷���ReturnState::Wait�Œ��f����Ȃ��Ȃ�B</summery>
		ReturnState Run(bool nowait = false);
		/// <summery>�X���b�h�̃X�^�b�N�������S�ɍ폜���A�w�肳�ꂽ�R�[�h�C���f�b�N�X������s����悤�ɐݒ肷��B���w��̏ꍇ�̓C���f�b�N�X0�Ԃ���J�n����B</summery>
		void Reset(int ep = 0);

		ReturnState CheckStack(unsigned int pop, unsigned int push);
		Value StackPop(int count = 1);
		Value StackPush(Value);
		Value& StackTop();
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
			SpecialNumbers,
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
			bool ParseAttrAsSpecialNumbers(Code& c, const std::string& attr);
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
