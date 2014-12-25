#include "script.h"
#include <math.h>

namespace Script {

	float Thread::StackPop() {
		if (workstack.size() == 0) {
			// ��O�𓊂���
			throw std::domain_error{"Stack underflow."};
		}
		float ret = workstack.back();
		workstack.pop_back();
		return ret;
	}

	float Thread::StackPush(float val) {
		workstack.push_back(val);
		return val;
	}

	float& Thread::StackTop() {
		return workstack.back();
	}

	unsigned int Thread::StackSize() {
		return workstack.size();
	}

	void Thread::ClearStack() {
		workstack.clear();
	}

	inline ReturnState CheckStack(Thread *state, unsigned int pop, unsigned int push) {
		if (state->workstack.size() < pop) {
			state->errorCode = WorkstackUnderflow;
			return Error;
		}
		state->workstack.reserve(state->workstack.size() - pop + push);
		return ReturnState::None;
	}

	//	����`�̖��ߌ�
	//	Stk : 0 / 0
	//	Opt : ���g�p
	ReturnState Thread::opNull(const Code& code) {
		errorCode = InvalidOpcode;
		return Error;
	};
	//	���f
	//	Stk : # / #
	//	Opt : �ҋ@�J�E���g(0�ȏ�A0�̏ꍇ�͒��f�̂ݍs��)
	ReturnState Thread::opWait(const Code& code) {
		waitcount = code.option >= 0 ? code.option : 1;
		return Wait;
	};
	//	�I��
	//	Stk : # / #
	//	Opt : ���g�p
	ReturnState Thread::opEnd(const Code& code) {
		errorCode = ScriptHasFinished;
		return Finished;
	};

	ReturnState Thread::opGoto(const Code& code) {
		if (CheckStack(this, code.option ? 1 : 0, 0)) return Error;
		int addr = code.option ? code.option : (int)StackPop();
		codeindex = addr;

		return None;
	}

	//	�W�����v
	//	Stk : 0 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJmp(const Code& code) {
		codeindex += code.option;
		
		return None;
	};
	//	���������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJeq(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left == right)
			codeindex += code.option;
		
		return None;
	};
	//	�s�������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJne(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left != right)
			codeindex += code.option;
		
		return None;
	};
	//	���ߏ����W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJgt(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left > right)
			codeindex += code.option;
		
		return None;
	};
	//	�ȏ�����W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJge(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left >= right)
			codeindex += code.option;
		
		return None;
	};
	//	���������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJlt(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left < right)
			codeindex += code.option;
		
		return None;
	};
	//	�ȉ������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJle(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left <= right)
			codeindex += code.option;
		
		return None;
	};
	//	�[�������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJz(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		float num = StackPop();
		if (num == 0.0)
			codeindex += code.option;
		
		return None;
	};
	//	��[�������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJnz(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		float num = StackPop();
		if (num != 0.0)
			codeindex += code.option;
		
		return None;
	};
	//	���������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJpos(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		float num = StackPop();
		if (num > 0.0)
			codeindex += code.option;

		return None;
	};
	//	���������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJneg(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		float num = StackPop();
		if (num < 0.0)
			codeindex += code.option;
		
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
	ReturnState Thread::opCmp(const Code& code) {
		if (CheckStack(this, 2, 1)) return Error;
		float right = StackPop();
		float left = StackPop();
		switch (code.option >> 1) {
			case 0:
				StackPush((float)((code.option & 1) ^ (left == right)));
				return None;
			case 1:
				StackPush((float)((code.option & 1) ^ (left > right)));
				return None;
			case 2:
				StackPush((float)((code.option & 1) ^ (left < right)));
				return None;
			case 3:
				StackPush((float)((code.option & 1) ^ (left != 0 && right != 0)));
				return None;
			case 4:
				StackPush((float)((code.option & 1) ^ (left != 0 || right != 0)));
				return None;
			case 5:
				StackPush((float)((code.option & 1) ^ (left > 0) ^ (right > 0)));
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
	ReturnState Thread::opIs(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		float num = StackPop();
		switch (code.option >> 1) {
			case 0:
				StackPush((float)((code.option & 1) ^ (num == 0.0)));
				return None;
			case 1:
				StackPush((float)((code.option & 1) ^ (num > 0.0)));
				return None;
			case 2:
				StackPush((float)((code.option & 1) ^ (num < 0.0)));
				return None;
			case 3:
				StackPush((float)((code.option & 1) ^ (num > 0 && num * 2 == num)));
				return None;
			case 4:
				StackPush((float)((code.option & 1) ^ (num < 0 && num * 2 == num)));
				return None;
			case 5:
				StackPush((float)((code.option & 1) ^ (num != num)));
				return None;
		}
		errorCode = InvalidOperand;
		return Error;
	};
	//	������
	//	Stk : 0 / 0
	//	Opt : ����ID 0�ň�ԋ߂��t���O
	ReturnState Thread::opFwd(const Code& code) {
		while (true) {
			const Code& c = state->provider->Get(codeindex);
			if (c.label && (code.option == 0 || code.option == c.label)) {
				break;
			}
			codeindex++;
		}
		// ���s��Ɉ�i�ނ��߁A������1�����Ă���
		codeindex--;
		return None;
	};
	//	�����߂�
	//	Stk : 0 / 0
	//	Opt : ����ID 0�ň�ԋ߂��t���O
	ReturnState Thread::opRew(const Code& code) {
		while (true) {
			const Code& c = state->provider->Get(codeindex);
			if (c.label && (code.option == 0 || code.option == c.label)) {
				break;
			}
			codeindex--;
		}
		// ���s��Ɉ�i�ނ��߁A������1�����Ă���
		codeindex--;
		return None;
	};
#if 0
	//	Fwd/Rev�p�`�F�b�N�|�C���g
	//	Stk : 0 / 0
	//	Opt : ���g�p(��W�����v���ɃV�O�l�`���Ƃ��ė��p�����)
	ReturnState Thread::opCpt(const Code& code) {
		return None;
	};
#endif
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,�܂��̓������A�h���X
	ReturnState Thread::opAdd(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() += f;
		return None;
	};
	//	�A�����Z
	//	Stk : Opt+1 / 1
	//	Opt : ���Z��(1�ŕ��ʂ̉��Z)
	ReturnState Thread::opAdds(const Code& code) {
		if (CheckStack(this, code.option + 1, 1)) return Error;
		if (code.option < 0) {
			errorCode = InvalidOperand;
			return Error;
		}
		float f = 0;
		int count = code.option;
		while (count--) {
			f += StackPop();
			if (StackSize() < 0) {
				errorCode = WorkstackUnderflow;
				return Error;
			}
		}

		StackTop() += f;

		return None;
	};
	//	��Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,�܂��̓������A�h���X
	ReturnState Thread::opMul(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() *= f;
		return None;
	};
	//	�A����Z
	//	Stk : Opt+1 / 1
	//	Opt : ��Z��(1�ŕ��ʂ̏�Z)
	ReturnState Thread::opMuls(const Code& code) {
		if (CheckStack(this, code.option + 1, 1)) return Error;
		if (code.option < 0) {
			errorCode = InvalidOperand;
			return Error;
		}
		float f = 1;
		int count = code.option;
		while (count--) {
			f *= StackPop();
			if (StackSize() < 0) {
				errorCode = WorkstackUnderflow;
				return Error;
			}
		}

		StackTop() *= f;

		return None;
	};
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,�܂��̓������A�h���X
	ReturnState Thread::opSub(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() -= f;
		return None;
	};
	//	�����ϊ�
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState Thread::opNeg(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = -StackTop();
		return None;
	};
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,�܂��̓������A�h���X
	ReturnState Thread::opDiv(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() /= f;
		return None;
	};
	//	��]
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,�܂��̓������A�h���X
	ReturnState Thread::opMod(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() = fmod(StackTop(), f);
		return None;
	};
	//	����
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState Thread::opSin(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = sin(StackTop());
		return None;
	};
	//	�]��
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState Thread::opCos(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = cos(StackTop());
		return None;
	};
	//	����
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState Thread::opTan(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = tan(StackTop());
		return None;
	};
	//	�Ίp
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,�܂��̓������A�h���X
	ReturnState Thread::opArg(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() = atan2(f, StackTop());
		return None;
	};
	//	������
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState Thread::opSqrt(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = sqrt(StackTop());
		return None;
	};
	//	�ݏ�
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,�܂��̓������A�h���X
	ReturnState Thread::opPow(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() = pow(StackTop(), f);
		return None;
	};
	//	�ΐ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 0,�܂��̓������A�h���X
	ReturnState Thread::opLog(const Code& code) {
		if (CheckStack(this, code.option ? 1 : 0, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackPush(log(f));
		return None;
	};
	//	sqrt(a^2 + b^2 + ...)���v�Z����
	//	Stk : Opt / 1
	//	Opt : �����̐�
	ReturnState Thread::opLen(const Code& code) {
		if (CheckStack(this, code.option, 1)) return Error;
		if (code.option < 0) {
			errorCode = InvalidOperand;
			return Error;
		}
		double d = 0;
		float f;
		int count = code.option;
		while (count--) {
			f = StackPop();
			d += f * f;
			if (StackSize() < 0) {
				errorCode = WorkstackUnderflow;
				return Error;
			}
		}

		StackPush((float)sqrt(d));

		return None;
	};
	//	�ϐ��ǂݏo��(�萔�A�h���X)
	//	Stk : 0 / 1
	//	Opt : �ϐ��Ԓn
	ReturnState Thread::opLod(const Code& code) {
		if (CheckStack(this, 0, 1)) return Error;
		StackPush(state->workarea[code.option]);
		return None;
	};
	//	�ϐ���������(�萔�A�h���X)
	//	Stk : 1 / 0
	//	Opt : �ϐ��Ԓn
	ReturnState Thread::opSto(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		state->workarea[code.option] = StackPop();
		return None;
	};
	//	�ϐ��ǂݍ���(�σA�h���X)
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState Thread::opVlod(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		auto index = (unsigned int)StackTop();
		if (index < 0 || state->workarea.size() <= index) {
			errorCode = InvalidOperand;
			return Error;
		}

		StackTop() = state->workarea[index];

		return None;
	};
	//	�ϐ���������(�σA�h���X)
	//	Stk : 2 / 0
	//	Opt : ���g�p
	ReturnState Thread::opVsto(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		auto index = (unsigned int)StackPop();
		if (index < 0 || state->workarea.size() <= index) {
			errorCode = InvalidOperand;
			return Error;
		}

		state->workarea[index] = StackPop();

		return None;
	};
	//	����l�v�b�V��
	//	Stk : 0 / 1
	//	Opt : �ςޒl�̎��
	//			0 : 0
	//			1 : 1
	//			2 : -1
	//			3 : ����+
	//			4 :  �V -
	//			5 : NaN
	ReturnState Thread::opSpps(const Code& code) {
		if (CheckStack(this, 0, 1)) return Error;
		float val;

		switch (code.option) {
			case 5:
				*((uint32_t*)&val) = 0x7FBFFFFF;
				break;
			case 3:
				val = 1e18f;
				break;
			case 4:
				val = -1e18f;
				break;
			case 0:
				val = 0.0f;
				break;
			case 1:
				val = 1.0f;
				break;
			case 2:
				val = -1.0f;
				break;
			default:
				errorCode = InvalidOperand;
				return Error;
		};
		StackPush(val);

		return None;
	};
	//	�X�^�b�N�g�b�v����
	//	Stk : 1 / Opt + 1
	//	Opt : �������鐔(0�ȉ���1�ɕ␳)
	ReturnState Thread::opDup(const Code& code) {
		if (CheckStack(this, 1, code.option + 1)) return Error;
		StackPush(StackTop());
		return None;
	};
	//	�X�^�b�N�g�b�v�폜
	//	Stk : Opt / 0
	//	Opt : �폜����v�f��
	ReturnState Thread::opDel(const Code& code) {
		if (CheckStack(this, code.option, 0)) return Error;
		if (StackSize() < (unsigned int)code.option) {
			errorCode = WorkstackUnderflow;
			return Error;
		}
		StackPop();
		return None;
	};
	//	�X�^�b�N�N���A
	//	Stk : All / 0
	//	Opt : ���g�p
	ReturnState Thread::opCls(const Code& code) {
		ClearStack();
		return None;
	};
	//	�T�u���[�`���W�����v
	//	Stk : 0 / 0
	//	Opt : �W�����v��A�h���X
	ReturnState Thread::opCall(const Code& code) {
		callstack.push_back(codeindex);
		codeindex = code.option;

		return None;
	};
	//	���^�[��
	//	Stk : 0 / 0
	//	Opt : ���g�p
	ReturnState Thread::opRet(const Code& code) {
		if (callstack.size() == 0) {
			errorCode = CallstackUnderflow;
			return Error;
		}
		codeindex = callstack.back();
		callstack.pop_back();

		return None;
	};

	ReturnState Thread::opPush(const Code& code) {
		StackPush(code.val);

		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ډ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState Thread::opNsAdd(const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || state->workarea.size() <= dst || src < 0 || state->workarea.size() <= src) {
			errorCode = WorkareaOutOfRange;
			return Error;
		}
		state->workarea[dst] += state->workarea[src];
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڌ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState Thread::opNsSub(const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || state->workarea.size() <= dst || src < 0 || state->workarea.size() <= src) {
			errorCode = WorkareaOutOfRange;
			return Error;
		}
		state->workarea[dst] -= state->workarea[src];
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڏ�Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ��Z��A�h���X(0 - 255)
	//        Opt[4-7] ��Z����l�̃A�h���X(0-255)
	ReturnState Thread::opNsMul(const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || state->workarea.size() <= dst || src < 0 || state->workarea.size() <= src) {
			errorCode = WorkareaOutOfRange;
			return Error;
		}
		state->workarea[dst] *= state->workarea[src];
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڏ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState Thread::opNsDiv(const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || state->workarea.size() <= dst || src < 0 || state->workarea.size() <= src) {
			errorCode = WorkareaOutOfRange;
			return Error;
		}
		state->workarea[dst] /= state->workarea[src];
		return None;
	}
}
