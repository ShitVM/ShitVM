#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

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
	template<typename T>
	void Interpreter::JumpCondition(std::uint32_t operand) {
		if (operand >= m_StackFrame.Instructions->GetLabelCount()) {
			OccurException(SVM_IEC_LABEL_OUTOFRANGE);
			return;
		}

		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
		if (type == IntType) {
			const IntObject& value = *m_Stack.GetTop<IntObject>();
			if (T::Compare(value.Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Pop<IntObject>();
			}
		} else if (type == LongType) {
			const LongObject& value = *m_Stack.GetTop<LongObject>();
			if (T::Compare(value.Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Pop<LongObject>();
			}
		} else if (type == DoubleType) {
			const DoubleObject& value = *m_Stack.GetTop<DoubleObject>();
			if (T::Compare(value.Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Pop<DoubleObject>();
			}
		}
	}
}

namespace {
#define CompareClass(n, o)								\
struct n final {										\
	template<typename T>								\
	static constexpr bool Compare(T value) noexcept	{	\
		return value o;									\
	}													\
};

	CompareClass(EqualZero, == 0);
	CompareClass(NotEqualZero, != 0);
	CompareClass(EqualOne, == 1);
	CompareClass(NotEqualOne, != 0);
	CompareClass(EqualMinusOne, == -1);
	CompareClass(NotEqualMinusOne, != -1);

#undef CompareClass
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
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJmp(std::uint32_t operand) {
		if (operand >= m_StackFrame.Instructions->GetLabelCount()) {
			OccurException(SVM_IEC_LABEL_OUTOFRANGE);
			return;
		}

		m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJe(std::uint32_t operand) {
		JumpCondition<EqualZero>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJne(std::uint32_t operand) {
		JumpCondition<NotEqualZero>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJa(std::uint32_t operand) {
		JumpCondition<EqualOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJae(std::uint32_t operand) {
		JumpCondition<NotEqualMinusOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJb(std::uint32_t operand) {
		JumpCondition<EqualMinusOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJbe(std::uint32_t operand) {
		JumpCondition<NotEqualOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCall(std::uint32_t operand) {
		m_StackFrame.Caller = static_cast<std::size_t>(m_InstructionIndex);
		m_Stack.Push(m_StackFrame);

		m_StackFrame = { m_Stack.GetUsedSize(), m_LocalVariables.size() };
		m_StackFrame.Function = &m_ByteFile.GetFunctions()[operand];
		m_StackFrame.Instructions = &m_StackFrame.Function->GetInstructions();

		std::size_t stackOffset = m_Stack.GetUsedSize() - sizeof(m_StackFrame);
		for (std::uint16_t j = 0; j < m_StackFrame.Function->GetArity(); ++j) {
			const Type** typePtr = m_Stack.Get<const Type*>(stackOffset);
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				m_StackFrame = m_Stack.Pop<StackFrame>().value();
				return;
			}

			m_LocalVariables.push_back(stackOffset);

			const Type* const type = *typePtr;
			if (type == IntType) {
				stackOffset -= sizeof(IntObject);
			} else if (type == LongType) {
				stackOffset -= sizeof(LongObject);
			} else if (type == DoubleType) {
				stackOffset -= sizeof(DoubleObject);
			} else {
				OccurException(SVM_IEC_STACK_EMPTY);
				m_StackFrame = m_Stack.Pop<StackFrame>().value();
				m_LocalVariables.erase(m_LocalVariables.end() - 1);
				return;
			}
		}

		m_InstructionIndex = static_cast<std::uint64_t>(-1);
		++m_Depth;
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretRet() {
		std::variant<std::monostate, IntObject, LongObject, DoubleObject> result;
		if (m_StackFrame.Function->HasResult()) {
			const Type** typePtr = m_Stack.GetTopType();
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type* const type = *typePtr;
			if (type == IntType) {
				result = m_Stack.Pop<IntObject>().value();
			} else if (type == LongType) {
				result = m_Stack.Pop<LongObject>().value();
			} else if (type == DoubleType) {
				result = m_Stack.Pop<DoubleObject>().value();
			} else {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}
		}

		m_LocalVariables.erase(m_LocalVariables.begin() + m_StackFrame.VariableBegin, m_LocalVariables.end());

		const std::uint16_t arity = m_StackFrame.Function->GetArity();
		m_Stack.RemoveTo(m_StackFrame.StackBegin);
		m_StackFrame = m_Stack.Pop<StackFrame>().value();

		m_InstructionIndex = m_StackFrame.Caller;

		for (std::uint16_t j = 0; j < arity; ++j) {
			const Type* const type = *m_Stack.GetTopType();
			if (type == IntType) {
				m_Stack.Pop<IntObject>();
			} else if (type == LongType) {
				m_Stack.Pop<LongObject>();
			} else if (type == DoubleType) {
				m_Stack.Pop<DoubleObject>();
			}
		}

		if (std::holds_alternative<std::monostate>(result)) {
			return;
		} else if (std::holds_alternative<IntObject>(result)) {
			m_Stack.Push(std::get<IntObject>(result));
		} else if (std::holds_alternative<LongObject>(result)) {
			m_Stack.Push(std::get<LongObject>(result));
		} else if (std::holds_alternative<DoubleObject>(result)) {
			m_Stack.Push(std::get<DoubleObject>(result));
		}

		--m_Depth;
	}
}