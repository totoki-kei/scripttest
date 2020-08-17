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
	// �O���錾
	class TypeTrait;
	class NilType;
	class IntType;
	class NumberType;
	class StringType;
	class ObjectType;


	struct ValueStorage {
		union {
			void* ptr;
			double number;
			int64_t integer;
		};

		void FillZero() {
			integer = 0;
		}
	};


	struct Value {
		ValueStorage stor;
		TypeTrait* type;
		long ref;
		void* tag;


	};

	static_assert(sizeof(ValueStorage) == 64 / 8, "unexpected ValueStorage size.");

	class TypeTrait {
	public:

		virtual ~TypeTrait() = 0;

		virtual const char* GetName() const = 0;

		/// <summary>
		/// �f�t�H���g�R���X�g���N�^
		/// �^�̏����l��ݒ肷��
		/// </summary>
		virtual void DefaultConstruct(ValueStorage& stor) {
			stor.FillZero();
		};

		virtual bool CopyConstruct(ValueStorage& stor, const ValueStorage& val) {
			stor = val;
			return true;
		}
		
		/// <summary>
		/// �f�X�g���N�^
		/// �l��j������
		/// </summary>
		virtual void Destruct(ValueStorage& stor) {
			
		};


		enum ConversionResult {
			CONVERSION_FAIL = 0,
			CONVERSION_SUCCESS,
			CONVERSION_NOT_NECESSARY,
		};


		/// <summary>
		/// �ϊ�(���̌^ => ���̌^)
		/// val�l�����̌^�ɕϊ����X�V����
		/// �ϊ��ɐ��������ꍇtrue��Ԃ�
		/// </summary>
		virtual ConversionResult ConvertFrom(const ValueStorage & stor, const TypeTrait* from, ValueStorage& out_result) {
			if (from == this) return CONVERSION_NOT_NECESSARY;
			return CONVERSION_FAIL;
		}

		/// <summary>
		/// �ϊ�(���̌^ => ���̌^)
		/// ���݂̌^�̒l�ł���val���A�ϊ���̌^�ɍX�V����
		/// �ϊ��ɐ��������ꍇtrue��Ԃ�
		/// </summary>
		virtual ConversionResult ConvertTo(const ValueStorage & stor, const TypeTrait* to, ValueStorage& out_result) {
			if (to == this) return CONVERSION_NOT_NECESSARY;
			return CONVERSION_FAIL;
		}

		/*
		���j
		���G�Ȍ^�ɕϊ��֐��𑽂߂Ɏ�������B

		(1) ���ConvertTo���s��
			=> ���̌^�̕ϊ����j�𑸏d
		(2) ConvertTo�Ɏ��s�����ꍇ��ConvertFrom���s��
			-> ���G�Ȍ^�̏ꍇ�A��������g��(Construct���@�Ȃǂ�m���Ă��邽��)
		(3) �����Ƃ��Ɏ��s�����ꍇ�͕ϊ����s�ƌ��Ȃ�(�X�N���v�g���s���G���[)
		*/

		/*
		���̑����Ď���
		�ECanConvertFrom / CanConvertTo �͕K�v���H
		�E�����I�ɕϊ�����@�\�͕K�v��
		*/

	};

	template <typename TValType>
	TypeTrait* TraitOf() {
		static TValType* trait_instance;
		return &trait_instance;
	}


	class NilType : public TypeTrait {
	public:
		using Base = TypeTrait;

		const char* GetName() const override {
			return "Nil";
		}

		void DefaultConstruct(ValueStorage& stor) override {
			stor.integer = 'Nil\0';
		}
	};

	class IntType : public TypeTrait {
	public:
		using Base = TypeTrait;

		const char* GetName() const override {
			return "Int";
		}

		ConversionResult ConvertFrom(const ValueStorage& stor, const TypeTrait* from, ValueStorage& out_result) override {
			if (from == TraitOf<NumberType>()) {
				out_result.integer = static_cast<int64_t>(stor.number);
				return CONVERSION_SUCCESS;
			}

			return Base::ConvertFrom(stor, from, out_result);
		}

	};

	class NumberType : public TypeTrait {
	public:
		using Base = TypeTrait;

		const char* GetName() const override {
			return "Number";
		}

		ConversionResult ConvertFrom(const ValueStorage& stor, const TypeTrait* from, ValueStorage& out_result) override {
			if (from == TraitOf<IntType>()) {
				out_result.number = static_cast<double>(stor.integer);
				return CONVERSION_SUCCESS;
			}

			return Base::ConvertFrom(stor, from, out_result);
		}
	};
	class StringType;


	class ObjectType : public TypeTrait {
		using Base = TypeTrait;

		using MapType = std::map<std::string, Value>;

		const char* GetName() const override {
			return "Object";
		}

		void DefaultConstruct(ValueStorage& stor) override {
			stor.ptr = new MapType();
		};

		bool CopyConstruct(ValueStorage& stor, const ValueStorage& val) override {
			auto* map = new MapType();
			auto* left_map = static_cast<MapType*>(val.ptr);

			for (auto& keyval : *left_map) {
				MapType::value_type newval;
				newval.first = keyval.first;
				
				keyval.second.type->CopyConstruct(newval.second.stor, keyval.second.stor);
				newval.second.type = keyval.second.type;
				newval.second.ref = 1;
				newval.second.tag = nullptr;

				map->insert(newval);
			}
		}

	};



}

#undef DEFAULT_CONSTRUCTOR
