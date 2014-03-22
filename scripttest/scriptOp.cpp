#include "script.h"
#include <math.h>

inline int Opcode(float f){
	return (*((int*)&f) >> 16) & 0x7F;
}

inline short Operand(float f){
	return (short)(*((int*)&f) & 0xFFFF);
}

inline ScriptReturnState CheckStack(ScriptState *state, int pop, int push){
	if (state->workstacktop < pop) {
		state->errorCode = WorkstackUnderflow;
		return Error;
	}
	if (state->workstacksize < state->workstacktop - pop + push) {
		state->errorCode = WorkstackOverflow;
		return Error;
	}
	return ScriptReturnState::None;
}

ScriptReturnState(ScriptState::*ScriptState::optable[])(short opt) = {
	//	0x00 - 0x0F
	&ScriptState::opEnd,
	&ScriptState::opYld,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,

	//	0x10 - 0x1F
	&ScriptState::opJmp,
	&ScriptState::opCpt,
	&ScriptState::opFwd,
	&ScriptState::opRev,
	&ScriptState::opJz,
	&ScriptState::opJnz,
	&ScriptState::opJpos,
	&ScriptState::opJneg,
	&ScriptState::opJeq,
	&ScriptState::opJne,
	&ScriptState::opJgt,
	&ScriptState::opJge,
	&ScriptState::opJlt,
	&ScriptState::opJle,
	&ScriptState::opCmp,
	&ScriptState::opIs,

	//	0x20 - 0x2F
	&ScriptState::opAdd,
	&ScriptState::opAdds,
	&ScriptState::opMul,
	&ScriptState::opMuls,
	&ScriptState::opSub,
	&ScriptState::opNeg,
	&ScriptState::opDiv,
	&ScriptState::opMod,
	&ScriptState::opSin,
	&ScriptState::opCos,
	&ScriptState::opTan,
	&ScriptState::opArg,
	&ScriptState::opSqrt,
	&ScriptState::opPow,
	&ScriptState::opLog,
	&ScriptState::opLen,

	//	0x30 - 0x3F
	&ScriptState::opLod,
	&ScriptState::opSto,
	&ScriptState::opVlod,
	&ScriptState::opVsto,
	&ScriptState::opDup,
	&ScriptState::opSpps,
	&ScriptState::opDel,
	&ScriptState::opCls,
	&ScriptState::opCall,
	&ScriptState::opRet,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,

	//	0x40 - 0x4F(��)
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,

	//	0x50 - 0x5F(��)
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,

	//	0x60 - 0x6F(��)
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,

	//	0x70 - 0x7F(��)
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
	&ScriptState::opNull,
};

//	����`�̖��ߌ�
//	Stk : 0 / 0
//	Opt : ���g�p
ScriptReturnState ScriptState::opNull(short opt){
	errorCode = InvalidOpcode;
	return Error;
};
//	���f
//	Stk : # / #
//	Opt : �ҋ@�J�E���g(0�ȏ�A0�̏ꍇ�͒��f�̂ݍs��)
ScriptReturnState ScriptState::opYld(short opt){
	waitcount = opt >= 0 ? opt : 1;
	return Yield;
};
//	�I��
//	Stk : # / #
//	Opt : ���g�p
ScriptReturnState ScriptState::opEnd(short opt){
	opCls(0);
	errorCode = ScriptHasFinished;
	return Finished;
};
//	�W�����v
//	Stk : 1 / 0
//	Opt : ���g�p
ScriptReturnState ScriptState::opJmp(short opt){
	if (CheckStack(this, 1, 0)) return Error;
	workstacktop--;
	codeindex = *((int*)&workstack[workstacktop]);
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	���������W�����v
//	Stk : 2 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJeq(short opt){
	if (CheckStack(this, 2, 0)) return Error;
	workstacktop -= 2;
	if (workstack[workstacktop] == workstack[workstacktop + 1])
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	�s�������W�����v
//	Stk : 2 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJne(short opt){
	if (CheckStack(this, 2, 0)) return Error;
	workstacktop -= 2;
	if (workstack[workstacktop] != workstack[workstacktop + 1])
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	���ߏ����W�����v
//	Stk : 2 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJgt(short opt){
	if (CheckStack(this, 2, 0)) return Error;
	workstacktop -= 2;
	if (workstack[workstacktop] > workstack[workstacktop + 1])
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	�ȏ�����W�����v
//	Stk : 2 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJge(short opt){
	if (CheckStack(this, 2, 0)) return Error;
	workstacktop -= 2;
	if (workstack[workstacktop] >= workstack[workstacktop + 1])
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	���������W�����v
//	Stk : 2 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJlt(short opt){
	if (CheckStack(this, 2, 0)) return Error;
	workstacktop -= 2;
	if (workstack[workstacktop] < workstack[workstacktop + 1])
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	�ȉ������W�����v
//	Stk : 2 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJle(short opt){
	if (CheckStack(this, 2, 0)) return Error;
	workstacktop -= 2;
	if (workstack[workstacktop] <= workstack[workstacktop + 1])
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	�[�������W�����v
//	Stk : 1 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJz(short opt){
	if (CheckStack(this, 1, 0)) return Error;
	workstacktop--;
	if (workstack[workstacktop] == 0.0)
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	��[�������W�����v
//	Stk : 1 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJnz(short opt){
	if (CheckStack(this, 1, 0)) return Error;
	workstacktop--;
	if (workstack[workstacktop] != 0.0)
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	���������W�����v
//	Stk : 1 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJpos(short opt){
	if (CheckStack(this, 1, 0)) return Error;
	workstacktop--;
	if (workstack[workstacktop] > 0.0)
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	���������W�����v
//	Stk : 1 / 0
//	Opt : �W�����v�I�t�Z�b�g
ScriptReturnState ScriptState::opJneg(short opt){
	if (CheckStack(this, 1, 0)) return Error;
	workstacktop--;
	if (workstack[workstacktop] < 0.0)
		codeindex += opt;
	if (codeindex > codeareasize){
		errorCode = CodeindexOutOfRange;
		return Error;
	}
	else
		return None;
};
//	��r���Z
//	Stk : 2 / 1
//	Opt : ������e  Opt[0]:���]�t���O, Opt[1-3]:����t���O
//			0 : ����
//			2 : ����
//			4 : ����
//			6 : And(�����Ƃ�0�ȊO)
//			8 : Or(���Ȃ��Ƃ��ǂ��炩��0�ȊO)
//			A : Xor(�ǂ��炩����݂̂�0�ȊO)
ScriptReturnState ScriptState::opCmp(short opt){
	if (CheckStack(this, 2, 1)) return Error;
	workstacktop--;
	switch (opt >> 1){
	case 0:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] == workstack[workstacktop]));
		return None;
	case 1:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] > workstack[workstacktop]));
		return None;
	case 2:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] < workstack[workstacktop]));
		return None;
	case 3:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] >0 && workstack[workstacktop] > 0));
		return None;
	case 4:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] > 0 || workstack[workstacktop] > 0));
		return None;
	case 5:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] > 0) ^ (workstack[workstacktop] > 0));
		return None;
	}
	errorCode = InvalidOperand;
	return Error;
};
//	���l�����擾
//	Stk : 1 / 1
//	Opt : ������e	Opt[0]:���]�t���O�AOpt[1-3]:����t���O
//			0 : �[��
//			2 : ���̐�(0�܂܂�)
//			4 : ���̐�(0�܂܂�)
//			6 : ���̖�����
//			8 : ���̖�����
//			A : NaN
ScriptReturnState ScriptState::opIs(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	switch (opt >> 1){
	case 0:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] == 0.0));
		return None;
	case 1:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] > 0.0));
		return None;
	case 2:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] < 0.0));
		return None;
	case 3:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] > 0 && workstack[workstacktop - 1] * 2 == workstack[workstacktop - 1]));
		return None;
	case 4:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] < 0 && workstack[workstacktop - 1] * 2 == workstack[workstacktop - 1]));
		return None;
	case 5:
		workstack[workstacktop - 1] = (float)((opt & 1) ^ (workstack[workstacktop - 1] != workstack[workstacktop - 1]));
		return None;
	}
	errorCode = InvalidOperand;
	return Error;
};
//	������
//	Stk : 0 / 0
//	Opt : ����ID 0�ň�ԋ߂��t���O
ScriptReturnState ScriptState::opFwd(short opt){
	float f;
	while (Opcode(f = codearea[codeindex]) != OpcodeRet){
		if (Opcode(f) == OpcodeCpt && (opt == 0 || opt == Operand(f))){
			break;
		}
		codeindex++;
		if (codeindex == codeareasize){
			errorCode = CodeindexOutOfRange;
			return Error;
		}
	}
	codeindex--;
	return None;
};
//	�����߂�
//	Stk : 0 / 0
//	Opt : ����ID 0�ň�ԋ߂��t���O
ScriptReturnState ScriptState::opRev(short opt){
	float f;
	while (Opcode(f = codearea[codeindex]) != OpcodeRet){
		if (Opcode(f) == OpcodeCpt && (opt == 0 || opt == Operand(f))){
			break;
		}
		codeindex--;
		if (codeindex == 0){
			break;
		}
	}
	codeindex--;
	return None;
};
//	Fwd/Rev�p�`�F�b�N�|�C���g
//	Stk : 0 / 0
//	Opt : ���g�p
ScriptReturnState ScriptState::opCpt(short opt){
	return None;
};
//	���Z
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,�܂��̓������A�h���X
ScriptReturnState ScriptState::opAdd(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] += f;
	return None;
};
//	�A�����Z
//	Stk : Opt+1 / 1
//	Opt : ���Z��(1�ŕ��ʂ̉��Z)
ScriptReturnState ScriptState::opAdds(short opt){
	if (CheckStack(this, opt + 1, 1)) return Error;
	if (opt < 0){
		errorCode = InvalidOperand;
		return Error;
	}
	float f = 0;
	while (opt--){
		f += workstack[--workstacktop];
		if (workstacktop < 0){
			errorCode = WorkstackUnderflow;
			return Error;
		}
	}

	workstack[workstacktop - 1] += f;

	return None;
};
//	��Z
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,�܂��̓������A�h���X
ScriptReturnState ScriptState::opMul(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] *= f;
	return None;
};
//	�A����Z
//	Stk : Opt+1 / 1
//	Opt : ��Z��(1�ŕ��ʂ̏�Z)
ScriptReturnState ScriptState::opMuls(short opt){
	if (CheckStack(this, opt + 1, 1)) return Error;
	if (opt < 0){
		errorCode = InvalidOperand;
		return Error;
	}
	float f = 1;
	while (opt--){
		f *= workstack[--workstacktop];
		if (workstacktop < 0){
			errorCode = WorkstackUnderflow;
			return Error;
		}
	}

	workstack[workstacktop - 1] *= f;

	return None;
};
//	���Z
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,�܂��̓������A�h���X
ScriptReturnState ScriptState::opSub(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] -= f;
	return None;
};
//	�����ϊ�
//	Stk : 1 / 1
//	Opt : ���g�p
ScriptReturnState ScriptState::opNeg(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = -workstack[workstacktop - 1];
	return None;
};
//	���Z
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,�܂��̓������A�h���X
ScriptReturnState ScriptState::opDiv(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] /= f;
	return None;
};
//	��]
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,�܂��̓������A�h���X
ScriptReturnState ScriptState::opMod(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] = fmod(workstack[workstacktop - 1], f);
	return None;
};
//	����
//	Stk : 1 / 1
//	Opt : ���g�p
ScriptReturnState ScriptState::opSin(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = sin(workstack[workstacktop - 1]);
	return None;
};
//	�]��
//	Stk : 1 / 1
//	Opt : ���g�p
ScriptReturnState ScriptState::opCos(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = cos(workstack[workstacktop - 1]);
	return None;
};
//	����
//	Stk : 1 / 1
//	Opt : ���g�p
ScriptReturnState ScriptState::opTan(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = tan(workstack[workstacktop - 1]);
	return None;
};
//	�Ίp
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,�܂��̓������A�h���X
ScriptReturnState ScriptState::opArg(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] = atan2(f, workstack[workstacktop - 1]);
	return None;
};
//	������
//	Stk : 1 / 1
//	Opt : ���g�p
ScriptReturnState ScriptState::opSqrt(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = sqrt(workstack[workstacktop - 1]);
	return None;
};
//	�ݏ�
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,�܂��̓������A�h���X
ScriptReturnState ScriptState::opPow(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] = pow(workstack[workstacktop - 1], f);
	return None;
};
//	�ΐ�
//	Stk : 1 / 1 or 0 / 1
//	Opt : 0,�܂��̓������A�h���X
ScriptReturnState ScriptState::opLog(short opt){
	if (CheckStack(this, opt ? 1 : 0, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop++] = log(f);
	return None;
};
//	sqrt(a^2 + b^2 + ...)���v�Z����
//	Stk : Opt / 1
//	Opt : �����̐�
ScriptReturnState ScriptState::opLen(short opt){
	if (CheckStack(this, opt, 1)) return Error;
	if (opt < 0){
		errorCode = InvalidOperand;
		return Error;
	}
	double d = 0;
	float f;
	while (opt--){
		f = workstack[--workstacktop];
		d += f * f;
		if (workstacktop < 0){
			errorCode = WorkstackUnderflow;
			return Error;
		}
	}

	workstack[workstacktop++] = (float)sqrt(d);

	return None;
};
//	�ϐ��ǂݏo��(�萔�A�h���X)
//	Stk : 0 / 1
//	Opt : �ϐ��Ԓn
ScriptReturnState ScriptState::opLod(short opt){
	if (CheckStack(this, 0, 1)) return Error;
	workstack[workstacktop++] = workarea[opt];
	return None;
};
//	�ϐ���������(�萔�A�h���X)
//	Stk : 1 / 0
//	Opt : �ϐ��Ԓn
ScriptReturnState ScriptState::opSto(short opt){
	if (CheckStack(this, 1, 0)) return Error;
	workarea[opt] = workstack[--workstacktop];
	return None;
};
//	�ϐ��ǂݍ���(�σA�h���X)
//	Stk : 1 / 1
//	Opt : ���g�p
ScriptReturnState ScriptState::opVlod(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	int index = (int)workstack[workstacktop - 1];
	if (index < 0 || workstacksize <= index){
		errorCode = InvalidOperand;
		return Error;
	}

	workstack[workstacktop - 1] = workarea[index];

	return None;
};
//	�ϐ���������(�σA�h���X)
//	Stk : 2 / 0
//	Opt : ���g�p
ScriptReturnState ScriptState::opVsto(short opt){
	if (CheckStack(this, 2, 0)) return Error;
	int index = (int)workstack[--workstacktop];
	if (index < 0 || workstacksize <= index){
		errorCode = InvalidOperand;
		return Error;
	}

	workarea[index] = workstack[--workstacktop];

	return None;
};
//	����l�v�b�V��
//	Stk : 0 / 1
//	Opt : �ςޒl�̎��
//			0 : NaN
//			1 : ����+
//			2 :  �V -
ScriptReturnState ScriptState::opSpps(short opt){
	if (CheckStack(this, 0, 1)) return Error;
	switch (opt){
	case 0:
		((int*)workstack)[workstacktop++] = 0x7FBFFFFF;
		break;
	case 1:
		workstack[workstacktop++] = 1e18f;
		break;
	case 2:
		workstack[workstacktop++] = -1e18f;
		break;
	default:
		errorCode = InvalidOperand;
		return Error;
	};

	return None;
};
//	�X�^�b�N�g�b�v����
//	Stk : 1 / Opt + 1
//	Opt : �������鐔(0�ȉ���1�ɕ␳)
ScriptReturnState ScriptState::opDup(short opt){
	if (CheckStack(this, 1, opt + 1)) return Error;
	workstack[workstacktop] = workstack[workstacktop - 1];
	workstacktop++;
	return None;
};
//	�X�^�b�N�g�b�v�폜
//	Stk : Opt / 0
//	Opt : �폜����v�f��
ScriptReturnState ScriptState::opDel(short opt){
	if (CheckStack(this, opt, 0)) return Error;
	if (workstacktop < opt){
		errorCode = WorkstackUnderflow;
		return Error;
	}
	workstacktop -= opt;
	return None;
};
//	�X�^�b�N�N���A
//	Stk : All / 0
//	Opt : ���g�p
ScriptReturnState ScriptState::opCls(short opt){
	workstacktop = 0;
	return None;
};
//	�T�u���[�`���W�����v
//	Stk : 0 / 0
//	Opt : �W�����v��A�h���X
ScriptReturnState ScriptState::opCall(short opt){
	callstack[callstacktop++] = codeindex;
	if (callstacktop == callstacksize){
		errorCode = CallstackOverflow;
		return Error;
	}
	codeindex = opt;

	return None;
};
//	���^�[��
//	Stk : 0 / 0
//	Opt : ���g�p
ScriptReturnState ScriptState::opRet(short opt){
	if (callstacktop == 0){
		errorCode = CallstackUnderflow;
		return Error;
	}
	codeindex = callstack[--callstacktop];

	return None;
};


ScriptReturnState ScriptState::opNsAdd(short opt){
}
ScriptReturnState ScriptState::opNsSub(short opt){

}
ScriptReturnState ScriptState::opNsMul(short opt){

}
ScriptReturnState ScriptState::opNsDiv(short opt){

}
