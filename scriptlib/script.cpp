#include "script.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

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
		, errorCode{ ErrorType::OK }
		, stackBase{ 0 } {}

	ReturnState Thread::Run(bool nowait) {
		if (errorCode != ErrorType::OK)
			return ReturnState::Error;

		if (waitcount) {
			waitcount--;
			return ReturnState::Wait;
		}

		while (true) {
			const Code& code = GetCodeProvider()->Get(codeindex);
			
			if (code.opcode) {
				switch (ReturnState rs = code.opcode(*this, code)) {
				case ReturnState::Wait:
						if (nowait) break;;

						codeindex++;
						/* fall-through */
				case ReturnState::Error:
				case ReturnState::Finished:
						return rs;
						break;
				}
			}
			codeindex++;
		};

		return ReturnState::Finished;
	};

	void Thread::Reset(int ep) {
		this->callstack.clear();
		this->workstack.clear();
		this->errorCode = ErrorType::OK;
		this->waitcount = 0;
		this->codeindex = ep;
		this->stackBase = 0;
	}

	Value Thread::StackPop(int n) {
		if (workstack.size() <= stackBase) {
			// 例外を投げる
			throw std::domain_error{ "Stack underflow." };
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
			this->SetErrorCode(ErrorType::WorkstackUnderflow);

			// 例外を投げる
			throw std::domain_error{ "Stack underflow." };
		}
		return workstack.back();
	}

	Value& Thread::StackAt(int index) {
		if (workstack.size() <= stackBase) {
			this->SetErrorCode(ErrorType::WorkstackUnderflow);

			// 例外を投げる
			throw std::domain_error{ "Stack underflow." };
		}
		if (workstack.size() - stackBase < index) {
			this->SetErrorCode(ErrorType::InvalidOpcode);

			// 例外を投げる
			throw std::domain_error{ "Invalid stack index." };
		}

		return workstack.at(stackBase + index);
	}

	unsigned int Thread::StackSize() {
		return workstack.size() - stackBase;
	}

	void Thread::ClearStack() {
		workstack.erase(workstack.begin() + stackBase, workstack.end());
	}

	bool Thread::CheckStack(unsigned int pop, unsigned int /* push ( unused ) */) {
		if (StackSize() < pop) {
			SetErrorCode(ErrorType::WorkstackUnderflow);
			return true;
		}
		//int requiredSize = (int)state->workstack.size() - (int)pop + (int)push;
		//while (requiredSize >= state->workstack.size()) {
		//	state->workstack.resize(state->workstack.size() * 2 + 1);
		//}
		return false;
	}

	ReturnState Thread::FramePush(int n) {
		if (CheckStack(n, 0)) return ReturnState::Error;
		size_t newbase = workstack.size() - n + 1;
		workstack.insert(workstack.begin() + newbase - 1, (int)stackBase);
		stackBase = newbase;
		return ReturnState::None;
	}

	ReturnState Thread::FramePop(int n) {
		if (stackBase == 0) {
			this->SetErrorCode(ErrorType::WorkstackUnderflow);
			return ReturnState::Error;
		}
		if (CheckStack(n, 0)) return ReturnState::Error;

		auto iter = workstack.begin() + stackBase - 1;
		size_t restorebase = (size_t)iter->int_;
		workstack.erase(iter, workstack.end() - n);
		stackBase = restorebase;
		return ReturnState::None;
	}

	ReturnState Thread::GoSub(int addr) {
		if (addr < 0 || GetCodeProvider()->Length() <= addr) {
			SetErrorCode(ErrorType::CodeindexOutOfRange);
			return ReturnState::Error;
		}

		callstack.push_back(codeindex);
		SetCodeIndex(addr - 1);
		return ReturnState::None;
	}

	ReturnState Thread::ReturnSub() {
		if (callstack.size() == 0) {
			SetErrorCode(ErrorType::ScriptHasFinished);
			return ReturnState::Finished;
		}
		SetCodeIndex(callstack.back());
		callstack.pop_back();

		return ReturnState::None;
	}



}
