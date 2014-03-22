#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include <functional>

struct ScriptState;

union ScriptCode{
	float val;
	struct{
		unsigned short
			flag:1,
			exp:8,
			opid:7;
		short option:16;
	};
};

enum ScriptReturnState{
	None = 0,
	Yield,
	Wait,
	Error,
	Finished,
};

enum ScriptError{
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

enum ScriptOpcode{
	OpcodeEnd	= 0x00,	//	end : \0
	OpcodeYld	= 0x01,	//	wait
//	OpcodeXxx	= 0x02,
//	OpcodeXxx	= 0x03,
//	OpcodeXxx	= 0x04,
//	OpcodeXxx	= 0x05,
//	OpcodeXxx	= 0x06,
//	OpcodeXxx	= 0x07,
//	OpcodeXxx	= 0x08,
//	OpcodeXxx	= 0x09,
//	OpcodeXxx	= 0x0A,
//	OpcodeXxx	= 0x0B,
//	OpcodeXxx	= 0x0C,
//	OpcodeXxx	= 0x0D,
//	OpcodeXxx	= 0x0E,
//	OpcodeXxx	= 0x0F,

	OpcodeJmp	= 0x10,	//	jump
	OpcodeCpt	= 0x11,	//	:
	OpcodeFwd	= 0x12,	//	>>
	OpcodeRev	= 0x13,	//	<<
	OpcodeJz	= 0x14,	//	!?
	OpcodeJnz	= 0x15,	//	?
	OpcodeJpos	= 0x16,	//	+?
	OpcodeJneg	= 0x17,	//	-?
	OpcodeJeq	= 0x18,	//	=?
	OpcodeJne	= 0x19,	//	!?
	OpcodeJgt	= 0x1A,	//	>?
	OpcodeJge	= 0x1B,	//	>=?
	OpcodeJlt	= 0x1C,	//	<?
	OpcodeJle	= 0x1D,	//	<=?
	OpcodeCmp	= 0x1E,	//	= : != : > : >= : < : <= : and : nand : or : nor : xor : nxor
	OpcodeIs	= 0x1F,	//	is() : isnot()
						//	pos npos neg nneg zero nzero posinf nposinf neginf nneginf nan nnan

	OpcodeAdd	= 0x20,	//	+
	OpcodeAdds	= 0x21,	//	++
	OpcodeMul	= 0x22,	//	*
	OpcodeMuls	= 0x23,	//	**
	OpcodeSub	= 0x24,	//	-
	OpcodeNeg	= 0x25,	//	:-
	OpcodeDiv	= 0x26,	//	/
	OpcodeMod	= 0x27,	//	%
	OpcodeSin	= 0x28,	//	sin
	OpcodeCos	= 0x29,	//	cos
	OpcodeTan	= 0x2A,	//	tan
	OpcodeArg	= 0x2B,	//	#
	OpcodeSqrt	= 0x2C,	//	sqrt
	OpcodePow	= 0x2D,	//	^
	OpcodeLog	= 0x2E,	//	log
	OpcodeLen	= 0x2F,	//	len

	OpcodeLod	= 0x30,	//	:<
	OpcodeSto	= 0x31,	//	:>
	OpcodeVlod	= 0x32,	//	::<
	OpcodeVsto	= 0x33,	//	::>
	OpcodeDup	= 0x34,	//	..
	OpcodeSpps	= 0x35,	//	posinf : neginf : nan
	OpcodeDel	= 0x36,	//	del
	OpcodeCls	= 0x37,	//	;
	OpcodeCall	= 0x38,	//	call
	OpcodeRet	= 0x39,	//	return
//	OpcodeXxx	= 0x3A,
//	OpcodeXxx	= 0x3B,
//	OpcodeXxx	= 0x3C,
//	OpcodeXxx	= 0x3D,
//	OpcodeXxx	= 0x3E,
//	OpcodeXxx	= 0x3F,

	OpcodeNsAdd	= 0x40,
	OpcodeNsSub	= 0x41,
	OpcodeNsMul	= 0x42,
	OpcodeNsDiv	= 0x43,
//	OpcodeXxx	= 0x44,
//	OpcodeXxx	= 0x45,
//	OpcodeXxx	= 0x46,
//	OpcodeXxx	= 0x47,
//	OpcodeXxx	= 0x48,
//	OpcodeXxx	= 0x49,
//	OpcodeXxx	= 0x4A,
//	OpcodeXxx	= 0x4B,
//	OpcodeXxx	= 0x4C,
//	OpcodeXxx	= 0x4D,
//	OpcodeXxx	= 0x4E,
//	OpcodeXxx	= 0x4F,

//	OpcodeXxx	= 0x50,
//	OpcodeXxx	= 0x51,
//	OpcodeXxx	= 0x52,
//	OpcodeXxx	= 0x53,
//	OpcodeXxx	= 0x54,
//	OpcodeXxx	= 0x55,
//	OpcodeXxx	= 0x56,
//	OpcodeXxx	= 0x57,
//	OpcodeXxx	= 0x58,
//	OpcodeXxx	= 0x59,
//	OpcodeXxx	= 0x5A,
//	OpcodeXxx	= 0x5B,
//	OpcodeXxx	= 0x5C,
//	OpcodeXxx	= 0x5D,
//	OpcodeXxx	= 0x5E,
//	OpcodeXxx	= 0x5F,

//	OpcodeXxx	= 0x60,
//	OpcodeXxx	= 0x61,
//	OpcodeXxx	= 0x62,
//	OpcodeXxx	= 0x63,
//	OpcodeXxx	= 0x64,
//	OpcodeXxx	= 0x65,
//	OpcodeXxx	= 0x66,
//	OpcodeXxx	= 0x67,
//	OpcodeXxx	= 0x68,
//	OpcodeXxx	= 0x69,
//	OpcodeXxx	= 0x6A,
//	OpcodeXxx	= 0x6B,
//	OpcodeXxx	= 0x6C,
//	OpcodeXxx	= 0x6D,
//	OpcodeXxx	= 0x6E,
//	OpcodeXxx	= 0x6F,

//	OpcodeXxx	= 0x70,
//	OpcodeXxx	= 0x71,
//	OpcodeXxx	= 0x72,
//	OpcodeXxx	= 0x73,
//	OpcodeXxx	= 0x74,
//	OpcodeXxx	= 0x75,
//	OpcodeXxx	= 0x76,
//	OpcodeXxx	= 0x77,
//	OpcodeXxx	= 0x78,
//	OpcodeXxx	= 0x79,
//	OpcodeXxx	= 0x7A,
//	OpcodeXxx	= 0x7B,
//	OpcodeXxx	= 0x7C,
//	OpcodeXxx	= 0x7D,
//	OpcodeXxx	= 0x7E,
//	OpcodeXxx	= 0x7F,
};

//typedef ScriptReturnState (*ScriptExternOp) (ScriptState* state,short opt);
typedef std::function<ScriptReturnState(ScriptState*, short)> ScriptExternOp;

class ScriptCodeProvider{
public:
	virtual float* GetCodes() = 0;
	virtual int GetWorkStackSize(){return 32;};	
	virtual int GetCallStackSize(){return 32;};	
	virtual int GetWorkAreaSize(){return 256;};	
	virtual int GetCodeAreaSize(){return 4096;};	
};

struct ScriptState{
	float *workstack;
	int *callstack;
	float *workarea;
	float *codearea;

	int workstacksize;
	int callstacksize;
	int workareasize;
	int codeareasize;

	int callstacktop;
	int workstacktop;

	int codeindex;

	int waitcount;

	ScriptCodeProvider* provider;
	bool owncode;

	ScriptError errorCode;

	ScriptState(char* filename);
	ScriptState(ScriptCodeProvider* codeProvider);
	ScriptState(int wstacksize,int cstacksize, int wareasize,int codesize,ScriptCode* codes);
	~ScriptState();

	ScriptReturnState Run();

	void Reset();

	void SetExternalOpTable(ScriptExternOp *opTable);

	static ScriptReturnState NullOp(ScriptState*,short);

private:
#pragma region Operation Definitions
	ScriptReturnState opEnd(short opt);
	ScriptReturnState opYld(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);

	ScriptReturnState opJmp(short opt);
	ScriptReturnState opCpt(short opt);
	ScriptReturnState opFwd(short opt);
	ScriptReturnState opRev(short opt);
	ScriptReturnState opJz(short opt);
	ScriptReturnState opJnz(short opt);
	ScriptReturnState opJpos(short opt);
	ScriptReturnState opJneg(short opt);
	ScriptReturnState opJeq(short opt);
	ScriptReturnState opJne(short opt);
	ScriptReturnState opJgt(short opt);
	ScriptReturnState opJge(short opt);
	ScriptReturnState opJlt(short opt);
	ScriptReturnState opJle(short opt);
	ScriptReturnState opCmp(short opt);
	ScriptReturnState opIs(short opt);

	ScriptReturnState opAdd(short opt);
	ScriptReturnState opAdds(short opt);
	ScriptReturnState opMul(short opt);
	ScriptReturnState opMuls(short opt);
	ScriptReturnState opSub(short opt);
	ScriptReturnState opNeg(short opt);
	ScriptReturnState opDiv(short opt);
	ScriptReturnState opMod(short opt);
	ScriptReturnState opSin(short opt);
	ScriptReturnState opCos(short opt);
	ScriptReturnState opTan(short opt);
	ScriptReturnState opArg(short opt);
	ScriptReturnState opSqrt(short opt);
	ScriptReturnState opPow(short opt);
	ScriptReturnState opLog(short opt);
	ScriptReturnState opLen(short opt);

	ScriptReturnState opLod(short opt);
	ScriptReturnState opSto(short opt);
	ScriptReturnState opVlod(short opt);
	ScriptReturnState opVsto(short opt);
	ScriptReturnState opDup(short opt);
	ScriptReturnState opSpps(short opt);
	ScriptReturnState opDel(short opt);
	ScriptReturnState opCls(short opt);
	ScriptReturnState opCall(short opt);
	ScriptReturnState opRet(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);

	ScriptReturnState opNsAdd(short opt);
	ScriptReturnState opNsSub(short opt);
	ScriptReturnState opNsMul(short opt);
	ScriptReturnState opNsDiv(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);

//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);

//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);

//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
//	ScriptReturnState opXxx(short opt);
	ScriptReturnState opNull(short opt);
#pragma endregion

	static ScriptReturnState (ScriptState::*optable[])(short opt);
	ScriptExternOp *extOpTable;
};

#undef OPDEFINE

#endif