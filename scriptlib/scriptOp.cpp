#include "script.h"
#include "scriptOp.h"

#include <math.h>

namespace Script {

	Value Thread::StackPop(int n) {
		if (workstack.size() <= stackBase ) {
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
		if (workstack.size() <= stackBase) {
			// 例外を投げる
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



	//	未定義の命令語
	//	Stk : 0 / 0
	//	Opt : 未使用
	ReturnState opNull(Thread& th, const Code& code) {
		th.errorCode = InvalidOpcode;
		return Error;
	};
	//	中断
	//	Stk : # / #
	//	Opt : 待機カウント(0以上、0の場合は中断のみ行う)
	ReturnState opWait(Thread& th, const Code& code) {
		th.waitcount = code.option > 0 ? code.option : 0;
		return Wait;
	};
	//	終了
	//	Stk : # / #
	//	Opt : 未使用
	ReturnState opEnd(Thread& th, const Code& code) {
		th.errorCode = ScriptHasFinished;
		return Finished;
	};

	//	絶対位置へのジャンプ
	//	Stk : 1 / 0 or 0 / 0
	//	Opt : -1、またはジャンプ位置
	ReturnState opGoto(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 0)) return Error;
		int addr = code.option < 0 ? (int)th.StackPop().float_ : code.option;
		th.codeindex = addr - 1;

		return None;
	}

	//	ジャンプ
	//	Stk : 0 / 0
	//	Opt : ジャンプオフセット
	ReturnState opJmp(Thread& th, const Code& code) {
		if (code.option >= 0)
			th.codeindex += code.option;
		
		return None;
	};
	//	等価条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	不等条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	超過条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	以上条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	未満条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	以下条件ジャンプ
	//	Stk : 2 / 0
	//	Opt : ジャンプオフセット
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
	//	ゼロ条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	非ゼロ条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	正数条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	負数条件ジャンプ
	//	Stk : 1 / 0
	//	Opt : ジャンプオフセット
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
	//	早送り
	//	Stk : 0 / 0
	//	Opt : 識別ID -1で一番近いフラグ
	ReturnState opFwd(Thread& th, const Code& code) {
		while (0 <= th.codeindex && th.codeindex < th.state->provider->Length()) {
			const Code& c = th.state->provider->Get(th.codeindex);
			if (!c.opcode && (code.option < 0 || code.option == c.option)) {
				break;
			}
			th.codeindex++;
		}
		// 実行後に一つ進むが、指されたコードは何も実行しないコードなので、スキップされても支障はない。
		return None;
	};
	//	巻き戻し
	//	Stk : 0 / 0
	//	Opt : 識別ID -1で一番近いフラグ
	ReturnState opRew(Thread& th, const Code& code) {
		while (0 <= th.codeindex && th.codeindex < th.state->provider->Length()) {
			const Code& c = th.state->provider->Get(th.codeindex);
			if (!c.opcode && (code.option < 0 || code.option == c.option)) {
				break;
			}
			th.codeindex--;
		}
		// 実行後に一つ進むが、指されたコードは何も実行しないコードなので、スキップされても支障はない。
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
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ += f;
		return None;
	};
	//	連続加算
	//	Stk : Opt+1 / 1
	//	Opt : 加算回数(1で普通の加算) ※ 要素数ではない
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
	//	乗算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opMul(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ *= f;
		return None;
	};
	//	連続乗算
	//	Stk : Opt+1 / 1
	//	Opt : 乗算回数(1で普通の乗算) ※ 要素数ではない
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
	//	減算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opSub(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option < 0 ? th.state->workarea[code.option] : th.StackPop());
		th.StackTop().float_ -= f;
		return None;
	};
	//	正負変換
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opNeg(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(-f);
		return None;
	};
	//	除算
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opDiv(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop().float_ /= f;
		return None;
	};
	//	剰余
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opMod(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = fmod(th.StackTop(), f);
		return None;
	};
	//	正弦
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opSin(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sin(f));
		return None;
	};
	//	余弦
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opCos(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(cos(f));
		return None;
	};
	//	正接
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opTan(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(tan(f));
		return None;
	};
	//	偏角
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opArg(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = atan2(f, th.StackTop());
		return None;
	};
	//	平方根
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opSqrt(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(sqrt(f));
		return None;
	};
	//	累乗
	//	Stk : 2 / 1 or 1 / 1
	//	Opt : 整数-1,または即値
	ReturnState opPow(Thread& th, const Code& code) {
		if (CheckStack(th, code.option == -1 ? 2 : 1, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackTop() = pow(th.StackTop(), f);
		return None;
	};
	//	自然対数
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opLog(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log(f));
		return None;
	};
	//	常用対数
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opLog10(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(log10(f));
		return None;
	};

	//	sqrt(a^2 + b^2 + ...)を計算する
	//	Stk : Opt / 1
	//	Opt : 次元の数
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

	//	度数法 -> 弧度法
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opD2r(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(3.1415926535898f * f / 180);
		return None;
	};

	//	弧度法 -> 度数法
	//	Stk : 1 / 1 or 0 / 1
	//	Opt : 整数-1,または即値
	ReturnState opR2d(Thread& th, const Code& code) {
		if (CheckStack(th, code.option < 0 ? 1 : 0, 1)) return Error;
		float f = (code.option == -1 ? th.StackPop() : code.val);
		th.StackPush(180 * f / 3.1415926535898f);
		return None;
	};


	//	変数読み出し(定数アドレス)
	//	Stk : 0 / 1
	//	Opt : 変数番地
	ReturnState opLod(Thread& th, const Code& code) {
		if (CheckStack(th, 0, 1)) return Error;
		th.StackPush(th.state->workarea[code.option]);
		return None;
	};
	//	変数書き込み(定数アドレス)
	//	Stk : 1 / 0
	//	Opt : 変数番地
	ReturnState opSto(Thread& th, const Code& code) {
		if (CheckStack(th, 1, 0)) return Error;
		th.state->workarea[code.option] = th.StackPop();
		return None;
	};
	//	変数読み込み(可変アドレス)
	//	Stk : 1 / 1
	//	Opt : 未使用
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
	//	変数書き込み(可変アドレス)
	//	Stk : 2 / 0
	//	Opt : 未使用
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
	//	特殊値プッシュ
	//	Stk : 0 / 1
	//	Opt : 積む値の種類
	//			0 : 0
	//			1 : 1
	//			2 : -1
	//			3 : 無限+
	//			4 :  〃 -
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
	//	スタックトップ複製
	//	Stk : 1 / Opt + 1
	//	Opt : 複製する数(0以下は1に補正)
	ReturnState opDup(Thread& th, const Code& code) {
		if (CheckStack(th, 1, code.option + 1)) return Error;
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
		if (CheckStack(th, n, 0)) return Error;
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
		th.callstack.push_back(th.codeindex);
		th.codeindex = code.option - 1;

		return None;
	};
	//	リターン
	//	Stk : 0 / 0
	//	Opt : 未使用
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
	
	// 定数アドレス変数間の直接加算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 加算先アドレス(0 - 255)
	//        Opt[4-7] 加算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接減算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 減算先アドレス(0 - 255)
	//        Opt[4-7] 減算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接乗算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 乗算先アドレス(0 - 255)
	//        Opt[4-7] 乗算する値のアドレス(0-255)
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

	// 定数アドレス変数間の直接除算
	//	Stk : 0 / 0
	//	Opt : Opt[0-3] 除算先アドレス(0 - 255)
	//        Opt[4-7] 除算する値のアドレス(0-255)
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
