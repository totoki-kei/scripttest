#include "script.h"
#include "scriptOp.h"

#include <math.h>

namespace Script {
	namespace Loader {
		void BuildOpcodes(std::unordered_map<std::string, CodeSkelton>& map) {
#define OPMAPI(name, op) map[ #name ] = { op , AttrType::Integer }
#define OPMAPF(name, op) map[ #name ] = { op , AttrType::Float }
#define OPMAP(name, op, attr) map[ #name ] = { op , attr }

			OPMAPI(nop, opNull);

			OPMAPI(wait, opWait);
			OPMAPI(end, opEnd);

			OPMAP(goto, opGoto, AttrType::EntryPointSymbol);

			OPMAPI(jump, opJmp);
			OPMAPI(jump_eq, opJeq);
			OPMAPI(jump_neq, opJne);
			OPMAPI(jump_gt, opJgt);
			OPMAPI(jump_geq, opJge);
			OPMAPI(jump_lt, opJlt);
			OPMAPI(jump_leq, opJle);
			OPMAPI(jump_zero, opJz);
			OPMAPI(jump_nonzero, opJnz);
			OPMAPI(jump_pos, opJpos);
			OPMAPI(jump_neg, opJneg);

			OPMAP(cmp, opCmp, AttrType::Comparer);
			OPMAP(chk, opIs, AttrType::SpecialNumbers);

			OPMAPI(fwd, opFwd);
			OPMAPI(rew, opRew);

			OPMAPF(add, opAdd);
			OPMAPI(adds, opAdds);
			OPMAPF(mul, opMul);
			OPMAPI(muls, opMuls);
			OPMAPF(sub, opSub);
			OPMAPF(neg, opNeg);
			OPMAPF(div, opDiv);
			OPMAPF(mod, opMod);
			OPMAPF(sin, opSin);
			OPMAPF(cos, opCos);
			OPMAPF(sincos, opSinCos);
			OPMAPF(cossin, opCosSin);
			OPMAPF(tan, opTan);
			OPMAPF(atan, opArg);
			OPMAPF(sqrt, opSqrt);
			OPMAPF(pow, opPow);
			OPMAPF(log, opLog);
			OPMAPF(ln, opLog10);
			OPMAPI(len, opLen);
			OPMAPF(deg2rad, opD2r);
			OPMAPF(rad2deg, opR2d);

			OPMAPI(get, opLod);
			OPMAPI(set, opSto);
			OPMAPI(vget, opVlod);
			OPMAPI(vset, opVsto);
			OPMAP(n, opSpps, AttrType::SpecialNumbers);

			OPMAPI(dup, opDup);
			OPMAPI(pop, opDel);
			OPMAPI(clear, opCls);

			OPMAP(call, opCall, AttrType::EntryPointSymbol);
			OPMAPI(ret, opRet);

			OPMAPI(enter, opPushSb);
			OPMAPI(leave, opPopSb);

			OPMAPF(push, opPush);
			OPMAPI(stklen, opStkLen);

			OPMAPI(dadd, opNsAdd);
			OPMAPI(dsub, opNsSub);
			OPMAPI(dmul, opNsMul);
			OPMAPI(ddiv, opNsDiv);

#undef OPMAP
#undef OPMAPF
#undef OPMAPI
		}
	}

	//	����`�̖��ߌ�
	//	Stk : 0 / 0
	//	Opt : ���g�p
	ReturnState opNull(Thread& th, const Code& code) {
		th.SetErrorCode(InvalidOpcode);
		return Error;
	};
	//	���f
	//	Stk : # / #
	//	Opt : �ҋ@�J�E���g(0�ȏ�A0�̏ꍇ�͒��f�̂ݍs��)
	ReturnState opWait(Thread& th, const Code& code) {
		th.WaitThread(code.option > 0 ? code.option : 0);
		return Wait;
	};
	//	�I��
	//	Stk : # / #
	//	Opt : ���g�p
	ReturnState opEnd(Thread& th, const Code& code) {
		th.SetErrorCode(ScriptHasFinished);
		return Finished;
	};

	//	��Έʒu�ւ̃W�����v
	//	Stk : 1 / 0 or 0 / 0
	//	Opt : -1�A�܂��̓W�����v�ʒu
	ReturnState opGoto(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 0)) return Error;
		int addr = code.option < 0 ? (int)th.StackPop().float_ : code.option;
		th.SetCodeIndex(addr - 1);

		return None;
	}

	//	�W�����v
	//	Stk : 0 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJmp(Thread& th, const Code& code) {
		if (code.option >= 0)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	���������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJeq(Thread& th, const Code& code) {
		if (th.CheckStack(2, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left == right)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	�s�������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJne(Thread& th, const Code& code) {
		if (th.CheckStack(2, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left != right)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	���ߏ����W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJgt(Thread& th, const Code& code) {
		if (th.CheckStack(2, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left > right)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	�ȏ�����W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJge(Thread& th, const Code& code) {
		if (th.CheckStack(2, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left >= right)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	���������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJlt(Thread& th, const Code& code) {
		if (th.CheckStack(2, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left < right)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	�ȉ������W�����v
	//	Stk : 2 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJle(Thread& th, const Code& code) {
		if (th.CheckStack(2, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float right = th.StackPop();
		float left = th.StackPop();
		if (left <= right)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	�[�������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJz(Thread& th, const Code& code) {
		if (th.CheckStack(1, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float num = th.StackPop();
		if (num == 0.0)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	��[�������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJnz(Thread& th, const Code& code) {
		if (th.CheckStack(1, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float num = th.StackPop();
		if (num != 0.0)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	���������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJpos(Thread& th, const Code& code) {
		if (th.CheckStack(1, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float num = th.StackPop();
		if (num > 0.0)
			th.AddCodeIndex(code.option);

		return None;
	};
	//	���������W�����v
	//	Stk : 1 / 0
	//	Opt : �W�����v�I�t�Z�b�g
	ReturnState opJneg(Thread& th, const Code& code) {
		if (th.CheckStack(1, 0)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		float num = th.StackPop();
		if (num < 0.0)
			th.AddCodeIndex(code.option);
		
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
		if (th.CheckStack(2, 1)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
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
		th.SetErrorCode(InvalidOperand);
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
		if (th.CheckStack(1, 1)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOpcode);
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
		th.SetErrorCode(InvalidOperand);
		return Error;
	};
	//	������
	//	Stk : 0 / 0
	//	Opt : ����ID -1�ň�ԋ߂��t���O
	ReturnState opFwd(Thread& th, const Code& code) {
		int codeindex = th.GetCodeIndex();
		auto provider = th.GetCodeProvider();
		while (0 <= codeindex && codeindex < provider->Length()) {
			const Code& c = provider->Get(codeindex);
			if (!c.opcode && (code.option < 0 || code.option == c.option)) {
				break;
			}
			codeindex++;
		}
		// ���s��Ɉ�i�ނ��A�w���ꂽ�R�[�h�͉������s���Ȃ��R�[�h�Ȃ̂ŁA�X�L�b�v����Ă��x��͂Ȃ��B
		th.SetCodeIndex(codeindex);
		return None;
	};
	//	�����߂�
	//	Stk : 0 / 0
	//	Opt : ����ID -1�ň�ԋ߂��t���O
	ReturnState opRew(Thread& th, const Code& code) {
		int codeindex = th.GetCodeIndex();
		auto provider = th.GetCodeProvider();
		while (0 <= codeindex && codeindex < provider->Length()) {
			const Code& c = provider->Get(codeindex);
			if (!c.opcode && (code.option < 0 || code.option == c.option)) {
				break;
			}
			codeindex--;
		}
		// ���s��Ɉ�i�ނ��A�w���ꂽ�R�[�h�͉������s���Ȃ��R�[�h�Ȃ̂ŁA�X�L�b�v����Ă��x��͂Ȃ��B
		th.SetCodeIndex(codeindex);
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
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ += f;
		return None;
	};
	//	�A�����Z
	//	Stk : Opt+1 / 1
	//	Opt : ���Z��(1�ŕ��ʂ̉��Z) �� �v�f���ł͂Ȃ�
	ReturnState opAdds(Thread& th, const Code& code) {
		if (code.option <= 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		if (th.CheckStack(code.option + 1, 1)) return Error;
		float f = 0;
		int count = code.option;
		while (count--) {
			f += th.StackPop().float_;
			if (th.StackSize() < 0) {
				th.SetErrorCode(WorkstackUnderflow);
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
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ *= f;
		return None;
	};
	//	�A����Z
	//	Stk : Opt+1 / 1
	//	Opt : ��Z��(1�ŕ��ʂ̏�Z) �� �v�f���ł͂Ȃ�
	ReturnState opMuls(Thread& th, const Code& code) {
		if (code.option <= 0) {
			th.SetErrorCode(InvalidOpcode);
			return Error;
		}
		if (th.CheckStack(code.option + 1, 1)) return Error;
		float f = 1;
		int count = code.option;
		while (count--) {
			f *= th.StackPop().float_;
			if (th.StackSize() < 0) {
				th.SetErrorCode(WorkstackUnderflow);
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
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option < 0 ? th.GetState()->At(code.option) : th.StackPop());
		th.StackTop().float_ -= f;
		return None;
	};
	//	�����ϊ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opNeg(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(-f);
		return None;
	};
	//	���Z
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opDiv(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ /= f;
		return None;
	};
	//	��]
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opMod(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = fmod(th.StackTop(), f);
		return None;
	};
	//	����
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opSin(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sin(f));
		return None;
	};
	//	�]��
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opCos(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(cos(f));
		return None;
	};

	//	�����Ɨ]��(�����A�]���A�̏��Ƀv�b�V�������)
	//	Stk : 1 / 2 or 0 / 2
	//	Opt : ����-1,�܂��͑��l
	ReturnState opSinCos(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 2)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sin(f));
		th.StackPush(cos(f));
		return None;
	};

	//	�����Ɨ]��(�]���A�����A�̏��Ƀv�b�V�������)
	//	Stk : 1 / 2 or 0 / 2
	//	Opt : ����-1,�܂��͑��l
	ReturnState opCosSin(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 2)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(cos(f));
		th.StackPush(sin(f));
		return None;
	};

	//	����
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opTan(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(tan(f));
		return None;
	};
	//	�Ίp
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opArg(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = atan2(f, th.StackTop());
		return None;
	};
	//	������
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opSqrt(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sqrt(f));
		return None;
	};
	//	�ݏ�
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opPow(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = pow(th.StackTop(), f);
		return None;
	};
	//	���R�ΐ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opLog(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log(f));
		return None;
	};
	//	��p�ΐ�
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opLog10(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log10(f));
		return None;
	};

	//	sqrt(a^2 + b^2 + ...)���v�Z����
	//	Stk : Opt / 1
	//	Opt : �����̐�
	ReturnState opLen(Thread& th, const Code& code) {
		if (th.CheckStack(code.option, 1)) return Error;
		if (code.option < 0) {
			th.SetErrorCode(InvalidOperand);
			return Error;
		}
		double d = 0;
		float f;
		int count = code.option;
		while (count--) {
			f = th.StackPop();
			d += f * f;
			if (th.StackSize() < 0) {
				th.SetErrorCode(WorkstackUnderflow);
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
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(3.1415926535898f * f / 180);
		return None;
	};

	//	�ʓx�@ -> �x���@
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : ����-1,�܂��͑��l
	ReturnState opR2d(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(180 * f / 3.1415926535898f);
		return None;
	};


	//	�ϐ��ǂݏo��(�萔�A�h���X)
	//	Stk : 0 / 1
	//	Opt : �ϐ��Ԓn
	ReturnState opLod(Thread& th, const Code& code) {
		if (th.CheckStack(0, 1)) return Error;
		th.StackPush(th.GetState()->At(code.option));
		return None;
	};
	//	�ϐ���������(�萔�A�h���X)
	//	Stk : 1 / 0
	//	Opt : �ϐ��Ԓn
	ReturnState opSto(Thread& th, const Code& code) {
		if (th.CheckStack(1, 0)) return Error;
		th.GetState()->At(code.option) = th.StackPop();
		return None;
	};
	//	�ϐ��ǂݍ���(�σA�h���X)
	//	Stk : 1 / 1
	//	Opt : ���g�p
	ReturnState opVlod(Thread& th, const Code& code) {
		if (th.CheckStack(1, 1)) return Error;
		auto index = (unsigned int)th.StackTop().float_;
		if (index < 0 || th.GetState()->Count() <= index) {
			th.SetErrorCode(InvalidOperand);
			return Error;
		}

		th.StackTop() = th.GetState()->At(index);

		return None;
	};
	//	�ϐ���������(�σA�h���X)
	//	Stk : 2 / 0
	//	Opt : ���g�p
	ReturnState opVsto(Thread& th, const Code& code) {
		if (th.CheckStack(2, 0)) return Error;
		auto index = (unsigned int)th.StackPop().float_;
		if (index < 0 || th.GetState()->Count() <= index) {
			th.SetErrorCode(InvalidOperand);
			return Error;
		}

		th.GetState()->At(index) = th.StackPop();

		return None;
	};
	//	����l�v�b�V��
	//	Stk : 0 / 1
	//	Opt : �ςޒl�̎��
	//			 0 : 0
	//			 2 : 1
	//			 4 : -1
	//			 6 : ����+
	//			 8 :  �V -
	//			10 : NaN
	ReturnState opSpps(Thread& th, const Code& code) {
		if (th.CheckStack(0, 1)) return Error;
		float val;

		switch (code.option) {
			case 0:
				val = 0.0f;
				break;
			case 2:
				val = 1.0f;
				break;
			case 4:
				val = -1.0f;
				break;
			case 6:
				*((uint32_t*)&val) = 0x7FBFFFFF;
				break;
			case 8:
				val = 1e18f;
				break;
			case 10:
				val = -1e18f;
				break;
			default: // �ے�r�b�g�t���̏ꍇ�������ɗ����
				th.SetErrorCode(InvalidOpcode);
				return Error;
		};
		th.StackPush(val);

		return None;
	};
	//	�X�^�b�N�g�b�v����
	//	Stk : 1 / Opt + 1
	//	Opt : �������鐔(0�ȉ���1�ɕ␳)
	ReturnState opDup(Thread& th, const Code& code) {
		if (th.CheckStack(1, code.option + 1)) return Error;
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
		if (th.CheckStack(n, 0)) return Error;
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
		if (auto ret = th.GoSub(code.option)) return ret;

		return None;
	};
	//	���^�[��
	//	Stk : 0 / 0
	//	Opt : ���g�p
	ReturnState opRet(Thread& th, const Code& code) {
		if (auto ret = th.ReturnSub()) return ret;

		return None;
	};

	ReturnState opPush(Thread& th, const Code& code) {
		th.StackPush(code.val);

		return None;
	}

	ReturnState opStkLen(Thread& th, const Code& code) {
		th.StackPush((float)th.StackSize());

		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ډ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState opNsAdd(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.GetState()->Count() <= dst || src < 0 || th.GetState()->Count() <= src) {
			th.SetErrorCode(WorkareaOutOfRange);
			return Error;
		}
		th.GetState()->At(dst).float_ += th.GetState()->At(src).float_;
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڌ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState opNsSub(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.GetState()->Count() <= dst || src < 0 || th.GetState()->Count() <= src) {
			th.SetErrorCode(WorkareaOutOfRange);
			return Error;
		}
		th.GetState()->At(dst).float_ -= th.GetState()->At(src).float_;
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڏ�Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ��Z��A�h���X(0 - 255)
	//        Opt[4-7] ��Z����l�̃A�h���X(0-255)
	ReturnState opNsMul(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.GetState()->Count() <= dst || src < 0 || th.GetState()->Count() <= src) {
			th.SetErrorCode(WorkareaOutOfRange);
			return Error;
		}
		th.GetState()->At(dst).float_ *= th.GetState()->At(src).float_;
		return None;
	}

	// �萔�A�h���X�ϐ��Ԃ̒��ڏ��Z
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] ���Z��A�h���X(0 - 255)
	//        Opt[4-7] ���Z����l�̃A�h���X(0-255)
	ReturnState opNsDiv(Thread& th, const Code& code) {
		unsigned char dst = code.option & 0xFF;
		unsigned char src = (code.option >> 4) & 0xFF;
		if (dst < 0 || th.GetState()->Count() <= dst || src < 0 || th.GetState()->Count() <= src) {
			th.SetErrorCode(WorkareaOutOfRange);
			return Error;
		}
		th.GetState()->At(dst).float_ /= th.GetState()->At(src).float_;
		return None;
	}

	ReturnState opPushSb(Thread& th, const Code& code) {
		int n = code.option;
		if (n < 0) n = 0;
		if (auto ret = th.FramePush(n)) return ret;

		return None;
	}

	ReturnState opPopSb(Thread& th, const Code& code) {
		int n = code.option;
		if (n < 0) n = 0;
		if (auto ret = th.FramePop(n)) return ret;

		return None;
	}



}
