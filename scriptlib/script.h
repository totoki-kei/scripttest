#ifndef SCRIPT_H_
#define SCRIPT_H_

#include <functional>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stdint.h>

namespace Script {

	/// <summary>Code�̎��s���ʂ�\���񋓑�</summary>
	enum ReturnState {
		/// <summary>����Ɋ���</summary>
		None = 0,
		/// <summary>�X�N���v�g�𒆒f����</summary>
		Wait,
		/// <summary>�G���[����������</summary>
		/// <remark>�����Ԃ��ꍇ�́AThread��errorCode�����o�ɃG���[���R���i�[���邱�ƁB</remark>
		Error,
		/// <summary>�X�N���v�g���I������</summary>
		Finished,
	};

	/// <summary>�X�N���v�g�G���[�̏��</summary>
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

	/// <summary>���[�N�X�^�b�N�ƃ��[�N�G���A�Ɋi�[�����l�̒P�ʁB�P���x���������_���ƕ����t��32�r�b�g�����A�^�����|�C���^�̋��p�́B</summary>
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

	/// <summary>�X�N���v�g�̏����̈�P�ʁB</summary>
	typedef std::function<ReturnState(Thread&, const Code&)> Opcode;

	/// <summary>���s�̍ŏ��P�ʁB���������s��Opcode�ƒǉ��̃I�v�V�����l����Ȃ�B���������ɉ����I�v�V�������w�肵�Ȃ������ꍇ�A����-1���ݒ肳���B</summary>
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

	/// <summary>��A�̃R�[�h�Q��񋟂���N���X</summary>
	class CodeProvider : public std::enable_shared_from_this<CodeProvider> {
	public:
		/// <summary>�|�C���^�^(shared_ptr)</summary>
		typedef std::shared_ptr<CodeProvider> Ptr;

		/// <summary>�w��C���f�b�N�X��Code�𓾂�</summary>
		virtual const Code& Get(int index) = 0;
		/// <summary>�R�[�h�Q�̃T�C�Y�𓾂�B</summary>
		virtual int Length() = 0;
		/// <summary>�w�肳�ꂽ���̂Ɋ֘A�t����ꂽ�R�[�h�C���f�b�N�X�𓾂�B���݂��Ȃ��ꍇ��-1��Ԃ��B</summary>
		virtual int Label(const char* name) = 0;
		/// <summary>������e�[�u�����當����𓾂�B���݂��Ȃ�ID�̏ꍇ��nullptr��Ԃ��B</summary>
		virtual const char* GetString(int id) = 0;

		/// <summary>State���쐬����</summary>
		virtual std::shared_ptr<State> CreateState();
	};

	/// <summary>���s���̃O���[�o���ȏ����i�[����N���X</summary>
	class State : public std::enable_shared_from_this<State> {
	public:
		typedef std::shared_ptr<State> Ptr;

	protected:
		std::shared_ptr<CodeProvider> provider;
		std::vector<Value> workarea;
		void* registry;

	public:
		State(std::shared_ptr<CodeProvider>);

		/// <summary>�w��̃R�[�h�C���f�b�N�X����J�n����X���b�h���쐬����B���w��̏ꍇ�̓C���f�b�N�X0�Ԃ���J�n����B</summary>
		std::shared_ptr<Thread> CreateThread(int entryPoint = 0);
		/// <summary>�w�薼�̂̃R�[�h�C���f�b�N�X����J�n����X���b�h���쐬����B</summary>
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


		/// <summary>���[�N�G���A�𐮐��l0�ŏ���������B</summary>
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

		/// <summary>�X�N���v�g�����s����Bnowait���w�肷���ReturnState::Wait�Œ��f����Ȃ��Ȃ�B</summary>
		ReturnState Run(bool nowait = false);
		/// <summary>�X���b�h�̃X�^�b�N�������S�ɍ폜���A�w�肳�ꂽ�R�[�h�C���f�b�N�X������s����悤�ɐݒ肷��B���w��̏ꍇ�̓C���f�b�N�X0�Ԃ���J�n����B</summary>
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
