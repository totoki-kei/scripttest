#pragma once

#include "script.h"
namespace Script {
#pragma region Operation Definitions
	ReturnState opEnd(Thread&, const Code& code);
	ReturnState opWait(Thread&, const Code& code);

	ReturnState opGoto(Thread&, const Code& code);
	ReturnState opJmp(Thread&, const Code& code);
	//ReturnState opCpt(Thread&, const Code& code);
	ReturnState opFwd(Thread&, const Code& code);
	ReturnState opRew(Thread&, const Code& code);
	ReturnState opJz(Thread&, const Code& code);
	ReturnState opJnz(Thread&, const Code& code);
	ReturnState opJpos(Thread&, const Code& code);
	ReturnState opJneg(Thread&, const Code& code);
	ReturnState opJeq(Thread&, const Code& code);
	ReturnState opJne(Thread&, const Code& code);
	ReturnState opJgt(Thread&, const Code& code);
	ReturnState opJge(Thread&, const Code& code);
	ReturnState opJlt(Thread&, const Code& code);
	ReturnState opJle(Thread&, const Code& code);
	ReturnState opCmp(Thread&, const Code& code);
	ReturnState opIs(Thread&, const Code& code);

	ReturnState opAdd(Thread&, const Code& code);
	ReturnState opAdds(Thread&, const Code& code);
	ReturnState opMul(Thread&, const Code& code);
	ReturnState opMuls(Thread&, const Code& code);
	ReturnState opSub(Thread&, const Code& code);
	ReturnState opNeg(Thread&, const Code& code);
	ReturnState opDiv(Thread&, const Code& code);
	ReturnState opMod(Thread&, const Code& code);
	ReturnState opSin(Thread&, const Code& code);
	ReturnState opCos(Thread&, const Code& code);
	ReturnState opTan(Thread&, const Code& code);
	ReturnState opArg(Thread&, const Code& code);
	ReturnState opSqrt(Thread&, const Code& code);
	ReturnState opPow(Thread&, const Code& code);
	ReturnState opLog(Thread&, const Code& code);
	ReturnState opLog10(Thread&, const Code& code);
	ReturnState opLen(Thread&, const Code& code);
	ReturnState opD2r(Thread&, const Code& code);
	ReturnState opR2d(Thread&, const Code& code);

	ReturnState opLod(Thread&, const Code& code);
	ReturnState opSto(Thread&, const Code& code);
	ReturnState opVlod(Thread&, const Code& code);
	ReturnState opVsto(Thread&, const Code& code);
	ReturnState opDup(Thread&, const Code& code);
	ReturnState opSpps(Thread&, const Code& code);
	ReturnState opDel(Thread&, const Code& code);
	ReturnState opCls(Thread&, const Code& code);
	ReturnState opCall(Thread&, const Code& code);
	ReturnState opRet(Thread&, const Code& code);

	ReturnState opPush(Thread&, const Code& code);

	ReturnState opNsAdd(Thread&, const Code& code);
	ReturnState opNsSub(Thread&, const Code& code);
	ReturnState opNsMul(Thread&, const Code& code);
	ReturnState opNsDiv(Thread&, const Code& code);

	ReturnState opPushSb(Thread&, const Code&);
	ReturnState opPopSb(Thread&, const Code&);

	ReturnState opNull(Thread&, const Code& code);
#pragma endregion
}
