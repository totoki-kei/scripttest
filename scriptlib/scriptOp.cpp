#include "script.h"
#include <math.h>

namespace Script {

	Value Thread::StackPop(int n) {
		if (workstack.size() == 0) {
			// ��O�𓊂���
			throw std::domain_error{"Stack underflow."};
		}
		float ret = workstack.back();
		auto begin = workstack.end() - n;
		workstack.erase(begin, workstack.end());
		return ret;
	}

	Value Thread::StackPush(Value val) {
		workstack.push_back(val);
		return val;
	}

	Value& Thread::StackTop() {
		return workstack.back();
	}

	unsigned int Thread::StackSize() {
		return workstack.size();
	}

	void Thread::ClearStack() {
		workstack.clear();
	}

	inline ReturnState CheckStack(Thread &th, unsigned int pop, unsigned int push) {
		if (th.workstack.size() < pop) {
			th.errorCode = WorkstackUnderflow;
			return Error;
		}
		//int requiredSize = (int)state->workstack.size() - (int)pop + (int)push;
		//while (requiredSize >= state->workstack.size()) {
		//	state->workstack.resize(state->workstack.size() * 2 + 1);
		//}
		return ReturnState::None;
	}

	//	����`�̖��ߌ�
	//	Stk : 0 / 0
	//	Opt : ���g�p
	ReturnState Thread::opNull(Thread& th, const Code& code) {
		th.errorCode = InvalidOpcode;
		return Error;
	};
	//	���f
	//	Stk : # / #
	//	Opt : �ҋ@�J�E���g(0�ȏ�A0�̏ꍇ�͒��f�̂ݍs��)
	ReturnState Thread::opWait(Thread& th, const Code& code) {
		th.waitcount = code.option > 0 ? code.option : 0;
		return Wait;
	};
	//	�I��
	//	Stk : # / #
	//	Opt : ���g�p
	ReturnState Thread::opEnd(Thread& th, const Code& code) {
		th.errorCode = ScriptHasFinished;
		return Finished;
	};

	ReturnState Thread::opGoto(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 0)) return Error;
		int addr = code.option < 0 ? (int)th.StackPop().float_ : code.option;
		th.codeindex = addr - 1;

		return None;
	}

	//	�W�����v
	//	Stk : 0 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJmp(Thread& th, const Code& code) {
		if (code.option >= 0)
			th.codeindex += code.option;
		
		return None;
	};
	//	���������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJeq(Thread& th, const Code& code) {
		if (CheckStack(th, 2, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left == right)
			th.codeindex += code.option;
		
		return None;
	};
	//	�s�������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJne(Thread& th, const Code& code) {
		if (CheckStack(th, 2, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left != right)
			th.codeindex += code.option;
		
		return None;
	};
	//	���ߏ����W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJgt(Thread& th, const Code& code) {
		if (CheckStack(th, 2, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left > right)
			th.codeindex += code.option;
		
		return None;
	};
	//	�ȏ�����W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJge(Thread& th, const Code& code) {
		if (CheckStack(th, 2, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left >= right)
			th.codeindex += code.option;
		
		return None;
	};
	//	���������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJlt(Thread& th, const Code& code) {
		if (CheckStack(th, 2, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left < right)
			th.codeindex += code.option;
		
		return None;
	};
	//	�ȉ������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJle(Thread& th, const Code& code) {
		if (CheckStack(th, 2, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left <= right)
			th.codeindex += code.option;
		
		return None;
	};
	//	�[�������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJz(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float num = th.StackPop();
		if (num == 0.0)
			th.codeindex += code.option;
		
		return None;
	};
	//	��[�������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJnz(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float num = th.StackPop();
		if (num != 0.0)
			th.codeindex += code.option;
		
		return None;
	};
	//	���������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJpos(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float num = th.StackPop();
		if (num > 0.0)
			th.codeindex += code.option;

		return None;
	};
	//	���������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState Thread::opJneg(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 0)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float num = th.StackPop();
		if (num < 0.0)
			th.codeindex += code.option;
		
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
	ReturnState Thread::opCmp(Thread& th, const Code& code) {
		if (CheckStack(th, 2, 1)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		switch (code.option >> 1) {
			case 0:
				th.StackPush((float)((code.option & 1) ^ (left == right)));
				return None;
			case 1:
				th.StackPush((float)((code.option & 1) ^ (left > right)));
				return None;
			case 2:
				th.StackPush((float)((code.option & 1) ^ (left < right)));
				return None;
			case 3:
				th.StackPush((float)((code.option & 1) ^ (left != 0 && right != 0)));
				return None;
			case 4:
				th.StackPush((float)((code.option & 1) ^ (left != 0 || right != 0)));
				return None;
			case 5:
				th.StackPush((float)((code.option & 1) ^ (left > 0) ^ (right > 0)));
				return None;
		}
		th.errorCode = InvalidOperand;
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
	ReturnState Thread::opIs(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 1)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOpcode;
			return Error;
		}
		float num = th.StackPop();
		switch (code.option >> 1) {
			case 0:
				th.StackPush((float)((code.option & 1) ^ (num == 0.0)));
				return None;
			case 1:
				th.StackPush((float)((code.option & 1) ^ (num > 0.0)));
				return None;
			case 2:
				th.StackPush((float)((code.option & 1) ^ (num < 0.0)));
				return None;
			case 3:
				th.StackPush((float)((code.option & 1) ^ (num > 0 && num * 2 == num)));
				return None;
			case 4:
				th.StackPush((float)((code.option & 1) ^ (num < 0 && num * 2 == num)));
				return None;
			case 5:
				th.StackPush((float)((code.option & 1) ^ (num != num)));
				return None;
		}
		th.errorCode = InvalidOperand;
		return Error;
	};
	//	������
	//	Stk : 0 / 0
	//	Opt : ����ID 0�ň�ԋ߂��t���O
	ReturnState Thread::opFwd(Thread& th, const Code& code) {
		while (0 <= th.codeindex && th.codeindex < th.state->provider->Length()) {
			const Code& c = th.state->provider->Get(th.codeindex);
			if (c.label && (code.option == 0 || code.option == c.label)) {
				break;
			}
			th.codeindex++;
		}
		// ���s��Ɉ�i�ނ��߁A������1�����Ă���
		th.codeindex--;
		return None;
	};
	//	�����߂�
	//	Stk : 0 / 0
	//	Opt : ����ID 0�ň�ԋ߂��t���O
	ReturnState Thread::opRew(Thread& th, const Code& code) {
		while (0 <= th.codeindex && th.codeindex < th.state->provider->Length()) {
			const Code& c = th.state->provider->Get(th.codeindex);
			if (c.label && (code.option <= 0 || code.option == c.label)) {
				break;
			}
			th.codeindex--;
		}
		// ���s��Ɉ�i�ނ��߁A������1�����Ă���
		th.codeindex--;
		return None;
	};
#if 0
	//	Fwd/Rev�p�`�F�b�N�|�C���g
	//	Stk : 0 / 0
	//	Opt : ���g�p(��W�����v���ɃV�O�l�`���Ƃ��ė��p�����)
	ReturnState Thread::opCpt(Thread& th, const Code& code) {
		return None;
	};
#endif
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opAdd(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ += f;
		return None;
	};
	//	�A�����Z
	//	Stk : Opt+1 / 1
	//	Opt : ���Z��(1�ŕ��ʂ̉��Z)
	ReturnState Thread::opAdds(Thread& th, const Code& code) {
		if (CheckStack(th, code.option + 1, 1)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOperand;
			return Error;
		}
		float f = 0;
		int count = code.option;
		while (count--) {
			f += th.StackPop().float_;
			if (th.StackSize() < 0) {
				th.errorCode = WorkstackUnderflow;
				return Error;
			}
		}

		th.StackTop().float_ += f;

		return None;
	};
	//	��Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opMul(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ *= f;
		return None;
	};
	//	�A����Z
	//	Stk : Opt+1 / 1
	//	Opt : ��Z��(1�ŕ��ʂ̏�Z)
	ReturnState Thread::opMuls(Thread& th, const Code& code) {
		if (CheckStack(th, code.option + 1, 1)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOperand;
			return Error;
		}
		float f = 1;
		int count = code.option;
		while (count--) {
			f *= th.StackPop().float_;
			if (th.StackSize() < 0) {
				th.errorCode = WorkstackUnderflow;
				return Error;
			}
		}

		th.StackTop().float_ *= f;

		return None;
	};
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opSub(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option < 0 ? th.state->workarea[code.option] : th.StackPop());
		th.StackTop().float_ -= f;
		return None;
	};
	//	�����ϊ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opNeg(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(-f);
		return None;
	};
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opDiv(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ /= f;
		return None;
	};
	//	��]
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opMod(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = fmod(th.StackTop(), f);
		return None;
	};
	//	����
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opSin(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sin(f));
		return None;
	};
	//	�]��
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opCos(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(cos(f));
		return None;
	};
	//	����
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opTan(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(tan(f));
		return None;
	};
	//	�Ίp
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opArg(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = atan2(f, th.StackTop());
		return None;
	};
	//	������
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opSqrt(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sqrt(f));
		return None;
	};
	//	�ݏ�
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opPow(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = pow(th.StackTop(), f);
		return None;
	};
	//	���R�ΐ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opLog(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log(f));
		return None;
	};
	//	��p�ΐ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState Thread::opLog10(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log10(f));
		return None;
	};

	//	sqrt(a^2 + b^2 + ...)���v�Z����
	//	Stk : Opt / 1
	//	Opt : �����̐�
	ReturnState Thread::opLen(Thread& th, const Code& code) {
		if (CheckStack(th, code.option, 1)) return Error;
		if (code.option < 0) {
			th.errorCode = InvalidOperand;
			return Error;
		}
		double d = 0;
		float f;
		int count = code.option;
		while (count--) {
			f = th.StackPop();
			d += f * f;
			if (th.StackSize() < 0) {
				th.errorCode = WorkstackUnderflow;
				return Error;
			}
		}

		th.StackPush((float)sqrt(d));

		return None;
	};
	//	�ϐ��ǂݏo��(�萔�A�h���X)
	//	Stk : 0 / 1
	//	Opt : �ϐ��Ԓn
	ReturnState Thread::opLod(Thread& th, const Code& code) {
		if (CheckStack(th, 0, 1)) return Error;
		th.StackPush(th.state->workarea[code.option]);
		return None;
	};
	//	�ϐ���������(�萔�A�h���X)
	//	Stk : 1 / 0
	//	Opt : �ϐ��Ԓn
	ReturnState Thread::opSto(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 0)) return Error;
		th.state->workarea[code.option] = th.StackPop();
		return None;
	};
	//	�ϐ��ǂݍ���(�σA�h���X)
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState Thread::opVlod(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 1)) return Error;
		auto index = (unsigned int)th.StackTop().float_;
		if (index < 0 || th.state->workarea.size() <= index) {
			th.errorCode = InvalidOperand;
			return Error;
		}

		th.StackTop() = th.state->workarea[index];

		return None;
	};
	//	�ϐ���������(�σA�h���X)
	//	Stk : 2 / 0
	//	Opt : ���g�p
	ReturnState Thread::opVsto(Thread& th, const Code& code) {
		if (CheckStack(th, 2, 0)) return Error;
		auto index = (unsigned int)th.StackPop().float_;
		if (index < 0 || th.state->workarea.size() <= index) {
			th.errorCode = InvalidOperand;
			return Error;
		}

		th.state->workarea[index] = th.StackPop();

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
	ReturnState Thread::opSpps(Thread& th, const Code& code) {
		if (CheckStack(th, 0, 1)) return Error;
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
				th.errorCode = InvalidOperand;
				return Error;
		};
		th.StackPush(val);

		return None;
	};
	//	�X�^�b�N�g�b�v����
	//	Stk : 1 / Opt + 1
	//	Opt : �������鐔(0�ȉ���1�ɕ␳)
	ReturnState Thread::opDup(Thread& th, const Code& code) {
		if (CheckStack(th, 1, code.option + 1)) return Error;
		th.StackPush(th.StackTop());
		return None;
	};
	//	�X�^�b�N�g�b�v�폜
	//	Stk : Opt / 0
	//	Opt : �폜����v�f��
	ReturnState Thread::opDel(Thread& th, const Code& code) {
		auto n = code.option < 0 ? 1 : code.option;
		if (CheckStack(th, n, 0)) return Error;
		th.StackPop(n);
		return None;
	};
	//	�X�^�b�N�N���A
	//	Stk : All / 0
	//	Opt : ���g�p
	ReturnState Thread::opCls(Thread& th, const Code& code) {
		th.ClearStack();
		return None;
	};
	//	�T�u���[�`���W�����v
	//	Stk : 0 / 0
	//	Opt : �W�����v��A�h���X
	ReturnState Thread::opCall(Thread& th, const Code& code) {
		th.callstack.push_back(th.codeindex);
		th.codeindex = code.option - 1;

		return None;
	};
	//	���^�[��
	//	Stk : 0 / 0
	//	Opt : ���g�p
	ReturnState Thread::opRet(Thread& th, const Code& code) {
		if (th.callstack.size() == 0) {
			th.errorCode = CallstackUnderflow;
			return Error;
		}
		th.codeindex = th.callstack.back();
		th.callstack.pop_back();

		return None;
	};

	ReturnState Thread::opPush(Thread& th, const Code& code) {
		th.StackPush(code.val);

		return None;
	}
	
	// �萔�A�h���X�ϐ��Ԃ̒��ډ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState Thread::opNsAdd(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.state->workarea.size() <= dst || src < 0 || th.state->workarea.size() <= src) {
			th.errorCode = WorkareaOutOfRange;
			return Error;
		}
		th.state->workarea[dst].float_ += th.state->workarea[src].float_;
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڌ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState Thread::opNsSub(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.state->workarea.size() <= dst || src < 0 || th.state->workarea.size() <= src) {
			th.errorCode = WorkareaOutOfRange;
			return Error;
		}
		th.state->workarea[dst].float_ -= th.state->workarea[src].float_;
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڏ�Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ��Z��A�h���X(0 - 255)
	//        Opt[4-7] ��Z����l�̃A�h���X(0-255)
	ReturnState Thread::opNsMul(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.state->workarea.size() <= dst || src < 0 || th.state->workarea.size() <= src) {
			th.errorCode = WorkareaOutOfRange;
			return Error;
		}
		th.state->workarea[dst].float_ *= th.state->workarea[src].float_;
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڏ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState Thread::opNsDiv(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.state->workarea.size() <= dst || src < 0 || th.state->workarea.size() <= src) {
			th.errorCode = WorkareaOutOfRange;
			return Error;
		}
		th.state->workarea[dst].float_ /= th.state->workarea[src].float_;
		return None;
	}
}
