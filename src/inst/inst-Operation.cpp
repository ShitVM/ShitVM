#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

#include <cmath>
#include <type_traits>

namespace svm {
	template<typename T>
	SVM_NOINLINE_FOR_PROFILING bool Interpreter::PopTwoSameTypeAndPushOne(const Type* rhsTypePtr, T*& lhs, const T*& rhs) noexcept {
		if (IsLocalVariable() || IsLocalVariable(sizeof(T))) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return false;
		}

		Type* const lhsTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - sizeof(T));
		if (!lhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return false;
		} else if (*lhsTypePtr != *rhsTypePtr) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return false;
		}

		lhs = reinterpret_cast<T*>(lhsTypePtr);
		rhs = reinterpret_cast<const T*>(rhsTypePtr);
		m_Stack.Reduce(sizeof(T));
		return true;
	}

	template<typename T>
	SVM_NOINLINE_FOR_PROFILING bool Interpreter::PopTwoSameType(const Type* rhsTypePtr, T& lhs, T& rhs) noexcept {
		if (IsLocalVariable() || IsLocalVariable(sizeof(T))) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return false;
		}

		Type* const lhsTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - sizeof(T));
		if (!lhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return false;
		} else if (*lhsTypePtr != *rhsTypePtr) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return false;
		}

		lhs = *reinterpret_cast<T*>(lhsTypePtr);
		rhs = *reinterpret_cast<const T*>(rhsTypePtr);
		m_Stack.Reduce(sizeof(T) * 2);
		return true;
	}
	template<typename T>
	SVM_NOINLINE_FOR_PROFILING IntObject Interpreter::CompareTwoSameType(T lhs, T rhs) noexcept {
		if (lhs > rhs) {
			return 1;
		} else if (lhs == rhs) {
			return 0;
		} else {
			return static_cast<std::uint32_t>(-1);
		}
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAdd() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value += rhs->Value;
		} else if (rhsType == LongType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value += rhs->Value;
		} else if (rhsType == SingleType) {
			SingleObject* lhs = nullptr;
			const SingleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value += rhs->Value;
		} else if (rhsType == DoubleType) {
			DoubleObject* lhs = nullptr;
			const DoubleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value += rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSub() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value -= rhs->Value;
		} else if (rhsType == LongType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value -= rhs->Value;
		} else if (rhsType == SingleType) {
			SingleObject* lhs = nullptr;
			const SingleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value -= rhs->Value;
		} else if (rhsType == DoubleType) {
			DoubleObject* lhs = nullptr;
			const DoubleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value -= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretMul() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value *= rhs->Value;
		} else if (rhsType == LongType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value *= rhs->Value;
		} else if (rhsType == SingleType) {
			SingleObject* lhs = nullptr;
			const SingleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value *= rhs->Value;
		} else if (rhsType == DoubleType) {
			DoubleObject* lhs = nullptr;
			const DoubleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value *= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIMul() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value = static_cast<std::int32_t>(lhs->Value) * static_cast<std::int32_t>(rhs->Value);
		} else if (rhsType == LongType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value = static_cast<std::int64_t>(lhs->Value) * static_cast<std::int64_t>(rhs->Value);
		} else if (rhsType == SingleType) {
			SingleObject* lhs = nullptr;
			const SingleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value *= rhs->Value;
		} else if (rhsType == DoubleType) {
			DoubleObject* lhs = nullptr;
			const DoubleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value *= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretDiv() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value /= rhs->Value;
		} else if (rhsType == LongType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value /= rhs->Value;
		} else if (rhsType == SingleType) {
			SingleObject* lhs = nullptr;
			const SingleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value /= rhs->Value;
		} else if (rhsType == DoubleType) {
			DoubleObject* lhs = nullptr;
			const DoubleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value /= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIDiv() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value = static_cast<std::int32_t>(lhs->Value) / static_cast<std::int32_t>(rhs->Value);
		} else if (rhsType == LongType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value = static_cast<std::int64_t>(lhs->Value) / static_cast<std::int64_t>(rhs->Value);
		} else if (rhsType == SingleType) {
			SingleObject* lhs = nullptr;
			const SingleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value /= rhs->Value;
		} else if (rhsType == DoubleType) {
			DoubleObject* lhs = nullptr;
			const DoubleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value /= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretMod() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value %= rhs->Value;
		} else if (rhsType == LongType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value %= rhs->Value;
		} else if (rhsType == SingleType) {
			SingleObject* lhs = nullptr;
			const SingleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value = std::fmod(lhs->Value, rhs->Value);
		} else if (rhsType == DoubleType) {
			DoubleObject* lhs = nullptr;
			const DoubleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value = std::fmod(lhs->Value, rhs->Value);
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIMod() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value = static_cast<std::int32_t>(lhs->Value) % static_cast<std::int32_t>(rhs->Value);
		} else if (rhsType == LongType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value = static_cast<std::int64_t>(lhs->Value) % static_cast<std::int64_t>(rhs->Value);
		} else if (rhsType == SingleType) {
			SingleObject* lhs = nullptr;
			const SingleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value = std::fmod(lhs->Value, rhs->Value);
		} else if (rhsType == DoubleType) {
			DoubleObject* lhs = nullptr;
			const DoubleObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;
			else if (rhs->Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}

			lhs->Value = std::fmod(lhs->Value, rhs->Value);
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNeg() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			IntObject& top = reinterpret_cast<IntObject&>(*typePtr);
			top.Value = -static_cast<std::int32_t>(top.Value);
		} else if (type == LongType) {
			LongObject& top = reinterpret_cast<LongObject&>(*typePtr);
			top.Value = -static_cast<std::int64_t>(top.Value);
		} else if (type == SingleType) {
			SingleObject& top = reinterpret_cast<SingleObject&>(*typePtr);
			top.Value = -top.Value;
		} else if (type == DoubleType) {
			DoubleObject& top = reinterpret_cast<DoubleObject&>(*typePtr);
			top.Value = -top.Value;
		} else if (type == PointerType || type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIncDec(int delta) noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (*typePtr != PointerType && *typePtr != GCPointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		Type* targetTypePtr = static_cast<Type*>(reinterpret_cast<const PointerObject*>(typePtr)->Value);
		if (!targetTypePtr) {
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (*typePtr == GCPointerType) {
			targetTypePtr = reinterpret_cast<Type*>(reinterpret_cast<ManagedHeapInfo*>(targetTypePtr) + 1);
		}

		if (targetTypePtr->IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
			return;
		}

		const Type targetType = *targetTypePtr;
		if (targetType == IntType) {
			reinterpret_cast<IntObject*>(targetTypePtr)->Value += delta;
		} else if (targetType == LongType) {
			reinterpret_cast<LongObject*>(targetTypePtr)->Value += delta;
		} else if (targetType == SingleType) {
			reinterpret_cast<SingleObject*>(targetTypePtr)->Value += delta;
		} else if (targetType == DoubleType) {
			reinterpret_cast<DoubleObject*>(targetTypePtr)->Value += delta;
		} else if (targetType == PointerType || targetType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (targetType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}

		m_Stack.Reduce(sizeof(PointerObject));
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAnd() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType || rhsType == SingleType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value &= rhs->Value;
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value &= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretOr() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType || rhsType == SingleType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value |= rhs->Value;
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value |= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretXor() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType || rhsType == SingleType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value ^= rhs->Value;
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value ^= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNot() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType || type == SingleType) {
			IntObject& top = reinterpret_cast<IntObject&>(*typePtr);
			top.Value = ~top.Value;
		} else if (type == LongType || type == DoubleType) {
			LongObject& top = reinterpret_cast<LongObject&>(*typePtr);
			top.Value = ~top.Value;
		} else if (type == PointerType || type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretShl() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType || rhsType == SingleType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value <<= rhs->Value;
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value <<= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSal() noexcept {
		InterpretShl();
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretShr() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType || rhsType == SingleType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value >>= rhs->Value;
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value >>= rhs->Value;
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSar() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType || rhsType == SingleType) {
			IntObject* lhs = nullptr;
			const IntObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value = static_cast<std::int32_t>(lhs->Value) >> static_cast<std::int32_t>(rhs->Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject* lhs = nullptr;
			const LongObject* rhs = nullptr;
			if (!PopTwoSameTypeAndPushOne(rhsTypePtr, lhs, rhs)) return;

			lhs->Value = static_cast<std::int64_t>(lhs->Value) >> static_cast<std::int64_t>(rhs->Value);
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCmp() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == SingleType) {
			SingleObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			PointerObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretICmp() noexcept {
		const Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType<std::int32_t>(lhs.Value, rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType<std::int64_t>(lhs.Value, rhs.Value));
		} else if (rhsType == SingleType) {
			SingleObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == PointerType || rhsType == GCPointerType) {
			PointerObject lhs, rhs;
			if (!PopTwoSameType(rhsTypePtr, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (rhsType.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}