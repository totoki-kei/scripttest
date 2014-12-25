#include "script.h"
#include <stdio.h>
#include <stdlib.h>

namespace Script {

	std::shared_ptr<State> CodeProvider::CreateState() {
		return std::make_shared<State>(shared_from_this());
	}

	State::State(std::shared_ptr<CodeProvider> p)
		: provider{ p }
		, workarea((size_t)8) {};

	State::~State() {};

	std::shared_ptr<Thread> State::CreateThread(int ent) {
		return std::make_shared<Thread>(shared_from_this(), ent);
	}

	std::shared_ptr<Thread> State::CreateThread(const char* ent) {
		return std::make_shared<Thread>(shared_from_this(), provider->EntryPoint(ent));
	}

	void State::Reset() {
		workarea.clear();
	}

	Thread::Thread(std::shared_ptr<State> s, int ent)
		: state{ s }
		, codeindex{ ent }
		, waitcount{ 0 }
		, errorCode{ OK } {}

	ReturnState Thread::Run() {
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
					case Error:
					case Wait:
					case Finished:
						return rs;
						break;
				}
			}
			codeindex++;
		};

		return Finished;
	};

}
