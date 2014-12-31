#include "script.h"
#include "scriptOp.h"

#include <math.h>

namespace Script {

	Value Thread::StackPop(int n) {
		if (workstack.size() <= stackBase ) {
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
		if (workstack.size() <= stackBase) {
			// ��O�𓊂���
			throw std::domain_error{ "Stack underflow." };
		}
		return workstack.back();
	}

	unsigned int Thread::StackSize() {
		return workstack.size() - stackBase;
	}

	void Thread::ClearStack() {
		workstack.erase(workstack.begin() + stackBase, workstack.end());
	}

	inline ReturnState CheckStack(Thread &th, unsigned int pop, unsigned int /* push ( unused ) */) {
		if (th.StackSize() < pop) {
			th.errorCode = WorkstackUnderflow;
			return Error;
		}
		//int requiredSize = (int)state->workstack.size() - (int)pop + (int)push;
		//while (requiredSize >= state->workstack.size()) {
		//	state->workstack.resize(state->workstack.size() * 2 + 1);
		//}
		return ReturnState::None;
	}

	bool Thread::FramePush(int n) {
		if (CheckStack(*this, n, 0)) return true;
		size_t newbase = workstack.size() - n + 1;
		workstack.insert(workstack.begin() + newbase - 1, (int)stackBase);
		stackBase = newbase;
		return false;
	}

	bool Thread::FramePop(int n) {
		if (stackBase == 0) {
			this->errorCode = WorkstackUnderflow;
			return true;
		}
		if (CheckStack(*this, n, 0)) return true;

		auto iter = workstack.begin() + stackBase - 1;
		size_t restorebase = (size_t)iter->int_;
		workstack.erase(iter, workstack.end() - n);
		stackBase = restorebase;
		return false;
	}



	//	����`�̖��ߌ�
	//	Stk : 0 / 0
	//	Opt : ���g�p
	ReturnState opNull(Thread& th, const Code& code) {
		th.errorCode = InvalidOpcode;
		return Error;
	};
	//	���f
	//	Stk : # / #
	//	Opt : �ҋ@�J�E���g(0�ȏ�A0�̏ꍇ�͒��f�̂ݍs��)
	ReturnState opWait(Thread& th, const Code& code) {
		th.waitcount = code.option > 0 ? code.option : 0;
		return Wait;
	};
	//	�I��
	//	Stk : # / #
	//	Opt : ���g�p
	ReturnState opEnd(Thread& th, const Code& code) {
		th.errorCode = ScriptHasFinished;
		return Finished;
	};

	//	��Έʒu�ւ̃W�����v
	//	Stk : 1 / 0 or 0 / 0
	//	Opt : -1�A�܂��̓W�����v�ʒu
	ReturnState opGoto(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 0)) return Error;
		int addr = code.option < 0 ? (int)th.StackPop().float_ : code.option;
		th.codeindex = addr - 1;

		return None;
	}

	//	�W�����v
	//	Stk : 0 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJmp(Thread& th, const Code& code) {
		if (code.option >= 0)
			th.codeindex += code.option;
		
		return None;
	};
	//	���������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJeq(Thread& th, const Code& code) {
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
	ReturnState opJne(Thread& th, const Code& code) {
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
	ReturnState opJgt(Thread& th, const Code& code) {
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
	ReturnState opJge(Thread& th, const Code& code) {
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
	ReturnState opJlt(Thread& th, const Code& code) {
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
	ReturnState opJle(Thread& th, const Code& code) {
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
	ReturnState opJz(Thread& th, const Code& code) {
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
	ReturnState opJnz(Thread& th, const Code& code) {
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
	ReturnState opJpos(Thread& th, const Code& code) {
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
	ReturnState opJneg(Thread& th, const Code& code) {
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
	ReturnState opCmp(Thread& th, const Code& code) {
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
	ReturnState opIs(Thread& th, const Code& code) {
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
	//	Opt : ����ID -1�ň�ԋ߂��t���O
	ReturnState opFwd(Thread& th, const Code& code) {
		while (0 <= th.codeindex && th.codeindex < th.state->provider->Length()) {
			const Code& c = th.state->provider->Get(th.codeindex);
			if (!c.opcode && (code.option < 0 || code.option == c.option)) {
				break;
			}
			th.codeindex++;
		}
		// ���s��Ɉ�i�ނ��A�w���ꂽ�R�[�h�͉������s���Ȃ��R�[�h�Ȃ̂ŁA�X�L�b�v����Ă��x��͂Ȃ��B
		return None;
	};
	//	�����߂�
	//	Stk : 0 / 0
	//	Opt : ����ID -1�ň�ԋ߂��t���O
	ReturnState opRew(Thread& th, const Code& code) {
		while (0 <= th.codeindex && th.codeindex < th.state->provider->Length()) {
			const Code& c = th.state->provider->Get(th.codeindex);
			if (!c.opcode && (code.option < 0 || code.option == c.option)) {
				break;
			}
			th.codeindex--;
		}
		// ���s��Ɉ�i�ނ��A�w���ꂽ�R�[�h�͉������s���Ȃ��R�[�h�Ȃ̂ŁA�X�L�b�v����Ă��x��͂Ȃ��B
		return None;
	};
#if 0
	//	Fwd/Rev�p�`�F�b�N�|�C���g
	//	Stk : 0 / 0
	//	Opt : ���g�p(��W�����v���ɃV�O�l�`���Ƃ��ė��p�����)
	ReturnState opCpt(Thread& th, const Code& code) {
		return None;
	};
#endif
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opAdd(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ += f;
		return None;
	};
	//	�A�����Z
	//	Stk : Opt+1 / 1
	//	Opt : ���Z��(1�ŕ��ʂ̉��Z) �� �v�f���ł͂Ȃ�
	ReturnState opAdds(Thread& th, const Code& code) {
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
	ReturnState opMul(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ *= f;
		return None;
	};
	//	�A����Z
	//	Stk : Opt+1 / 1
	//	Opt : ��Z��(1�ŕ��ʂ̏�Z) �� �v�f���ł͂Ȃ�
	ReturnState opMuls(Thread& th, const Code& code) {
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
	ReturnState opSub(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option < 0 ? th.state->workarea[code.option] : th.StackPop());
		th.StackTop().float_ -= f;
		return None;
	};
	//	�����ϊ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opNeg(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(-f);
		return None;
	};
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opDiv(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ /= f;
		return None;
	};
	//	��]
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opMod(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = fmod(th.StackTop(), f);
		return None;
	};
	//	����
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opSin(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sin(f));
		return None;
	};
	//	�]��
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opCos(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(cos(f));
		return None;
	};
	//	����
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opTan(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(tan(f));
		return None;
	};
	//	�Ίp
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opArg(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = atan2(f, th.StackTop());
		return None;
	};
	//	������
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opSqrt(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sqrt(f));
		return None;
	};
	//	�ݏ�
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opPow(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = pow(th.StackTop(), f);
		return None;
	};
	//	���R�ΐ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opLog(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log(f));
		return None;
	};
	//	��p�ΐ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opLog10(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log10(f));
		return None;
	};

	//	sqrt(a^2 + b^2 + ...)���v�Z����
	//	Stk : Opt / 1
	//	Opt : �����̐�
	ReturnState opLen(Thread& th, const Code& code) {
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

	//	�x���@ -> �ʓx�@
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opD2r(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(3.1415926535898f * f / 180);
		return None;
	};

	//	�ʓx�@ -> �x���@
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opR2d(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(180 * f / 3.1415926535898f);
		return None;
	};


	//	�ϐ��ǂݏo��(�萔�A�h���X)
	//	Stk : 0 / 1
	//	Opt : �ϐ��Ԓn
	ReturnState opLod(Thread& th, const Code& code) {
		if (CheckStack(th, 0, 1)) return Error;
		th.StackPush(th.state->workarea[code.option]);
		return None;
	};
	//	�ϐ���������(�萔�A�h���X)
	//	Stk : 1 / 0
	//	Opt : �ϐ��Ԓn
	ReturnState opSto(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 0)) return Error;
		th.state->workarea[code.option] = th.StackPop();
		return None;
	};
	//	�ϐ��ǂݍ���(�σA�h���X)
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState opVlod(Thread& th, const Code& code) {
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
	ReturnState opVsto(Thread& th, const Code& code) {
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
	ReturnState opSpps(Thread& th, const Code& code) {
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
	ReturnState opDup(Thread& th, const Code& code) {
		if (CheckStack(th, 1, code.option + 1)) return Error;
		int count = code.option;
		if (count <= 0) count = 1;
		for (int i = 0; i < count; i++) {
			th.StackPush(th.StackTop());
		}
		return None;
	};
	//	�X�^�b�N�g�b�v�폜
	//	Stk : Opt / 0
	//	Opt : �폜����v�f��(0�ȉ���1�ɕ␳)
	ReturnState opDel(Thread& th, const Code& code) {
		auto n = code.option < 0 ? 1 : code.option;
		if (CheckStack(th, n, 0)) return Error;
		th.StackPop(n);
		return None;
	};
	//	�X�^�b�N�N���A
	//	Stk : All / 0
	//	Opt : ���g�p
	ReturnState opCls(Thread& th, const Code& code) {
		th.ClearStack();
		return None;
	};
	//	�T�u���[�`���W�����v
	//	Stk : 0 / 0
	//	Opt : �W�����v��A�h���X
	ReturnState opCall(Thread& th, const Code& code) {
		th.callstack.push_back(th.codeindex);
		th.codeindex = code.option - 1;

		return None;
	};
	//	���^�[��
	//	Stk : 0 / 0
	//	Opt : ���g�p
	ReturnState opRet(Thread& th, const Code& code) {
		if (th.callstack.size() == 0) {
			th.errorCode = CallstackUnderflow;
			return Error;
		}
		th.codeindex = th.callstack.back();
		th.callstack.pop_back();

		return None;
	};

	ReturnState opPush(Thread& th, const Code& code) {
		th.StackPush(code.val);

		return None;
	}
	
	// �萔�A�h���X�ϐ��Ԃ̒��ډ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState opNsAdd(Thread& th, const Code& code) {
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
	ReturnState opNsSub(Thread& th, const Code& code) {
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
	ReturnState opNsMul(Thread& th, const Code& code) {
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
	ReturnState opNsDiv(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.state->workarea.size() <= dst || src < 0 || th.state->workarea.size() <= src) {
			th.errorCode = WorkareaOutOfRange;
			return Error;
		}
		th.state->workarea[dst].float_ /= th.state->workarea[src].float_;
		return None;
	}

	ReturnState opPushSb(Thread& th, const Code& code) {
		int n = code.option;
		if (n < 0) n = 0;
		if (th.FramePush(n)) return Error;

		return None;
	}

	ReturnState opPopSb(Thread& th, const Code& code) {
		int n = code.option;
		if (n < 0) n = 0;
		if (th.FramePop(n)) return Error;

		return None;
	}



}
