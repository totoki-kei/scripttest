#pragma once

#include <boost/variant/variant.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
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
	};
	class Integer {
		int64_t val_;
	public:
		DEFAULT_CONSTRUCTOR(Integer);
		template <typename T>
		Integer(T val) : val_(val) { }

		int64_t value() const { return val_; }
	};

	class Number {
		double val_;
	public:
		DEFAULT_CONSTRUCTOR(Number);
		template <typename T>
		Number(T val) : val_(val) { }

		double value() const { return val_; }
	};

	class String {
		std::string str_;
	public:
		DEFAULT_CONSTRUCTOR(String);
		template <typename T>
		String(T str) : str_(str) {}

		const std::string& str() const { return str_; }
	};

	class Array {
		std::shared_ptr<ValueList> array_;
	public:
		Array() : array_(std::make_shared<ValueList>()) {}
		Array(const Array&) = default;
		Array(Array&&) = default;

		ValueList* vector() { return array_.get(); }
		const ValueList* vector() const { return array_.get(); }
	};

	class Object {
		std::shared_ptr<ValueDictionary> map_;
	public:
		Object() : map_(std::make_shared<ValueDictionary>()) {};
		Object(const Object& obj) = default;
		Object(Object&&) = default;

		ValueDictionary* map() { return map_.get(); }
		const ValueDictionary* map() const { return map_.get(); }
	};

	class NativePtr {
		void* ptr_;
		
		struct Tag {
			int refcount;
			std::function<void(void*)> deleter;
		};
		Tag* tag_;

	public:
		NativePtr() : ptr_(nullptr), tag_(nullptr) {}
		NativePtr(const NativePtr& p) : ptr_(p.ptr_), tag_(p.tag_) {
			if (tag_) tag_->refcount++;
		}
		NativePtr(NativePtr&& p) : ptr_(p.ptr_), tag_(p.tag_) {
			// é”Ç¡ÇΩdeleteÇîÇØÇÈÇΩÇﬂÉÄÅ[Éuå≥Ç©ÇÁè¡Ç∑
			p.ptr_ = nullptr;
			p.tag_ = nullptr;
		}

		NativePtr(void* ptr) : ptr_(ptr), tag_(nullptr) { }

		template<typename Fn>
		NativePtr(void* ptr, Fn deleter) : ptr_(ptr), tag_(new Tag()) {
			tag_->deleter = deleter;
			tag_->refcount = 1;
		}

		NativePtr& operator =(const NativePtr& right) {
			if (this == &right) return *this;
			this->ptr_ = right.ptr_;
			this->tag_ = right.tag_;
			if (tag_) tag_->refcount++;

			return *this;
		}

		NativePtr& operator =(NativePtr&& right) {
			if (this == &right) return *this;
			this->ptr_ = right.ptr_;
			this->tag_ = right.tag_;
			right.ptr_ = nullptr;
			right.tag_ = nullptr;

			return *this;
		}

		~NativePtr()
		{
			if (tag_) {
				if (!--tag_->refcount) {
					tag_->deleter(ptr_);
					delete tag_;
				}
			}
			tag_ = nullptr;
			ptr_ = nullptr;
		}

		void* ptr() const { return ptr_; }
	};

}

#undef DEFAULT_CONSTRUCTOR
