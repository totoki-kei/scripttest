#include "script.h"

#include <stdio.h>
#include <stdlib.h>

namespace Script {

	std::shared_ptr<State> CodeProvider::CreateState() {
		return std::make_shared<State>(shared_from_this());
	}

	State::State(std::shared_ptr<CodeProvider> p)
		: provider{ p }
		, workarea((size_t)8)
		, registry{ nullptr } {};

	std::shared_ptr<Thread> State::CreateThread(int ent) {
		return std::make_shared<Thread>(shared_from_this(), ent);
	}

	std::shared_ptr<Thread> State::CreateThread(const char* ent) {
		return std::make_shared<Thread>(shared_from_this(), provider->Label(ent));
	}

	void State::Reset() {
		workarea.clear();
	}

	Thread::Thread(std::shared_ptr<State> s, int ent)
		: state{ s }
		, codeindex{ ent }
		, waitcount{ 0 }
		, errorCode{ OK }
		, stackBase{ 0 } {}

	ReturnState Thread::Run(bool nowait) {
		if (errorCode)
			return Error;

		if (waitcount) {
			waitcount--;
			return Wait;
		}

		while (true) {
			const Code& code = state->provider->Get(codeindex);
			
			if (code.opcode) {
				switch (ReturnState rs = code.opcode(*this, code)) {
					case Wait:
						if (!nowait) break;;
						codeindex++;
						/* fall-through */
					case Error:
					case Finished:
						return rs;
						break;
				}
			}
			codeindex++;
		};

		return Finished;
	};

	void Thread::Reset(int ep) {
		this->callstack.clear();
		this->workstack.clear();
		this->errorCode = ErrorType::OK;
		this->waitcount = 0;
		this->codeindex = ep;
		this->stackBase = 0;
	}

}
