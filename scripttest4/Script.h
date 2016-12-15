#pragma once

#include <boost/variant/variant.hpp>
#include <cstdint>
#include <string>

namespace Script {
	class Number;
	class String;
	class Array;
	class Object;
	class NativePtr;

	struct Add;
}

namespace Script {

	//using Value = boost::variant <Number, String, Array, Object, NativePtr>;
	using Value = boost::variant <int, double>;


	struct Add : boost::static_visitor<Value> {
		const Value& left_;
		const Value& right_;

		Add() = delete;
		Add(const Add&) = default;
		Add(Add&&) = default;

		Add(const Value& left, const Value& right)
			: left_(left), right_(right)
		{ }

		Value operator ()() {
			// ステップ1：左辺の型を展開
			return left_.apply_visitor(*this);
		}

		template <typename LeftT>
		Value operator() (const LeftT left) {
			// ステップ2：右辺の型を展開
			return right_.apply_visitor(Visitor<LeftT>(left));
		}

		template <typename LeftT>
		struct Visitor : boost::static_visitor<Value> {
			const LeftT& left_;
			Visitor(const LeftT& left) : left_(left) { }

			template <typename RightT>
			Value operator ()(const RightT& right) {
				// ステップ3：実計算を実施
				return left_ + right;
			}
		};

	};

	// ↓ 汎用化

	template <typename Op>
	struct BinaryOperation : boost::static_visitor<Value> {
		const Value& left_;
		const Value& right_;

		BinaryOperation() = delete;
		BinaryOperation(const Add&) = default;
		BinaryOperation(Add&&) = default;

		BinaryOperation(const Value& left, const Value& right)
			: left_(left), right_(right)
		{ }

		Value operator ()() {
			// ステップ1：左辺の型を展開
			return left_.apply_visitor(*this);
		}

		template <typename LeftT>
		Value operator() (const LeftT left) {
			// ステップ2：右辺の型を展開
			return right_.apply_visitor(Visitor<LeftT>(left));
		}

		template <typename LeftT>
		struct Visitor : boost::static_visitor<Value> {
			const LeftT& left_;
			Visitor(const LeftT& left) : left_(left) { }

			template <typename RightT>
			Value operator ()(const RightT& right) {
				// ステップ3：実計算を実施
				return Op()(left_, right);
			}
		};

	};

	void hoge() {
		Value val1 = 1, val2 = 2.5;
		
		Value result = Add(val1, val2)();
		
	}
}
