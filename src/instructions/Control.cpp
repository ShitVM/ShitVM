#include <svm/Interpreter.hpp>

#include <svm/Type.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <cstring>

namespace svm {
	template<typename T>
	SVM_NOINLINE_FOR_PROFILING void Interpreter::JumpCondition(std::uint32_t operand) {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (operand >= m_StackFrame.Instructions->GetLabelCount()) {
			OccurException(SVM_IEC_LABEL_OUTOFRANGE);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			const IntObject* value = reinterpret_cast<IntObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Remove(sizeof(IntObject));
			}
		} else if (type == LongType) {
			const LongObject* value = reinterpret_cast<LongObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Remove(sizeof(LongObject));
			}
		} else if (type == DoubleType) {
			const DoubleObject* value = reinterpret_cast<DoubleObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Remove(sizeof(DoubleObject));
			}
		} else if (type == PointerType) {
			const PointerObject* value = reinterpret_cast<PointerObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_InstructionIndex = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Remove(sizeof(PointerObject));
			}
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace {
#define CompareClass(n, o, v)								\
struct n final {											\
	template<typename T>									\
	static constexpr bool Compare(T value) noexcept	{		\
		return value o v;									\
	}														\
	static constexpr bool Compare(void* value) noexcept {	\
		return value o reinterpret_cast<void*>(v);			\
	}														\
};

	CompareClass(EqualZero, ==, 0);
	CompareClass(NotEqualZero, !=, 0);
	CompareClass(EqualOne, ==, 1);
	CompareClass(NotEqualOne, !=, 1);
	CompareClass(EqualMinusOne, ==, -1);
	CompareClass(NotEqualMinusOne, !=, -1);
#undef CompareClass
}

namespace svm {
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
		if (operand >= m_ByteFile.GetFunctions().size()) {
			OccurException(SVM_IEC_FUNCTION_OUTOFRANGE);
			return;
		}

		m_StackFrame.Caller = static_cast<std::size_t>(m_InstructionIndex);
		if (!m_Stack.Push(m_StackFrame)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		m_StackFrame = { NoneType, m_Stack.GetUsedSize(), m_LocalVariables.size() };
		m_StackFrame.Function = &m_ByteFile.GetFunctions()[operand];
		m_StackFrame.Instructions = &m_StackFrame.Function->GetInstructions();

		std::size_t stackOffset = m_Stack.GetUsedSize() - sizeof(m_StackFrame);
		for (std::uint16_t j = 0; j < m_StackFrame.Function->GetArity(); ++j) {
			Type* const typePtr = m_Stack.Get<Type>(stackOffset);
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				m_StackFrame = *m_Stack.Pop<StackFrame>();
				return;
			}

			m_LocalVariables.push_back(stackOffset);

			const Type type = *typePtr;
			if (type.IsValidType()) {
				stackOffset -= type->Size;
			} else {
				OccurException(SVM_IEC_STACK_EMPTY);
				m_StackFrame = m_Stack.Pop<StackFrame>().value();
				m_LocalVariables.erase(m_LocalVariables.end() - j - 1, m_LocalVariables.end());
				return;
			}
		}

		m_InstructionIndex = static_cast<std::uint64_t>(-1);
		++m_Depth;
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretRet() {
		if (m_Depth == 0) {
			m_InstructionIndex = m_StackFrame.Instructions->GetInstructionCount() - 1;
			return;
		}

		Result result;
		if (m_StackFrame.Function->HasResult()) {
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
				result = m_Stack.Pop<IntObject>()->Value;
			} else if (type == LongType) {
				result = m_Stack.Pop<LongObject>()->Value;
			} else if (type == DoubleType) {
				result = m_Stack.Pop<DoubleObject>()->Value;
			} else if (type == PointerType) {
				result = m_Stack.Pop<PointerObject>()->Value;
			} else if (type.IsStructure()) {
				result = reinterpret_cast<StructureObject*>(typePtr);
				m_Stack.Remove(type->Size);
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
			const Type type = *m_Stack.GetTopType();
			m_Stack.Remove(type->Size);
		}

		--m_Depth;
		if (std::holds_alternative<std::monostate>(result)) {
			return;
		} else if (std::holds_alternative<std::uint32_t>(result)) {
			m_Stack.Push<IntObject>(std::get<std::uint32_t>(result));
		} else if (std::holds_alternative<std::uint64_t>(result)) {
			m_Stack.Push<LongObject>(std::get<std::uint64_t>(result));
		} else if (std::holds_alternative<double>(result)) {
			m_Stack.Push<DoubleObject>(std::get<double>(result));
		} else if (std::holds_alternative<void*>(result)) {
			m_Stack.Push<PointerObject>(std::get<void*>(result));
		} else if (std::holds_alternative<const StructureObject*>(result)) {
			const StructureObject* const structure = std::get<const StructureObject*>(result);
			m_Stack.Add(structure->GetType()->Size);
			std::memmove(m_Stack.GetTopType(), structure, structure->GetType()->Size);
		}
	}
}