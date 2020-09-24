/* これをincludeする前に、以下のマクロを定義する */
// MAKEOP(name, op, attr)

#ifndef MAKEOP
#warning MAKEOP macro is not defined.
#define MARKUP_DUMMY_IMPL
#define MAKEOP(name, op, attr) /* dummy */
#endif

// 以下は、必要に応じてオーバーライドする
#ifndef MAKEOP_INT
#define MAKEOP_INT_DEFAULT_IMPL
#define MAKEOP_INT(name, op) MAKEOP(name, op, AttrType::Integer)
#endif

#ifndef MAKEOP_FLOAT
#define MAKEOP_FLOAT_DEFAULT_IMPL
#define MAKEOP_FLOAT(name, op) MAKEOP(name, op, AttrType::Float)
#endif

#ifndef MAKEOP_CMP
#define MAKEOP_CMP_DEFAULT_IMPL
#define MAKEOP_CMP(name, op) MAKEOP(name, op, AttrType::Comparer)
#endif

#ifndef MAKEOP_NT
#define MAKEOP_NT_DEFAULT_IMPL
#define MAKEOP_NT(name, op) MAKEOP(name, op, AttrType::NumType)
#endif

#ifndef MAKEOP_ENTRYPOINT
#define MAKEOP_ENTRYPOINT_DEFAULT_IMPL
#define MAKEOP_ENTRYPOINT(name, op) MAKEOP(name, op, AttrType::EntryPointSymbol)
#endif

#ifndef MAKEOP_PROP
#define MAKEOP_PROP_DEFAULT_IMPL
#define MAKEOP_PROP(name, op) MAKEOP(name, op, AttrType::Property)
#endif

#ifndef MAKEOP_STR
#define MAKEOP_STR_DEFAULT_IMPL
#define MAKEOP_STR(name, op) MAKEOP(name, op, AttrType::String)
#endif

#ifndef MAKEOP_UNIT
#define MAKEOP_UNIT_DEFAULT_IMPL
#define MAKEOP_UNIT(name, op) MAKEOP(name, op, AttrType::Integer)
#endif


MAKEOP_UNIT(nop, opNull)

MAKEOP_INT(wait, opWait)
MAKEOP_UNIT(end, opEnd)

MAKEOP_ENTRYPOINT(warp, opGoto)

MAKEOP_INT(jump, opJmp)
MAKEOP_INT(jump_eq, opJeq)
MAKEOP_INT(jump_neq, opJne)
MAKEOP_INT(jump_gt, opJgt)
MAKEOP_INT(jump_geq, opJge)
MAKEOP_INT(jump_lt, opJlt)
MAKEOP_INT(jump_leq, opJle)
MAKEOP_INT(jump_zero, opJz)
MAKEOP_INT(jump_nonzero, opJnz)
MAKEOP_INT(jump_pos, opJpos)
MAKEOP_INT(jump_neg, opJneg)

MAKEOP_CMP(cmp, opCmp)
MAKEOP_NT(chk, opIs)

MAKEOP_INT(fwd, opFwd)
MAKEOP_INT(rew, opRew)

MAKEOP_FLOAT(add, opAdd)
MAKEOP_INT(adds, opAdds)
MAKEOP_FLOAT(mul, opMul)
MAKEOP_INT(muls, opMuls)
MAKEOP_FLOAT(sub, opSub)
MAKEOP_FLOAT(neg, opNeg)
MAKEOP_FLOAT(div, opDiv)
MAKEOP_FLOAT(mod, opMod)
MAKEOP_FLOAT(sin, opSin)
MAKEOP_FLOAT(cos, opCos)
MAKEOP_FLOAT(sincos, opSinCos)
MAKEOP_FLOAT(cossin, opCosSin)
MAKEOP_FLOAT(tan, opTan)
MAKEOP_FLOAT(atan, opArg)
MAKEOP_FLOAT(sqrt, opSqrt)
MAKEOP_FLOAT(pow, opPow)
MAKEOP_FLOAT(log, opLog)
MAKEOP_FLOAT(ln, opLog10)
MAKEOP_INT(len, opLen)
MAKEOP_FLOAT(deg2rad, opD2r)
MAKEOP_FLOAT(rad2deg, opR2d)
MAKEOP_FLOAT(abs, opAbs)
MAKEOP_FLOAT(round, opRound)
MAKEOP_FLOAT(trunc, opTrunc)
MAKEOP_FLOAT(floor, opFloor)
MAKEOP_FLOAT(ceil, opCeil)

MAKEOP_INT(int2num, opI2n)
MAKEOP_UNIT(num2int, opN2i)
MAKEOP_INT(ipush, opIPush)
MAKEOP_INT(ilsh, opILsh)
MAKEOP_INT(irsh, opIRsh)
MAKEOP_UNIT(iand, opIAnd)
MAKEOP_UNIT(ior, opIOr)
MAKEOP_UNIT(ixor, opIXor)
MAKEOP_UNIT(ibool, opIBool)

MAKEOP_INT(get, opLod)
MAKEOP_INT(set, opSto)
//MAKEOP_INT(vget, opVlod)
//MAKEOP_INT(vset, opVsto)
MAKEOP_NT(n, opSpps)

MAKEOP_INT(dup, opDup)
MAKEOP_INT(pop, opDel)
MAKEOP_INT(lget, opSLod)
MAKEOP_INT(lset, opSSto)
MAKEOP_INT(clear, opCls)

MAKEOP_ENTRYPOINT(call, opCall)
MAKEOP_UNIT(ret, opRet)

MAKEOP_INT(enter, opPushSb)
MAKEOP_INT(leave, opPopSb)

MAKEOP_FLOAT(push, opPush)
MAKEOP_UNIT(stklen, opStkLen)

MAKEOP_INT(dadd, opNsAdd)
MAKEOP_INT(dsub, opNsSub)
MAKEOP_INT(dmul, opNsMul)
MAKEOP_INT(ddiv, opNsDiv)


#ifdef MAKEOP_INT_DEFAULT_IMPL
#undef MAKEOP_INT_DEFAULT_IMPL
#undef MAKEOP_INT
#endif

#ifdef MAKEOP_FLOAT_DEFAULT_IMPL
#undef MAKEOP_FLOAT_DEFAULT_IMPL
#undef MAKEOP_FLOAT
#endif

#ifdef MAKEOP_CMP_DEFAULT_IMPL
#undef MAKEOP_CMP_DEFAULT_IMPL
#undef MAKEOP_CMP
#endif

#ifdef MAKEOP_NT_DEFAULT_IMPL
#undef MAKEOP_NT_DEFAULT_IMPL
#undef MAKEOP_NT
#endif

#ifdef MAKEOP_ENTRYPOINT_DEFAULT_IMPL
#undef MAKEOP_ENTRYPOINT_DEFAULT_IMPL
#undef MAKEOP_ENTRYPOINT
#endif

#ifdef MAKEOP_PROP_DEFAULT_IMPL
#undef MAKEOP_PROP_DEFAULT_IMPL
#undef MAKEOP_PROP
#endif

#ifdef MAKEOP_STR_DEFAULT_IMPL
#undef MAKEOP_STR_DEFAULT_IMPL
#undef MAKEOP_STR
#endif

#ifdef MAKEOP_UNIT_DEFAULT_IMPL
#undef MAKEOP_UNIT_DEFAULT_IMPL
#undef MAKEOP_UNIT
#endif

#ifdef MARKUP_DUMMY_IMPL
#undef MARKUP_DUMMY_IMPL
#undef MAKEUP
#endif


