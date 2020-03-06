#include <svm/Interpreter.hpp>

#include <svm/Type.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <cmath>
#include <type_traits>

namespace svm {
	template<typename T>
	bool Interpreter::PopTwoSameType(Type& rhsType, T& lhs, T& rhs) noexcept {
		if (IsLocalVariable() || IsLocalVariable(sizeof(T))) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return false;
		}

		Type* const lhsTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - sizeof(T));
		if (!lhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return false;
		} else if (rhsType != *lhsTypePtr) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return false;
		}

		lhs = *reinterpret_cast<T*>(lhsTypePtr);
		rhs = reinterpret_cast<T&>(rhsType);

		m_Stack.Pop<T>();
		m_Stack.Pop<T>();
		return true;
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAdd() {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(lhs.Value + rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<LongObject>(lhs.Value + rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<DoubleObject>(lhs.Value + rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSub() {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(lhs.Value - rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<LongObject>(lhs.Value - rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<DoubleObject>(lhs.Value - rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretMul() {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(lhs.Value * rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<LongObject>(lhs.Value * rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<DoubleObject>(lhs.Value * rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIMul() {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value)* static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value)* static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<DoubleObject>(lhs.Value * rhs.Value);
		} else if (rhsType == PointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretDiv() {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(lhs.Value / rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(lhs.Value / rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) / static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) / static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(lhs.Value % rhs.Value);
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(lhs.Value % rhs.Value);
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) % static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			if (rhs.Value == 0) {
				OccurException(SVM_IEC_ARITHMETIC_DIVIDEBYZERO);
				return;
			}
			m_Stack.Push<LongObject>(static_cast<std::int64_t>(lhs.Value) % static_cast<std::int64_t>(rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& type = *typePtr;
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
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretIncDec(std::uint32_t operand, int delta) {
		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);
		if (operand >= m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_OUTOFRANGE);
			return;
		}

		Type& type = *m_Stack.Get<Type>(m_LocalVariables[operand]);
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(lhs.Value & rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(lhs.Value | rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(lhs.Value ^ rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& type = *typePtr;
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(lhs.Value << rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		InterpretShl();
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretShr() {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(lhs.Value >> rhs.Value);
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push<IntObject>(static_cast<std::int32_t>(lhs.Value) >> static_cast<std::int32_t>(rhs.Value));
		} else if (rhsType == LongType || rhsType == DoubleType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
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
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == PointerType) {
			PointerObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretICmp() {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type& rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			IntObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType<std::int32_t>(lhs.Value, rhs.Value));
		} else if (rhsType == LongType) {
			LongObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType<std::int64_t>(lhs.Value, rhs.Value));
		} else if (rhsType == DoubleType) {
			DoubleObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else if (rhsType == PointerType) {
			PointerObject lhs, rhs;
			if (!PopTwoSameType(rhsType, lhs, rhs)) return;
			m_Stack.Push(CompareTwoSameType(lhs.Value, rhs.Value));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}