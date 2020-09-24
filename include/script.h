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

	/// <summary>
	/// ��r���Z���@
	/// </summary>
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

	/// <summary>
	/// ���l����
	/// </summary>
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

	/// <summary>
	/// �v���p�e�B���o�͕���
	/// </summary>
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
	//typedef std::function<ReturnState(Thread&, const Code&)> Opcode;
	typedef ReturnState (*Opcode)(Thread&, const Code&);

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
		CodeProvider::Ptr provider;
		std::vector<Value> workarea;
		void* registry;

	public:
		State(CodeProvider::Ptr);

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

	/// <summary>
	/// ���s�R���e�L�X�g��\���N���X
	/// </summary>
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


	/// <summary>
	/// CodeProvider�C���X�^���X�𐶐�����N���X�A�֐��A�񋓑̂Ȃǂ��܂Ƃ߂閼�O���
	/// </summary>
	namespace Loader {

		/// <summary>
		/// ���ߑ������
		/// </summary>
		enum AttrType {
			Integer,
			Float,
			Comparer,
			NumType,
			EntryPointSymbol,
			Property,
			String,
		};

		/// <summary>
		/// ���s�P��
		/// </summary>
		struct CodeSkelton {
			Opcode opcode;
			AttrType type;
		};

		/// <summary>
		/// �R�[�h�����ɗp����W�F�l���[�^�[ ���߂Ȃǂ�ێ�����
		/// </summary>
		class Generator {
		public:

			/// <summary>
			/// ���߃Z�b�g ���ߖ��Ɩ��߂�Code�ЂȌ`�̑g
			/// �K�v�ɉ����ēƎ���`���߂�ǉ�����
			/// </summary>
			std::unordered_map<std::string, CodeSkelton> codeMap;

			/// <summary>
			/// ������e�[�u��
			/// </summary>
			std::vector<std::string> stringTable;

			Generator();

			/// <summary>
			/// Code��g�ݗ��Ă�
			/// </summary>
			/// <param name="sig">���ߖ�(codeMap�̃L�[�ɑΉ�)</param>
			/// <param name="attr">�����l������\��</param>
			/// <param name="outBindSymbol"></param>
			/// <returns></returns>
			Code MakeCode(const std::string& sig, const std::string& attr, std::string& outBindSymbol);

		private:
			bool ParseAttrAsInteger(Code& c, const std::string& attr);
			bool ParseAttrAsFloat(Code& c, const std::string& attr);
			bool ParseAttrAsComparer(Code& c, const std::string& attr);
			bool ParseAttrAsNumType(Code& c, const std::string& attr);
			bool ParseAttrAsProperty(Code& c, const std::string& attr);
			bool ParseAttrAsString(Code& c, const std::string& attr);
		};

		struct CodeUnit {
			Opcode opcode;
			Code::Attribute attr;
			std::string str;

			CodeUnit(Opcode op) : opcode(op), attr(), str() {};

			CodeUnit(Opcode op, float f) : opcode(op), attr(f), str() {};
			CodeUnit(Opcode op, int32_t si) : opcode(op), attr(si), str() {};
			CodeUnit(Opcode op, ComparerAttribute c) : opcode(op), attr(c), str() {};
			CodeUnit(Opcode op, NumTypeAttribute na) : opcode(op), attr(na), str() {};
			CodeUnit(Opcode op, uint32_t ui) : opcode(op), attr(ui), str() {};
			CodeUnit(Opcode op, PropertyAttribute p) : opcode(op), attr(p), str() {};
			CodeUnit(Opcode op, const std::string& s) : opcode(op), attr(), str(s) {};

			CodeUnit(const std::string& entrypointname) : opcode(nullptr), attr(), str(entrypointname) {};
		};

		/// <summary>
		/// �X�N���v�g�t�@�C������ǂݍ��݁A�R�[�h���Ԃ�
		/// </summary>
		/// <param name="filepath">�X�N���v�g�t�@�C���̃p�X</param>
		/// <param name="gen">�g�p����Generator</param>
		/// <returns>CodeProvider�C���X�^���X</returns>
		CodeProvider::Ptr Load(const char* filepath, Generator& gen);

		/// <summary>
		/// �X�N���v�g����������߂��A�R�[�h���Ԃ�
		/// </summary>
		/// <param name="source">�X�N���v�g������</param>
		/// <param name="gen">�g�p����Generator</param>
		/// <returns>CodeProvider�C���X�^���X</returns>
		CodeProvider::Ptr FromString(const std::string& source, Generator& gen);

		/// <summary>
		/// Code�̔z�񂩂�CodeProvider�C���X�^���X���쐬���Ԃ�
		/// </summary>
		/// <param name="codes">Code��vector�ŕ\����閽�ߔz��</param>
		/// <returns>CodeProvider�C���X�^���X</returns>
		CodeProvider::Ptr FromCodeSet(const Code* codes, size_t codes_length);

		/// <summary>
		/// Code�̔z�񂩂�CodeProvider�C���X�^���X���쐬���Ԃ�
		/// </summary>
		/// <param name="codes">Code�̔z��</param>
		/// <param name="entrypoints">�G���g���|�C���g���ƃC���f�b�N�X�̎���</param>
		/// <returns>CodeProvider�C���X�^���X</returns>
		CodeProvider::Ptr FromCodeSet(const Code* codes, size_t codes_length,
			                                      const std::unordered_map<std::string, int>& entrypoints);

		/// <summary>
		/// Code�̔z�񂩂�CodeProvider�C���X�^���X���쐬���Ԃ�
		/// </summary>
		/// <param name="codes">Code��vector�ŕ\����閽�ߔz��</param>
		/// <returns>CodeProvider�C���X�^���X</returns>
		CodeProvider::Ptr FromCodeSet(const std::vector<Code>& codes);

		/// <summary>
		/// Code�̔z�񂩂�CodeProvider�C���X�^���X���쐬���Ԃ�
		/// </summary>
		/// <param name="codes">Code��vector�ŕ\����閽�ߔz��</param>
		/// <param name="entrypoints">�G���g���|�C���g���ƃC���f�b�N�X�̎���</param>
		/// <returns>CodeProvider�C���X�^���X</returns>
		CodeProvider::Ptr FromCodeSet(const std::vector<Code>& codes,
                                                  const std::unordered_map<std::string, int>& entrypoints);

		/// <summary>
		/// Code�̔z�񂩂�CodeProvider�C���X�^���X���쐬���Ԃ�
		/// </summary>
		/// <param name="codes">Code��vector�ŕ\����閽�ߔz��</param>
		/// <returns>CodeProvider�C���X�^���X</returns>
		CodeProvider::Ptr FromCodeSet(std::vector<Code>&& codes);

		/// <summary>
		/// Code�̔z�񂩂�CodeProvider�C���X�^���X���쐬���Ԃ�
		/// </summary>
		/// <param name="codes">Code��vector�ŕ\����閽�ߔz��</param>
		/// <param name="entrypoints">�G���g���|�C���g���ƃC���f�b�N�X�̎���</param>
		/// <returns>CodeProvider�C���X�^���X</returns>
		CodeProvider::Ptr FromCodeSet(std::vector<Code>&& codes,
												  std::unordered_map<std::string, int>&& entrypoints);

		
		CodeProvider::Ptr FromCodeSet(const CodeUnit* codes, size_t codes_length);

		
		class Builder {
			std::vector<Code> code_array;
			std::vector<std::string> string_table;
			std::unordered_map<std::string, int> entrypoints;

			struct DifferedBind {
				size_t index;
				std::string entrypoint_name;
			};
			std::vector<DifferedBind> differed_bind_list;

		public:
			Builder();

			CodeProvider::Ptr MakeCodeProvider() const;

#define MAKEOP(name, op, attr) Builder& name ( Code::Attribute operand );
#define MAKEOP_INT(name, op) Builder& name ( int operand ); Builder& name ();
#define MAKEOP_FLOAT(name, op) Builder& name ( float operand ); Builder& name ();
#define MAKEOP_CMP(name, op) Builder& name ( ComparerAttribute operand );
#define MAKEOP_NT(name, op) Builder& name ( NumTypeAttribute operand );
#define MAKEOP_ENTRYPOINT(name, op) Builder& name ( const std::string& entrypoint_name ); Builder& Builder:: ## name ();
#define MAKEOP_PROP(name, op) Builder& name ( PropertyAttribute direction );
#define MAKEOP_STR(name, op) Builder& name ( const std::string& str ); Builder& name ();
#define MAKEOP_UNIT(name, op) Builder& name ();

#include "scriptOp.inl"

#undef MAKEOP_UNIT
#undef MAKEOP_STR
#undef MAKEOP_PROP
#undef MAKEOP_ENTRYPOINT
#undef MAKEOP_NT
#undef MAKEOP_CMP
#undef MAKEOP_FLOAT
#undef MAKEOP_INT
#undef MAKEOP

			/// <summary>
			/// ������e�[�u���ɕ������ǉ������̃C���f�b�N�X�𓾂�
			/// </summary>
			/// <param name="str"></param>
			/// <returns></returns>
			int AddString(const std::string& str);

			/// <summary>
			/// �G���g���|�C���g��ݒ肷��
			/// </summary>
			/// <param name="name">�G���g���|�C���g��</param>
			/// <param name="pos">�ݒ肷�閽�߈ʒu</param>
			/// <returns></returns>
			void AddEntryPoint(const std::string& name, int pos);


			Builder& operator ()(Opcode op);
			Builder& operator ()(Opcode op, Code::Attribute attr);
			Builder& operator ()(Opcode op, const std::string& attr, bool is_entrypoint = false);
			

			// ���݂̈ʒu�Ƀ��x����ǉ�
			Builder& operator [](const std::string& label_name);


		};
	}
}

#endif
