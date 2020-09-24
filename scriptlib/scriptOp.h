#pragma once

#include "script.h"
namespace Script {
	namespace Loader {
		void BuildOpcodes(std::unordered_map<std::string, Loader::CodeSkelton>& map);
	}

#pragma region Operation Definitions
	ReturnState opEnd(Thread&, const Code&);
	ReturnState opWait(Thread&, const Code&);

	ReturnState opGoto(Thread&, const Code&);
	ReturnState opJmp(Thread&, const Code&);
	//ReturnState opCpt(Thread&, const Code&);
	ReturnState opFwd(Thread&, const Code&);
	ReturnState opRew(Thread&, const Code&);
	ReturnState opJz(Thread&, const Code&);
	ReturnState opJnz(Thread&, const Code&);
	ReturnState opJpos(Thread&, const Code&);
	ReturnState opJneg(Thread&, const Code&);
	ReturnState opJeq(Thread&, const Code&);
	ReturnState opJne(Thread&, const Code&);
	ReturnState opJgt(Thread&, const Code&);
	ReturnState opJge(Thread&, const Code&);
	ReturnState opJlt(Thread&, const Code&);
	ReturnState opJle(Thread&, const Code&);
	ReturnState opCmp(Thread&, const Code&);
	ReturnState opIs(Thread&, const Code&);

	ReturnState opAdd(Thread&, const Code&);
	ReturnState opAdds(Thread&, const Code&);
	ReturnState opMul(Thread&, const Code&);
	ReturnState opMuls(Thread&, const Code&);
	ReturnState opSub(Thread&, const Code&);
	ReturnState opNeg(Thread&, const Code&);
	ReturnState opDiv(Thread&, const Code&);
	ReturnState opMod(Thread&, const Code&);
	ReturnState opSin(Thread&, const Code&);
	ReturnState opCos(Thread&, const Code&);
	ReturnState opSinCos(Thread&, const Code&);
	ReturnState opCosSin(Thread&, const Code&);
	ReturnState opTan(Thread&, const Code&);
	ReturnState opArg(Thread&, const Code&);
	ReturnState opSqrt(Thread&, const Code&);
	ReturnState opPow(Thread&, const Code&);
	ReturnState opLog(Thread&, const Code&);
	ReturnState opLog10(Thread&, const Code&);
	ReturnState opLen(Thread&, const Code&);
	ReturnState opD2r(Thread&, const Code&);
	ReturnState opR2d(Thread&, const Code&);
	ReturnState opAbs(Thread&, const Code&);
	ReturnState opRound(Thread&, const Code&);
	ReturnState opTrunc(Thread&, const Code&);
	ReturnState opFloor(Thread&, const Code&);
	ReturnState opCeil(Thread&, const Code&);

	ReturnState opI2n(Thread&, const Code&);
	ReturnState opN2i(Thread&, const Code&);
	ReturnState opIPush(Thread&, const Code&);
	ReturnState opILsh(Thread&, const Code&);
	ReturnState opIRsh(Thread&, const Code&);
	ReturnState opIAnd(Thread&, const Code&);
	ReturnState opIOr(Thread&, const Code&);
	ReturnState opIXor(Thread&, const Code&);
	ReturnState opIBool(Thread&, const Code&);

	ReturnState opLod(Thread&, const Code&);
	ReturnState opSto(Thread&, const Code&);
	ReturnState opVlod(Thread&, const Code&);
	ReturnState opVsto(Thread&, const Code&);
	ReturnState opDup(Thread&, const Code&);
	ReturnState opSpps(Thread&, const Code&);
	ReturnState opDel(Thread&, const Code&);
	ReturnState opCls(Thread&, const Code&);
	ReturnState opCall(Thread&, const Code&);
	ReturnState opRet(Thread&, const Code&);

	ReturnState opPush(Thread&, const Code&);
	ReturnState opStkLen(Thread&, const Code&);
	ReturnState opSLod(Thread&, const Code&);
	ReturnState opSSto(Thread&, const Code&);

	ReturnState opGStkLen(Thread&, const Code&);
	ReturnState opGLod(Thread&, const Code&);
	ReturnState opGSto(Thread&, const Code&);


	ReturnState opNsAdd(Thread&, const Code&);
	ReturnState opNsSub(Thread&, const Code&);
	ReturnState opNsMul(Thread&, const Code&);
	ReturnState opNsDiv(Thread&, const Code&);

	ReturnState opPushSb(Thread&, const Code&);
	ReturnState opPopSb(Thread&, const Code&);

	ReturnState opNull(Thread&, const Code&);
#pragma endregion
}
