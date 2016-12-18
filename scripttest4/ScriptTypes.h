#pragma once

#include <boost/variant/variant.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

#if 0
#pragma region General Operator

template <typename TLeft>							 
Error operator +(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator -(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator *(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator /(const T& right) const {		 
	return Error("Bad operation");				 
}												 


template <typename T>							 
Error operator ==(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator !=(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator <(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator <=(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator >(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator >=(const T& right) const {		 
	return Error("Bad operation");				 
}


template <typename T>							 
Error operator &&(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator ||(const T& right) const {		 
	return Error("Bad operation");				 
}


template <typename T>							 
Error operator &(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator |(const T& right) const {		 
	return Error("Bad operation");				 
}												 
												 
template <typename T>							 
Error operator ^(const T& right) const {		 
	return Error("Bad operation");				 
}


template <typename T>							 
Error operator [](const T& right) const {		 
	return Error("Bad operation");				 
}

#pragma endregion
#endif

#define DEFAULT_CONSTRUCTOR(clsname) \
clsname() = default;				 \
clsname(const clsname &) = default;	 \
clsname(clsname &&) = default;		 \
//////////////////////////////////////

namespace Script {

	class Nil;
	class Error;
	class Integer;
	class Number;
	class String;
	class Array;
	class Object;
	class NativePtr;

	using Value = boost::variant <Nil, Error, Number, String, Array, Object, NativePtr>;
	using ValueList = std::vector<Value>;
	using ValueDictionary = std::map<std::string, Value>;


	class Nil {
		DEFAULT_CONSTRUCTOR(Nil);

		// empty class
	};

	class Error {
		std::string what_;
	public:
		DEFAULT_CONSTRUCTOR(Error);
		template <typename T>
		Error(T what) : what_(what) {}
		
		const char* what() const { return what_.c_str(); }
		operator std::exception() const {
			return std::exception(what());
		}
	};
	class Integer {
		int64_t val_;
	public:
		DEFAULT_CONSTRUCTOR(Integer);
		template <typename T>
		Integer(T val) : val_(val) { }

		int64_t value() const { return val_; }
		operator double() const {
			return value();
		}
	};

	class Number {
		double val_;
	public:
		DEFAULT_CONSTRUCTOR(Number);
		template <typename T>
		Number(T val) : val_(val) { }

		double value() const { return val_; }
		operator int64_t() const {
			return value();
		}
	};

	class String {
		std::string str_;
	public:
		DEFAULT_CONSTRUCTOR(String);
		template <typename T>
		String(T str) : str_(str) {}

		const std::string& str() const { return str_; }

		operator const std::string& () const {
			return str_;
		}
		operator const char* () const {
			return str_.c_str();
		}
	};

	class Array {
		std::shared_ptr<ValueList> array_;
	public:
		Array() : array_(std::make_shared<ValueList>()) {}
		Array(const Array&) = default;
		Array(Array&&) = default;

		ValueList& vector() { return *array_; }
		const ValueList& vector() const { return *array_; }
	};

	class Object {
		std::shared_ptr<ValueDictionary> map_;
	public:
		Object() : map_(std::make_shared<ValueDictionary>());
		Object(const Object& obj) = default;
		Object(Object&&) = default;

		ValueDictionary map() { return *map_; }
		const ValueDictionary map() const { return *map_; }
	};

	class NativePtr {
		void* ptr;
	};

}

#undef DEFAULT_CONSTRUCTOR
