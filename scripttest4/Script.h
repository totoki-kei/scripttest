#pragma once

/*

class Nil;
class Error;
class Integer;
class Number;
class String;
class Array;
class Object;
class NativePtr;

using Value = boost::variant <Nil, Error, Number, String, Array, Object, NativePtr>;

*/

#include "ScriptTypes.h"

#include <boost/variant/multivisitors.hpp>

namespace Script {

	//namespace Visitors {

	//	struct BinAdd;
	//	struct BinSub;
	//	struct BinMul;
	//	struct BinDiv;
	//	struct BinEquals;
	//	struct BinNotEquals;
	//	struct BinLessThan;
	//	struct BinLessEqual;
	//	struct BinGreaterThan;
	//	struct BinGreaterEqual;
	//	struct BinAnd;
	//	struct BinOr;
	//	struct BinBitAnd;
	//	struct BinBitOr;
	//	struct BinBitXor;
	//	struct BinIndex;

	//	struct UniBool; // operator bool => Integer
	//	struct UniNeg;
	//	struct UniNot;
	//	struct UniLen;
	//	struct UniTypeIndex;
	//	struct UniTypeName;

	//	struct UniStr;
	//	struct UniNum;

	//	struct Prop;
	//}
}

//namespace Script {
//
//	
//	//using Value = boost::variant <int, double>;
//
//#if 0
//	struct Add : boost::static_visitor<Value> {
//		const Value& left_;
//		const Value& right_;
//
//		Add() = delete;
//		Add(const Add&) = default;
//		Add(Add&&) = default;
//
//		Add(const Value& left, const Value& right)
//			: left_(left), right_(right)
//		{ }
//
//		Value operator ()() {
//			// ステップ1：左辺の型を展開
//			return left_.apply_visitor(*this);
//		}
//
//		template <typename LeftT>
//		Value operator() (const LeftT left) {
//			// ステップ2：右辺の型を展開
//			return right_.apply_visitor(Visitor<LeftT>(left));
//		}
//
//		template <typename LeftT>
//		struct Visitor : boost::static_visitor<Value> {
//			const LeftT& left_;
//			Visitor(const LeftT& left) : left_(left) { }
//
//			template <typename RightT>
//			Value operator ()(const RightT& right) {
//				// ステップ3：実計算を実施
//				return left_ + right;
//			}
//		};
//
//	};
//
//	// ↓ 汎用化
//#endif
//	template <typename Op>
//	struct BinaryOperation : boost::static_visitor<Value> {
//		const Value& left_;
//		const Value& right_;
//
//		BinaryOperation() = delete;
//		BinaryOperation(const BinaryOperation<Op>&) = default;
//		BinaryOperation(BinaryOperation<Op>&&) = default;
//
//		BinaryOperation(const Value& left, const Value& right)
//			: left_(left), right_(right)
//		{ }
//
//		Value operator ()() {
//			// ステップ1：左辺の型を展開
//			return left_.apply_visitor(*this);
//		}
//
//		template <typename LeftT>
//		Value operator() (const LeftT left) {
//			// ステップ2：右辺の型を展開
//			return right_.apply_visitor(Visitor<LeftT>(left));
//		}
//
//		template <typename LeftT>
//		struct Visitor : boost::static_visitor<Value> {
//			const LeftT& left_;
//			Visitor(const LeftT& left) : left_(left) { }
//
//			template <typename RightT>
//			Value operator ()(const RightT& right) {
//				// ステップ3：実計算を実施
//				return Op()(left_, right);
//			}
//		};
//
//	};
//
//	struct Adder {
//		template <typename Left, typename Right>
//		auto operator ()(const Left& left, const Right& right) -> decltype(left + right)
//		{
//			return left + right;
//		}
//	};
//
//	void hoge() {
//		Value val1 = Integer(1), val2 = Number(2.5);
//		
//		//Value result = Add(val1, val2)();
//		
//
//		Value result = BinaryOperation<Adder>(val1, val2)();
//		
//		
//	}
//
//	void fuga() {
//	}
//}
