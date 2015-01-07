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

	//	未定義の命令語
	//	Stk : 0 / 0
	//	Opt : 未使用
	ReturnState opNull(Thread& th, const Code& code) {
		th.SetErrorCode(InvalidOpcode);
		return Error;
	};
	//	中断
	//	Stk : # / #
	//	Opt : 待機カウント(0以上、0の場合は中断のみ行う)
	ReturnState opWait(Thread& th, const Code& code) {
		th.WaitThread(code.option > 0 ? code.option : 0);
		return Wait;
	};
	//	終了
	//	Stk : # / #
	//	Opt : 未使用
	ReturnState opEnd(Thread& th, const Code& code) {
		th.SetErrorCode(ScriptHasFinished);
		return Finished;
	};

	//	絶対位置へのジャンプ
	//	Stk : 1 / 0 or 0 / 0
	//	Opt : -1、またはジャンプ位置
	ReturnState opGoto(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 0)) return Error;
		int addr = code.option < 0 ? (int)th.StackPop().float_ : code.option;
		th.SetCodeIndex(addr - 1);

		return None;
	}

	//	ジャンプ
	//	Stk : 0 / 0
	//	Opt : ジャンプオフセット
	ReturnState opJmp(Thread& th, const Code& code) {
		if (code.option >= 0)
			th.AddCodeIndex(code.option);
		
		return None;
	};
	//	等価条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	不等条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	超過条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	以上条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	未満条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	以下条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	ゼロ条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	非ゼロ条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	正数条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	負数条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	比較演算
	//	Stk : 2 / 1
	//	Opt : 動作内容  Opt[0]:反転フラグ, Opt[1-3]:動作フラグ
	//			0 : 等価
	//			2 : 超過
	//			4 : 未満
	//			6 : And(両方とも0以外)
	//			8 : Or(少なくともどちらかが0以外)
	//			A : Xor(どちらか一方のみが0以外)
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
	//	数値属性取得
	//	Stk : 1 / 1
	//	Opt : 動作内容	Opt[0]:反転フラグ、Opt[1-3]:動作フラグ
	//			0 : ゼロ
	//			2 : 正の数(0含まず)
	//			4 : 負の数(0含まず)
	//			6 : 正の無限大
	//			8 : 負の無限大
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
	//	早送り
	//	Stk : 0 / 0
	//	Opt : 識別ID -1で一番近いフラグ
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
		// 実行後に一つ進むが、指されたコードは何も実行しないコードなので、スキップされても支障はない。
		th.SetCodeIndex(codeindex);
		return None;
	};
	//	巻き戻し
	//	Stk : 0 / 0
	//	Opt : 識別ID -1で一番近いフラグ
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
		// 実行後に一つ進むが、指されたコードは何も実行しないコードなので、スキップされても支障はない。
		th.SetCodeIndex(codeindex);
		return None;
	};
#if 0
	//	Fwd/Rev用チェックポイント
	//	Stk : 0 / 0
	//	Opt : 未使用(被ジャンプ時にシグネチャとして利用される)
	ReturnState opCpt(Thread& th, const Code& code) {
		return None;
	};
#endif
	//	加算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opAdd(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ += f;
		return None;
	};
	//	連続加算
	//	Stk : Opt+1 / 1
	//	Opt : 加算回数(1で普通の加算) ※ 要素数ではない
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
	//	乗算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opMul(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ *= f;
		return None;
	};
	//	連続乗算
	//	Stk : Opt+1 / 1
	//	Opt : 乗算回数(1で普通の乗算) ※ 要素数ではない
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
	//	減算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opSub(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option < 0 ? th.GetState()->At(code.option) : th.StackPop());
		th.StackTop().float_ -= f;
		return None;
	};
	//	正負変換
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opNeg(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(-f);
		return None;
	};
	//	除算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opDiv(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ /= f;
		return None;
	};
	//	剰余
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opMod(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = fmod(th.StackTop(), f);
		return None;
	};
	//	正弦
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opSin(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sin(f));
		return None;
	};
	//	余弦
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opCos(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(cos(f));
		return None;
	};

	//	正弦と余弦(正弦、余弦、の順にプッシュされる)
	//	Stk : 1 / 2 or 0 / 2
	//	Opt : 整数-1,または即値
	ReturnState opSinCos(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 2)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sin(f));
		th.StackPush(cos(f));
		return None;
	};

	//	正弦と余弦(余弦、正弦、の順にプッシュされる)
	//	Stk : 1 / 2 or 0 / 2
	//	Opt : 整数-1,または即値
	ReturnState opCosSin(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 2)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(cos(f));
		th.StackPush(sin(f));
		return None;
	};

	//	正接
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opTan(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(tan(f));
		return None;
	};
	//	偏角
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opArg(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = atan2(f, th.StackTop());
		return None;
	};
	//	平方根
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opSqrt(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sqrt(f));
		return None;
	};
	//	累乗
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opPow(Thread& th, const Code& code) {
		if (th.CheckStack(code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = pow(th.StackTop(), f);
		return None;
	};
	//	自然対数
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opLog(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log(f));
		return None;
	};
	//	常用対数
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opLog10(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log10(f));
		return None;
	};

	//	sqrt(a^2 + b^2 + ...)を計算する
	//	Stk : Opt / 1
	//	Opt : 次元の数
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

	//	度数法 -> 弧度法
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opD2r(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(3.1415926535898f * f / 180);
		return None;
	};

	//	弧度法 -> 度数法
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opR2d(Thread& th, const Code& code) {
		if (th.CheckStack(code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(180 * f / 3.1415926535898f);
		return None;
	};


	//	変数読み出し(定数アドレス)
	//	Stk : 0 / 1
	//	Opt : 変数番地
	ReturnState opLod(Thread& th, const Code& code) {
		if (th.CheckStack(0, 1)) return Error;
		th.StackPush(th.GetState()->At(code.option));
		return None;
	};
	//	変数書き込み(定数アドレス)
	//	Stk : 1 / 0
	//	Opt : 変数番地
	ReturnState opSto(Thread& th, const Code& code) {
		if (th.CheckStack(1, 0)) return Error;
		th.GetState()->At(code.option) = th.StackPop();
		return None;
	};
	//	変数読み込み(可変アドレス)
	//	Stk : 1 / 1
	//	Opt : 未使用
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
	//	変数書き込み(可変アドレス)
	//	Stk : 2 / 0
	//	Opt : 未使用
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
	//	特殊値プッシュ
	//	Stk : 0 / 1
	//	Opt : 積む値の種類
	//			 0 : 0
	//			 2 : 1
	//			 4 : -1
	//			 6 : 無限+
	//			 8 :  〃 -
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
			default: // 否定ビット付きの場合もここに流れる
				th.SetErrorCode(InvalidOpcode);
				return Error;
		};
		th.StackPush(val);

		return None;
	};
	//	スタックトップ複製
	//	Stk : 1 / Opt + 1
	//	Opt : 複製する数(0以下は1に補正)
	ReturnState opDup(Thread& th, const Code& code) {
		if (th.CheckStack(1, code.option + 1)) return Error;
		int count = code.option;
		if (count <= 0) count = 1;
		for (int i = 0; i < count; i++) {
			th.StackPush(th.StackTop());
		}
		return None;
	};
	//	スタックトップ削除
	//	Stk : Opt / 0
	//	Opt : 削除する要素数(0以下は1に補正)
	ReturnState opDel(Thread& th, const Code& code) {
		auto n = code.option < 0 ? 1 : code.option;
		if (th.CheckStack(n, 0)) return Error;
		th.StackPop(n);
		return None;
	};
	//	スタッククリア
	//	Stk : All / 0
	//	Opt : 未使用
	ReturnState opCls(Thread& th, const Code& code) {
		th.ClearStack();
		return None;
	};
	//	サブルーチンジャンプ
	//	Stk : 0 / 0
	//	Opt : ジャンプ先アドレス
	ReturnState opCall(Thread& th, const Code& code) {
		if (auto ret = th.GoSub(code.option)) return ret;

		return None;
	};
	//	リターン
	//	Stk : 0 / 0
	//	Opt : 未使用
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

	// 定数アドレス変数間の直接加算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 加算先アドレス(0 - 255)
	//        Opt[4-7] 加算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接減算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 減算先アドレス(0 - 255)
	//        Opt[4-7] 減算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接乗算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 乗算先アドレス(0 - 255)
	//        Opt[4-7] 乗算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接除算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 除算先アドレス(0 - 255)
	//        Opt[4-7] 除算する値のアドレス(0-255)
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
