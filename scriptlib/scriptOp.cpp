#include "script.h"
#include <math.h>

namespace Script {

	float Thread::StackPop() {
		if (workstack.size() == 0) {
			// 例外を投げる
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

	//	未定義の命令語
	//	Stk : 0 / 0
	//	Opt : 未使用
	ReturnState Thread::opNull(const Code& code) {
		errorCode = InvalidOpcode;
		return Error;
	};
	//	中断
	//	Stk : # / #
	//	Opt : 待機カウント(0以上、0の場合は中断のみ行う)
	ReturnState Thread::opWait(const Code& code) {
		waitcount = code.option >= 0 ? code.option : 1;
		return Wait;
	};
	//	終了
	//	Stk : # / #
	//	Opt : 未使用
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

	//	ジャンプ
	//	Stk : 0 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJmp(const Code& code) {
		codeindex += code.option;
		
		return None;
	};
	//	等価条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJeq(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left == right)
			codeindex += code.option;
		
		return None;
	};
	//	不等条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJne(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left != right)
			codeindex += code.option;
		
		return None;
	};
	//	超過条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJgt(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left > right)
			codeindex += code.option;
		
		return None;
	};
	//	以上条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJge(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left >= right)
			codeindex += code.option;
		
		return None;
	};
	//	未満条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJlt(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left < right)
			codeindex += code.option;
		
		return None;
	};
	//	以下条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJle(const Code& code) {
		if (CheckStack(this, 2, 0)) return Error;
		float right = StackPop();
		float left = StackPop();
		if (left <= right)
			codeindex += code.option;
		
		return None;
	};
	//	ゼロ条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJz(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		float num = StackPop();
		if (num == 0.0)
			codeindex += code.option;
		
		return None;
	};
	//	非ゼロ条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJnz(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		float num = StackPop();
		if (num != 0.0)
			codeindex += code.option;
		
		return None;
	};
	//	正数条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJpos(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		float num = StackPop();
		if (num > 0.0)
			codeindex += code.option;

		return None;
	};
	//	負数条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJneg(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		float num = StackPop();
		if (num < 0.0)
			codeindex += code.option;
		
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
	//	数値属性取得
	//	Stk : 1 / 1
	//	Opt : 動作内容	Opt[0]:反転フラグ、Opt[1-3]:動作フラグ
	//			0 : ゼロ
	//			2 : 正の数(0含まず)
	//			4 : 負の数(0含まず)
	//			6 : 正の無限大
	//			8 : 負の無限大
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
	//	早送り
	//	Stk : 0 / 0
	//	Opt : 識別ID 0で一番近いフラグ
	ReturnState Thread::opFwd(const Code& code) {
		while (true) {
			const Code& c = state->provider->Get(codeindex);
			if (c.label && (code.option == 0 || code.option == c.label)) {
				break;
			}
			codeindex++;
		}
		// 実行後に一つ進むため、ここで1引いておく
		codeindex--;
		return None;
	};
	//	巻き戻し
	//	Stk : 0 / 0
	//	Opt : 識別ID 0で一番近いフラグ
	ReturnState Thread::opRew(const Code& code) {
		while (true) {
			const Code& c = state->provider->Get(codeindex);
			if (c.label && (code.option == 0 || code.option == c.label)) {
				break;
			}
			codeindex--;
		}
		// 実行後に一つ進むため、ここで1引いておく
		codeindex--;
		return None;
	};
#if 0
	//	Fwd/Rev用チェックポイント
	//	Stk : 0 / 0
	//	Opt : 未使用(被ジャンプ時にシグネチャとして利用される)
	ReturnState Thread::opCpt(const Code& code) {
		return None;
	};
#endif
	//	加算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,またはメモリアドレス
	ReturnState Thread::opAdd(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() += f;
		return None;
	};
	//	連続加算
	//	Stk : Opt+1 / 1
	//	Opt : 加算回数(1で普通の加算)
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
	//	乗算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,またはメモリアドレス
	ReturnState Thread::opMul(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() *= f;
		return None;
	};
	//	連続乗算
	//	Stk : Opt+1 / 1
	//	Opt : 乗算回数(1で普通の乗算)
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
	//	減算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,またはメモリアドレス
	ReturnState Thread::opSub(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() -= f;
		return None;
	};
	//	正負変換
	//	Stk : 1 / 1
	//	Opt : 未使用
	ReturnState Thread::opNeg(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = -StackTop();
		return None;
	};
	//	除算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,またはメモリアドレス
	ReturnState Thread::opDiv(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() /= f;
		return None;
	};
	//	剰余
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,またはメモリアドレス
	ReturnState Thread::opMod(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() = fmod(StackTop(), f);
		return None;
	};
	//	正弦
	//	Stk : 1 / 1
	//	Opt : 未使用
	ReturnState Thread::opSin(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = sin(StackTop());
		return None;
	};
	//	余弦
	//	Stk : 1 / 1
	//	Opt : 未使用
	ReturnState Thread::opCos(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = cos(StackTop());
		return None;
	};
	//	正接
	//	Stk : 1 / 1
	//	Opt : 未使用
	ReturnState Thread::opTan(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = tan(StackTop());
		return None;
	};
	//	偏角
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,またはメモリアドレス
	ReturnState Thread::opArg(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() = atan2(f, StackTop());
		return None;
	};
	//	平方根
	//	Stk : 1 / 1
	//	Opt : 未使用
	ReturnState Thread::opSqrt(const Code& code) {
		if (CheckStack(this, 1, 1)) return Error;
		StackTop() = sqrt(StackTop());
		return None;
	};
	//	累乗
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 0,またはメモリアドレス
	ReturnState Thread::opPow(const Code& code) {
		if (CheckStack(this, code.option ? 2 : 1, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackTop() = pow(StackTop(), f);
		return None;
	};
	//	対数
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 0,またはメモリアドレス
	ReturnState Thread::opLog(const Code& code) {
		if (CheckStack(this, code.option ? 1 : 0, 1)) return Error;
		float f = (code.option ? state->workarea[code.option] : StackPop());
		StackPush(log(f));
		return None;
	};
	//	sqrt(a^2 + b^2 + ...)を計算する
	//	Stk : Opt / 1
	//	Opt : 次元の数
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
	//	変数読み出し(定数アドレス)
	//	Stk : 0 / 1
	//	Opt : 変数番地
	ReturnState Thread::opLod(const Code& code) {
		if (CheckStack(this, 0, 1)) return Error;
		StackPush(state->workarea[code.option]);
		return None;
	};
	//	変数書き込み(定数アドレス)
	//	Stk : 1 / 0
	//	Opt : 変数番地
	ReturnState Thread::opSto(const Code& code) {
		if (CheckStack(this, 1, 0)) return Error;
		state->workarea[code.option] = StackPop();
		return None;
	};
	//	変数読み込み(可変アドレス)
	//	Stk : 1 / 1
	//	Opt : 未使用
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
	//	変数書き込み(可変アドレス)
	//	Stk : 2 / 0
	//	Opt : 未使用
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
	//	特殊値プッシュ
	//	Stk : 0 / 1
	//	Opt : 積む値の種類
	//			0 : 0
	//			1 : 1
	//			2 : -1
	//			3 : 無限+
	//			4 :  〃 -
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
	//	スタックトップ複製
	//	Stk : 1 / Opt + 1
	//	Opt : 複製する数(0以下は1に補正)
	ReturnState Thread::opDup(const Code& code) {
		if (CheckStack(this, 1, code.option + 1)) return Error;
		StackPush(StackTop());
		return None;
	};
	//	スタックトップ削除
	//	Stk : Opt / 0
	//	Opt : 削除する要素数
	ReturnState Thread::opDel(const Code& code) {
		if (CheckStack(this, code.option, 0)) return Error;
		if (StackSize() < (unsigned int)code.option) {
			errorCode = WorkstackUnderflow;
			return Error;
		}
		StackPop();
		return None;
	};
	//	スタッククリア
	//	Stk : All / 0
	//	Opt : 未使用
	ReturnState Thread::opCls(const Code& code) {
		ClearStack();
		return None;
	};
	//	サブルーチンジャンプ
	//	Stk : 0 / 0
	//	Opt : ジャンプ先アドレス
	ReturnState Thread::opCall(const Code& code) {
		callstack.push_back(codeindex);
		codeindex = code.option;

		return None;
	};
	//	リターン
	//	Stk : 0 / 0
	//	Opt : 未使用
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

	// 定数アドレス変数間の直接加算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 加算先アドレス(0 - 255)
	//        Opt[4-7] 加算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接減算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 減算先アドレス(0 - 255)
	//        Opt[4-7] 減算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接乗算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 乗算先アドレス(0 - 255)
	//        Opt[4-7] 乗算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接除算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 除算先アドレス(0 - 255)
	//        Opt[4-7] 除算する値のアドレス(0-255)
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
