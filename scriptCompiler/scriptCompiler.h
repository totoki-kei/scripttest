#pragma once

#include "../scriptlib/script.h"
#include "../scriptlib/scriptOp.h"

namespace Script {
	namespace Compiler {

		enum ValueType {
			VAL_NONE,
			VAL_NUMBER,
			VAL_INTEGER,
			VAL_POINTER,
		};

		enum FunctionType {
			FN_OPERATOR_LEFT,
			FN_OPERATOR_RIGHT,
			FN_FUNCTION,
		};

		struct Function {
			FunctionType Type;
			ValueType *pArguments;
			ValueType *pReturns;
		};

	}
}


