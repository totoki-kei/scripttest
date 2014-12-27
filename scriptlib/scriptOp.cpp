#include "script.h"
#include <math.h>

namespace Script {

	Value Thread::StackPop(int n) {
		if (workstack.size() == 0) {
			// 例外を投げる
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

	//	未定義の命令語
	//	Stk : 0 / 0
	//	Opt : 未使用
	ReturnState Thread::opNull(Thread& th, const Code& code) {
		th.errorCode = InvalidOpcode;
		return Error;
	};
	//	中断
	//	Stk : # / #
	//	Opt : 待機カウント(0以上、0の場合は中断のみ行う)
	ReturnState Thread::opWait(Thread& th, const Code& code) {
		th.waitcount = code.option > 0 ? code.option : 0;
		return Wait;
	};
	//	終了
	//	Stk : # / #
	//	Opt : 未使用
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

	//	ジャンプ
	//	Stk : 0 / 0
	//	Opt : ジャンプオフセット
	ReturnState Thread::opJmp(Thread& th, const Code& code) {
		if (code.option >= 0)
			th.codeindex += code.option;
		
		return None;
	};
	//	等価条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	不等条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	超過条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	以上条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	未満条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	以下条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	ゼロ条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	非ゼロ条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	正数条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	負数条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	比較演算
	//	Stk : 2 / 1
	//	Opt : 動作内容  Opt[0]:反転フラグ, Opt[1-3]:動作フラグ
	//			0 : 等価
	//			2 : 超過
	//			4 : 未満
	//			6 : And(両方とも0以外)
	//			8 : Or(少なくともどちらかが0以外)
	//			A : Xor(どちらか一方のみが0以外)
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
	//	数値属性取得
	//	Stk : 1 / 1
	//	Opt : 動作内容	Opt[0]:反転フラグ、Opt[1-3]:動作フラグ
	//			0 : ゼロ
	//			2 : 正の数(0含まず)
	//			4 : 負の数(0含まず)
	//			6 : 正の無限大
	//			8 : 負の無限大
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
	//	早送り
	//	Stk : 0 / 0
	//	Opt : 識別ID 0で一番近いフラグ
	ReturnState Thread::opFwd(Thread& th, const Code& code) {
		while (0 <= th.codeindex && th.codeindex < th.state->provider->Length()) {
			const Code& c = th.state->provider->Get(th.codeindex);
			if (c.label && (code.option == 0 || code.option == c.label)) {
				break;
			}
			th.codeindex++;
		}
		// 実行後に一つ進むため、ここで1引いておく
		th.codeindex--;
		return None;
	};
	//	巻き戻し
	//	Stk : 0 / 0
	//	Opt : 識別ID 0で一番近いフラグ
	ReturnState Thread::opRew(Thread& th, const Code& code) {
		while (0 <= th.codeindex && th.codeindex < th.state->provider->Length()) {
			const Code& c = th.state->provider->Get(th.codeindex);
			if (c.label && (code.option <= 0 || code.option == c.label)) {
				break;
			}
			th.codeindex--;
		}
		// 実行後に一つ進むため、ここで1引いておく
		th.codeindex--;
		return None;
	};
#if 0
	//	Fwd/Rev用チェックポイント
	//	Stk : 0 / 0
	//	Opt : 未使用(被ジャンプ時にシグネチャとして利用される)
	ReturnState Thread::opCpt(Thread& th, const Code& code) {
		return None;
	};
#endif
	//	加算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opAdd(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ += f;
		return None;
	};
	//	連続加算
	//	Stk : Opt+1 / 1
	//	Opt : 加算回数(1で普通の加算)
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
	//	乗算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opMul(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ *= f;
		return None;
	};
	//	連続乗算
	//	Stk : Opt+1 / 1
	//	Opt : 乗算回数(1で普通の乗算)
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
	//	減算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opSub(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option < 0 ? th.state->workarea[code.option] : th.StackPop());
		th.StackTop().float_ -= f;
		return None;
	};
	//	正負変換
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opNeg(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(-f);
		return None;
	};
	//	除算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opDiv(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ /= f;
		return None;
	};
	//	剰余
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opMod(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = fmod(th.StackTop(), f);
		return None;
	};
	//	正弦
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opSin(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sin(f));
		return None;
	};
	//	余弦
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opCos(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(cos(f));
		return None;
	};
	//	正接
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opTan(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(tan(f));
		return None;
	};
	//	偏角
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opArg(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = atan2(f, th.StackTop());
		return None;
	};
	//	平方根
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opSqrt(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sqrt(f));
		return None;
	};
	//	累乗
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opPow(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = pow(th.StackTop(), f);
		return None;
	};
	//	自然対数
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opLog(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log(f));
		return None;
	};
	//	常用対数
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState Thread::opLog10(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log10(f));
		return None;
	};

	//	sqrt(a^2 + b^2 + ...)を計算する
	//	Stk : Opt / 1
	//	Opt : 次元の数
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
	//	変数読み出し(定数アドレス)
	//	Stk : 0 / 1
	//	Opt : 変数番地
	ReturnState Thread::opLod(Thread& th, const Code& code) {
		if (CheckStack(th, 0, 1)) return Error;
		th.StackPush(th.state->workarea[code.option]);
		return None;
	};
	//	変数書き込み(定数アドレス)
	//	Stk : 1 / 0
	//	Opt : 変数番地
	ReturnState Thread::opSto(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 0)) return Error;
		th.state->workarea[code.option] = th.StackPop();
		return None;
	};
	//	変数読み込み(可変アドレス)
	//	Stk : 1 / 1
	//	Opt : 未使用
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
	//	変数書き込み(可変アドレス)
	//	Stk : 2 / 0
	//	Opt : 未使用
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
	//	特殊値プッシュ
	//	Stk : 0 / 1
	//	Opt : 積む値の種類
	//			0 : 0
	//			1 : 1
	//			2 : -1
	//			3 : 無限+
	//			4 :  〃 -
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
	//	スタックトップ複製
	//	Stk : 1 / Opt + 1
	//	Opt : 複製する数(0以下は1に補正)
	ReturnState Thread::opDup(Thread& th, const Code& code) {
		if (CheckStack(th, 1, code.option + 1)) return Error;
		th.StackPush(th.StackTop());
		return None;
	};
	//	スタックトップ削除
	//	Stk : Opt / 0
	//	Opt : 削除する要素数
	ReturnState Thread::opDel(Thread& th, const Code& code) {
		auto n = code.option < 0 ? 1 : code.option;
		if (CheckStack(th, n, 0)) return Error;
		th.StackPop(n);
		return None;
	};
	//	スタッククリア
	//	Stk : All / 0
	//	Opt : 未使用
	ReturnState Thread::opCls(Thread& th, const Code& code) {
		th.ClearStack();
		return None;
	};
	//	サブルーチンジャンプ
	//	Stk : 0 / 0
	//	Opt : ジャンプ先アドレス
	ReturnState Thread::opCall(Thread& th, const Code& code) {
		th.callstack.push_back(th.codeindex);
		th.codeindex = code.option - 1;

		return None;
	};
	//	リターン
	//	Stk : 0 / 0
	//	Opt : 未使用
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
	
	// 定数アドレス変数間の直接加算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 加算先アドレス(0 - 255)
	//        Opt[4-7] 加算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接減算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 減算先アドレス(0 - 255)
	//        Opt[4-7] 減算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接乗算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 乗算先アドレス(0 - 255)
	//        Opt[4-7] 乗算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接除算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 除算先アドレス(0 - 255)
	//        Opt[4-7] 除算する値のアドレス(0-255)
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
