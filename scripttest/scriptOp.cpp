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

	//	0x40 - 0x4F(空)
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

	//	0x50 - 0x5F(空)
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

	//	0x60 - 0x6F(空)
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

	//	0x70 - 0x7F(空)
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

//	未定義の命令語
//	Stk : 0 / 0
//	Opt : 未使用
ScriptReturnState ScriptState::opNull(short opt){
	errorCode = InvalidOpcode;
	return Error;
};
//	中断
//	Stk : # / #
//	Opt : 待機カウント(0以上、0の場合は中断のみ行う)
ScriptReturnState ScriptState::opYld(short opt){
	waitcount = opt >= 0 ? opt : 1;
	return Yield;
};
//	終了
//	Stk : # / #
//	Opt : 未使用
ScriptReturnState ScriptState::opEnd(short opt){
	opCls(0);
	errorCode = ScriptHasFinished;
	return Finished;
};
//	ジャンプ
//	Stk : 1 / 0
//	Opt : 未使用
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
//	等価条件ジャンプ
//	Stk : 2 / 0
//	Opt : ジャンプオフセット
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
//	不等条件ジャンプ
//	Stk : 2 / 0
//	Opt : ジャンプオフセット
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
//	超過条件ジャンプ
//	Stk : 2 / 0
//	Opt : ジャンプオフセット
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
//	以上条件ジャンプ
//	Stk : 2 / 0
//	Opt : ジャンプオフセット
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
//	未満条件ジャンプ
//	Stk : 2 / 0
//	Opt : ジャンプオフセット
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
//	以下条件ジャンプ
//	Stk : 2 / 0
//	Opt : ジャンプオフセット
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
//	ゼロ条件ジャンプ
//	Stk : 1 / 0
//	Opt : ジャンプオフセット
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
//	非ゼロ条件ジャンプ
//	Stk : 1 / 0
//	Opt : ジャンプオフセット
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
//	正数条件ジャンプ
//	Stk : 1 / 0
//	Opt : ジャンプオフセット
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
//	負数条件ジャンプ
//	Stk : 1 / 0
//	Opt : ジャンプオフセット
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
//	比較演算
//	Stk : 2 / 1
//	Opt : 動作内容  Opt[0]:反転フラグ, Opt[1-3]:動作フラグ
//			0 : 等価
//			2 : 超過
//			4 : 未満
//			6 : And(両方とも0以外)
//			8 : Or(少なくともどちらかが0以外)
//			A : Xor(どちらか一方のみが0以外)
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
//	数値属性取得
//	Stk : 1 / 1
//	Opt : 動作内容	Opt[0]:反転フラグ、Opt[1-3]:動作フラグ
//			0 : ゼロ
//			2 : 正の数(0含まず)
//			4 : 負の数(0含まず)
//			6 : 正の無限大
//			8 : 負の無限大
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
//	早送り
//	Stk : 0 / 0
//	Opt : 識別ID 0で一番近いフラグ
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
//	巻き戻し
//	Stk : 0 / 0
//	Opt : 識別ID 0で一番近いフラグ
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
//	Fwd/Rev用チェックポイント
//	Stk : 0 / 0
//	Opt : 未使用
ScriptReturnState ScriptState::opCpt(short opt){
	return None;
};
//	加算
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,またはメモリアドレス
ScriptReturnState ScriptState::opAdd(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] += f;
	return None;
};
//	連続加算
//	Stk : Opt+1 / 1
//	Opt : 加算回数(1で普通の加算)
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
//	乗算
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,またはメモリアドレス
ScriptReturnState ScriptState::opMul(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] *= f;
	return None;
};
//	連続乗算
//	Stk : Opt+1 / 1
//	Opt : 乗算回数(1で普通の乗算)
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
//	減算
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,またはメモリアドレス
ScriptReturnState ScriptState::opSub(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] -= f;
	return None;
};
//	正負変換
//	Stk : 1 / 1
//	Opt : 未使用
ScriptReturnState ScriptState::opNeg(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = -workstack[workstacktop - 1];
	return None;
};
//	除算
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,またはメモリアドレス
ScriptReturnState ScriptState::opDiv(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] /= f;
	return None;
};
//	剰余
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,またはメモリアドレス
ScriptReturnState ScriptState::opMod(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] = fmod(workstack[workstacktop - 1], f);
	return None;
};
//	正弦
//	Stk : 1 / 1
//	Opt : 未使用
ScriptReturnState ScriptState::opSin(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = sin(workstack[workstacktop - 1]);
	return None;
};
//	余弦
//	Stk : 1 / 1
//	Opt : 未使用
ScriptReturnState ScriptState::opCos(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = cos(workstack[workstacktop - 1]);
	return None;
};
//	正接
//	Stk : 1 / 1
//	Opt : 未使用
ScriptReturnState ScriptState::opTan(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = tan(workstack[workstacktop - 1]);
	return None;
};
//	偏角
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,またはメモリアドレス
ScriptReturnState ScriptState::opArg(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] = atan2(f, workstack[workstacktop - 1]);
	return None;
};
//	平方根
//	Stk : 1 / 1
//	Opt : 未使用
ScriptReturnState ScriptState::opSqrt(short opt){
	if (CheckStack(this, 1, 1)) return Error;
	workstack[workstacktop - 1] = sqrt(workstack[workstacktop - 1]);
	return None;
};
//	累乗
//	Stk : 2 / 1 or 1 / 1
//	Opt : 0,またはメモリアドレス
ScriptReturnState ScriptState::opPow(short opt){
	if (CheckStack(this, opt ? 2 : 1, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop - 1] = pow(workstack[workstacktop - 1], f);
	return None;
};
//	対数
//	Stk : 1 / 1 or 0 / 1
//	Opt : 0,またはメモリアドレス
ScriptReturnState ScriptState::opLog(short opt){
	if (CheckStack(this, opt ? 1 : 0, 1)) return Error;
	float f = (opt ? workarea[opt] : workstack[--workstacktop]);
	workstack[workstacktop++] = log(f);
	return None;
};
//	sqrt(a^2 + b^2 + ...)を計算する
//	Stk : Opt / 1
//	Opt : 次元の数
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
//	変数読み出し(定数アドレス)
//	Stk : 0 / 1
//	Opt : 変数番地
ScriptReturnState ScriptState::opLod(short opt){
	if (CheckStack(this, 0, 1)) return Error;
	workstack[workstacktop++] = workarea[opt];
	return None;
};
//	変数書き込み(定数アドレス)
//	Stk : 1 / 0
//	Opt : 変数番地
ScriptReturnState ScriptState::opSto(short opt){
	if (CheckStack(this, 1, 0)) return Error;
	workarea[opt] = workstack[--workstacktop];
	return None;
};
//	変数読み込み(可変アドレス)
//	Stk : 1 / 1
//	Opt : 未使用
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
//	変数書き込み(可変アドレス)
//	Stk : 2 / 0
//	Opt : 未使用
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
//	特殊値プッシュ
//	Stk : 0 / 1
//	Opt : 積む値の種類
//			0 : NaN
//			1 : 無限+
//			2 :  〃 -
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
//	スタックトップ複製
//	Stk : 1 / Opt + 1
//	Opt : 複製する数(0以下は1に補正)
ScriptReturnState ScriptState::opDup(short opt){
	if (CheckStack(this, 1, opt + 1)) return Error;
	workstack[workstacktop] = workstack[workstacktop - 1];
	workstacktop++;
	return None;
};
//	スタックトップ削除
//	Stk : Opt / 0
//	Opt : 削除する要素数
ScriptReturnState ScriptState::opDel(short opt){
	if (CheckStack(this, opt, 0)) return Error;
	if (workstacktop < opt){
		errorCode = WorkstackUnderflow;
		return Error;
	}
	workstacktop -= opt;
	return None;
};
//	スタッククリア
//	Stk : All / 0
//	Opt : 未使用
ScriptReturnState ScriptState::opCls(short opt){
	workstacktop = 0;
	return None;
};
//	サブルーチンジャンプ
//	Stk : 0 / 0
//	Opt : ジャンプ先アドレス
ScriptReturnState ScriptState::opCall(short opt){
	callstack[callstacktop++] = codeindex;
	if (callstacktop == callstacksize){
		errorCode = CallstackOverflow;
		return Error;
	}
	codeindex = opt;

	return None;
};
//	リターン
//	Stk : 0 / 0
//	Opt : 未使用
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
