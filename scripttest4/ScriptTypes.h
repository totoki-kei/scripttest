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
	// 前方宣言
	class TypeTrait;
	class NilType;
	class IntType;
	class NumberType;
	class StringType;
	class DictionaryType;

	/// <summary>
	/// 変数データを格納するメモリ領域
	/// </summary>
	struct ValueStorage {
		union {
			void* ptr;
			double number;
			int64_t integer;
		};

		ValueStorage() : integer(0) {}

		void FillZero() {
			integer = 0;
		}
	};

	static_assert(sizeof(ValueStorage) == sizeof(void*), "unexpected ValueStorage size.");

	/// <summary>
	/// 型情報とメモリ領域を持った変数情報
	/// </summary>
	struct Value {
		ValueStorage stor;
		TypeTrait* type;
		long ref;
		void* tag;


	};



	// Valueのサイズはポインタ４つ分とする
	static_assert(sizeof(Value) == sizeof(void*) * 4, "unexpected Value size.");

	/// <summary>
	/// 型情報
	/// </summary>
	class TypeTrait {
	public:

		virtual ~TypeTrait() = 0;

		/// <summary>
		/// 型名を取得する
		/// </summary>
		/// <returns></returns>
		virtual const char* GetName() const = 0;

		/// <summary>
		/// デフォルトコンストラクタ
		/// 指定のValueStorageにこの型の初期値を設定する
		/// </summary>
		virtual void DefaultConstruct(ValueStorage& stor) {
			stor.FillZero();
		};

		/// <summary>
		/// コピーコンストラクタ
		/// ValueStorage間で値をコピーする
		/// </summary>
		virtual bool CopyConstruct(ValueStorage& stor, const ValueStorage& val) {
			stor = val;
			return true;
		}
		
		/// <summary>
		/// デストラクタ
		/// 指定のValueStorage内の値を破棄する
		/// </summary>
		virtual void Destruct(ValueStorage& stor) {
			
		};

		/// <summary>
		/// 型変換結果コード
		/// </summary>
		enum ConversionResult {
			/// <summary>
			/// 変換に失敗
			/// </summary>
			CONVERSION_FAIL = 0,
			/// <summary>
			/// 変換成功
			/// </summary>
			CONVERSION_SUCCESS,
			/// <summary>
			/// 変換は不要のため実施されなかった
			/// </summary>
			CONVERSION_NOT_NECESSARY,
		};


		/// <summary>
		/// 変換(他の型 => この型)
		/// val値をこの型に変換し更新する
		/// 変換に成功した場合trueを返す
		/// </summary>
		virtual ConversionResult ConvertFrom(const ValueStorage & stor, const TypeTrait* from, ValueStorage& out_result) {
			if (from == this) return CONVERSION_NOT_NECESSARY;
			return CONVERSION_FAIL;
		}

		/// <summary>
		/// 変換(この型 => 他の型)
		/// 現在の型の値であるvalを、変換先の型に更新する
		/// 変換に成功した場合trueを返す
		/// </summary>
		virtual ConversionResult ConvertTo(const ValueStorage & stor, const TypeTrait* to, ValueStorage& out_result) {
			if (to == this) return CONVERSION_NOT_NECESSARY;
			return CONVERSION_FAIL;
		}

		/*
		方針
		複雑な型に変換関数を多めに実装する。

		(1) 先にConvertToを行う
			-> 元の型の変換方針を尊重
		(2) ConvertToに失敗した場合はConvertFromを行う
			-> 複雑な型の場合、こちらを使う(Construct方法などを知っているため)
		(3) 両方ともに失敗した場合は変換失敗と見なす(スクリプト実行時エラー)
		*/

		/*
		その他懸案事項
		・CanConvertFrom / CanConvertTo は必要か？
		・強制的に変換する機能は必要か
		*/

	};

	template <typename TValType>
	TypeTrait* TraitOf() {
		static TValType* trait_instance;
		return &trait_instance;
	}

	/// <summary>
	/// 組み込み型 Nil の型情報
	/// </summary>
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

	/// <summary>
	/// 組み込み型 Int の型情報
	/// </summary>
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

	/// <summary>
	/// 組み込み型 Number の型情報
	/// </summary>
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

	class StringType : public TypeTrait {
	public:
		using Base = TypeTrait;
		const char* GetName() const override {
			return "String";
		}
		void DefaultConstruct(ValueStorage& stor) override {
			stor.ptr = new std::string();
		};
		bool CopyConstruct(ValueStorage& stor, const ValueStorage& val) override {
			auto* str = new std::string();
			auto* left_str = static_cast<std::string*>(val.ptr);
			str->assign(*left_str);
			stor.ptr = str;
		}

		void Destruct(ValueStorage& stor) override {
			auto* str = static_cast<std::string*>(stor.ptr);
			delete str;
		}
	};


	class DictionaryType : public TypeTrait {
		using Base = TypeTrait;

		using MapType = std::map<std::string, Value>;

		const char* GetName() const override {
			return "Dictionary";
		}

		void DefaultConstruct(ValueStorage& stor) override {
			stor.ptr = new MapType();
		};

		bool CopyConstruct(ValueStorage& stor, const ValueStorage& val) override {
			auto* map = new MapType();
			auto* left_map = static_cast<MapType*>(val.ptr);

			for (auto& keyval : *left_map) {
				std::string newkey;

				newkey = keyval.first;
				
				Value newval = {};
				keyval.second.type->CopyConstruct(newval.stor, keyval.second.stor);
				newval.type = keyval.second.type;
				newval.ref = 1;
				newval.tag = nullptr;

				map->insert(std::make_pair(newkey, newval));
			}
		}

		void Destruct(ValueStorage& stor) override {
			auto* map = static_cast<MapType*>(stor.ptr);
			for (auto& keyval : *map) {
				keyval.second.type->Destruct(keyval.second.stor);
			}
			delete map;
		}
	};



}

#undef DEFAULT_CONSTRUCTOR
