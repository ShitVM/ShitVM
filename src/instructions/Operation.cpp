#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

#include <cmath>
#include <type_traits>

namespace svm {
	template<typename T>
	void Interpreter::PopTwoSameType(const Type*& rhsType, T& lhs, T& rhs) noexcept {
		if constexpr (std::is_same_v<IntObject, T>) {
			const Type** const lhsTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(IntObject));
			if (!lhsTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (rhsType != *lhsTypePtr) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			lhs = *reinterpret_cast<IntObject*>(lhsTypePtr);
			rhs = reinterpret_cast<IntObject&>(rhsType);
		} else if constexpr (std::is_same_v<LongObject, T>) {
			const Type** const lhsTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(LongObject));
			if (!lhsTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (rhsType != *lhsTypePtr) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			lhs = *reinterpret_cast<LongObject*>(lhsTypePtr);
			rhs = reinterpret_cast<LongObject&>(rhsType);
		} else if constexpr (std::is_same_v<DoubleObject, T>) {
			const Type** const lhsTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(DoubleObject));
			if (!lhsTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (rhsType != *lhsTypePtr) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			lhs = *reinterpret_cast<DoubleObject*>(lhsTypePtr);
			rhs = reinterpret_cast<DoubleObject&>(rhsType);
		} else if constexpr (std::is_same_v<PointerObject, T>) {
			const Type** const lhsTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(PointerObject));
			if (!lhsTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (rhsType != *lhsTypePtr) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			lhs = *reinterpret_cast<PointerObject*>(lhsTypePtr);
			rhs = reinterpret_cast<PointerObject&>(rhsType);
		}

		m_Stack.Pop<T>();
		m_Stack.Pop<T>();
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAdd() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value + rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value + rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<DoubleObject>(lhs.Value + rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSub() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value - rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value - rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<DoubleObject>(lhs.Value - rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretMul() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value * rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value * rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<DoubleObject>(lhs.Value * rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIMul() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value)* static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value)* static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<DoubleObject>(lhs.Value * rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretDiv() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(lhs.Value / rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(lhs.Value / rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<DoubleObject>(lhs.Value / rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIDiv() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) / static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) / static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<DoubleObject>(lhs.Value / rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretMod() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(lhs.Value % rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(lhs.Value % rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<DoubleObject>(std::fmod(lhs.Value, rhs.Value));
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIMod() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) % static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) % static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<DoubleObject>(std::fmod(lhs.Value, rhs.Value));
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNeg() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& type = *typePtr;
		if (type == IntType) {
			IntObject& top = reinterpret_cast<IntObject&>(type);
			top.Value = -static_cast<std::int32_t>(top.Value);
		} else if (type == LongType) {
			LongObject& top = reinterpret_cast<LongObject&>(type);
			top.Value = -static_cast<std::int64_t>(top.Value);
		} else if (type == DoubleType) {
			DoubleObject& top = reinterpret_cast<DoubleObject&>(type);
			top.Value = -top.Value;
		} else if (type == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIncDec(int delta) {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& type = *typePtr;
		if (type == IntType) {
			reinterpret_cast<IntObject&>(type).Value += delta;
		} else if (type == LongType) {
			reinterpret_cast<LongObject&>(type).Value += delta;
		} else if (type == DoubleType) {
			reinterpret_cast<DoubleObject&>(type).Value += delta;
		} else if (type == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAnd() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value & rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value & rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretOr() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value | rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value | rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretXor() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value ^ rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value ^ rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNot() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& type = *typePtr;
		if (type == IntType) {
			IntObject& top = reinterpret_cast<IntObject&>(type);
			top.Value = ~top.Value;
		} else if (type == LongType || type == DoubleType) {
			LongObject& top = reinterpret_cast<LongObject&>(type);
			top.Value = ~top.Value;
		} else if (type == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretShl() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value << rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value << rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSal() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value << rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value << rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretShr() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(lhs.Value >> rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(lhs.Value >> rhs.Value);
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSar() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) >> static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) >> static_cast<std::int64_t>(rhs.Value));
			if (rhsType == DoubleType) {
				*m_Stack.GetTopType() = DoubleType;
			}
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace svm {
	template<typename T>
	IntObject Interpreter::CompareTwoSameType(T lhs, T rhs) noexcept {
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
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCmp() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == PointerType) {
			PointerObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretICmp() {
		const Type** const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType<std::int32_t>(lhs.Value, rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType<std::int64_t>(lhs.Value, rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == PointerType) {
			PointerObject lhs, rhs;
			PopTwoSameType(rhsType, lhs, rhs);
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}