#pragma once

#include <memory>

namespace Script {
	namespace Compiler {


		struct AstNode : public std::enable_shared_from_this<AstNode> {
			virtual void Emit() = 0;
		};

		struct UnaryNode : public AstNode {
			enum {
				OP_NEGATE,
				OP_SIN,
				OP_COS,
				OP_NOT,
			} op;
		};
	}
}


