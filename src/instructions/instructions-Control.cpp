#include <svm/Interpreter.hpp>

#include <svm/Macro.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>
#include <svm/virtual/VirtualStack.hpp>

#include <cstring>

namespace svm {
	template<typename T>
	SVM_NOINLINE_FOR_PROFILING void Interpreter::JumpCondition(std::uint32_t operand) noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (operand >= m_StackFrame.Instructions->GetLabelCount()) {
			OccurException(SVM_IEC_LABEL_OUTOFRANGE);
			return;
		}

		const Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			const IntObject* value = reinterpret_cast<const IntObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_StackFrame.Caller = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Reduce(sizeof(IntObject));
			}
		} else if (type == LongType) {
			const LongObject* value = reinterpret_cast<const LongObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_StackFrame.Caller = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Reduce(sizeof(LongObject));
			}
		} else if (type == DoubleType) {
			const DoubleObject* value = reinterpret_cast<const DoubleObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_StackFrame.Caller = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Reduce(sizeof(DoubleObject));
			}
		} else if (type == PointerType) {
			const PointerObject* value = reinterpret_cast<const PointerObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_StackFrame.Caller = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Reduce(sizeof(PointerObject));
			}
		} else if (type == GCPointerType) {
			const GCPointerObject* value = reinterpret_cast<const GCPointerObject*>(typePtr);
			if (T::Compare(value->Value)) {
				m_StackFrame.Caller = m_StackFrame.Instructions->GetLabel(operand) - 1;
				m_Stack.Reduce(sizeof(GCPointerObject));
			}
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace {
#define CompareClass(n, o, v)								\
struct n final {											\
	template<typename T>									\
	static bool Compare(T value) noexcept	{				\
		return value o v;									\
	}														\
	static bool Compare(void* value) noexcept {				\
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
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJmp(std::uint32_t operand) noexcept {
		if (operand >= m_StackFrame.Instructions->GetLabelCount()) {
			OccurException(SVM_IEC_LABEL_OUTOFRANGE);
			return;
		}

		m_StackFrame.Caller = m_StackFrame.Instructions->GetLabel(operand) - 1;
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJe(std::uint32_t operand) noexcept {
		JumpCondition<EqualZero>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJne(std::uint32_t operand) noexcept {
		JumpCondition<NotEqualZero>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJa(std::uint32_t operand) noexcept {
		JumpCondition<EqualOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJae(std::uint32_t operand) noexcept {
		JumpCondition<NotEqualMinusOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJb(std::uint32_t operand) noexcept {
		JumpCondition<EqualMinusOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretJbe(std::uint32_t operand) noexcept {
		JumpCondition<NotEqualOne>(operand);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCall(std::uint32_t operand) {
		if (operand >= GetFunctionCount()) {
			OccurException(SVM_IEC_FUNCTION_OUTOFRANGE);
			return;
		} else if (!m_Stack.Push(m_StackFrame)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		m_StackFrame = { NoneType, m_Stack.GetUsedSize(), static_cast<std::uint32_t>(m_LocalVariables.size()) };
		std::uint16_t arity = 0;

		m_StackFrame.Function = GetFunction(operand);
		if (std::holds_alternative<const Function*>(m_StackFrame.Function)) {
			const Function* const function = std::get<const Function*>(m_StackFrame.Function);

			m_StackFrame.Instructions = &function->GetInstructions();
			m_StackFrame.Caller = static_cast<std::uint64_t>(-1);

			arity = function->GetArity();
		} else {
			const VirtualFunction* const function = std::get<const VirtualFunction*>(m_StackFrame.Function);

			arity = function->GetArity();
		}

		std::size_t stackOffset = m_Stack.GetUsedSize() - sizeof(m_StackFrame);
		for (std::uint16_t j = 0; j < arity; ++j) {
			const Type* const typePtr = m_Stack.Get<Type>(stackOffset);
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				m_StackFrame = *m_Stack.Pop<StackFrame>();
				return;
			}

			m_LocalVariables.push_back(stackOffset);

			const Type type = *typePtr;
			if (type.IsArray()) {
				stackOffset -= CalcArraySize(reinterpret_cast<const ArrayObject*>(typePtr));
			} else if (type.IsValidType()) {
				stackOffset -= type->Size;
			} else {
				OccurException(SVM_IEC_STACK_EMPTY);
				m_StackFrame = *m_Stack.Pop<StackFrame>();
				m_LocalVariables.erase(m_LocalVariables.end() - j - 1, m_LocalVariables.end());
				return;
			}
		}

		++m_Depth;

		if (std::holds_alternative<const VirtualFunction*>(m_StackFrame.Function)) {
			const VirtualFunction* const function = std::get<const VirtualFunction*>(m_StackFrame.Function);

			(*function)({ &m_Stack, &m_StackFrame, &m_LocalVariables });
			InterpretRet();
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretRet() noexcept {
		if (m_Depth == 0) {
			m_StackFrame.Caller = m_StackFrame.Instructions->GetInstructionCount() - 1;
			return;
		}

		std::uint16_t arity = 0;
		bool hasResult = false;
		if (std::holds_alternative<const Function*>(m_StackFrame.Function)) {
			const Function* const function = std::get<const Function*>(m_StackFrame.Function);

			arity = function->GetArity();
			hasResult = function->HasResult();
		} else {
			const VirtualFunction* const function = std::get<const VirtualFunction*>(m_StackFrame.Function);

			arity = function->GetArity();
			hasResult = function->HasResult();
		}

		const Type* result = nullptr;
		if (hasResult) {
			if (IsLocalVariable()) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			result = m_Stack.GetTopType();
			if (!result) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type type = *result;
			if (!type.IsValidType()) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			if (type.IsArray()) {
				m_Stack.Reduce(CalcArraySize(reinterpret_cast<const ArrayObject*>(result)));
			} else {
				m_Stack.Reduce(type->Size);
			}
		}

		m_LocalVariables.erase(m_LocalVariables.begin() + m_StackFrame.VariableBegin, m_LocalVariables.end());

		m_Stack.SetUsedSize(m_StackFrame.StackBegin);
		m_StackFrame = *m_Stack.Pop<StackFrame>();

		for (std::uint16_t j = 0; j < arity; ++j) {
			const Type type = *m_Stack.GetTopType();
			m_Stack.Reduce(type->Size);
		}

		--m_Depth;
		if (result) {
			std::size_t size = 0;
			if (result->IsArray()) {
				size = CalcArraySize(reinterpret_cast<const ArrayObject*>(result));
			} else {
				size = result->GetReference().Size;
			}

			m_Stack.Expand(size);
			std::memmove(m_Stack.GetTopType(), result, size);
		}
	}
}