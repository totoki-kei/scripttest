#pragma once

#ifndef SCRIPEXCEPTION_H_
#define SCRIPEXCEPTION_H_

#include <stdexcept>
#include <string>

// —áŠO’è‹`
namespace Scrip {
	/*************************************************/
#define DECLARE_EXCEPTION(name, base)             \
	class name : public base {                    \
		public:                                   \
		name(std::string_view msg) : base(msg) {} \
	}                                             \
/*************************************************/
	class Exception : std::runtime_error {
		std::string message;
	public:
		Exception(std::string_view msg) : std::runtime_error(std::string(msg)), message(msg) {}
		const char* what() const noexcept override {
			return message.c_str();
		}
	};


	DECLARE_EXCEPTION(CompileErrorException, Exception);
	DECLARE_EXCEPTION(SyntaxErrorException, CompileErrorException);
	DECLARE_EXCEPTION(CompilationStackOverflowException, CompileErrorException);

	DECLARE_EXCEPTION(RuntimeErrorException, Exception);

#undef DECLARE_EXCEPTION
}

#endif // SCRIPEXCEPTION_H_
