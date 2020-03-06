#include <svm/Interpreter.hpp>

#include <svm/Type.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <algorithm>

namespace svm {
	template<typename T>
	void Interpreter::DRefAndAssign(Type* rhsTypePtr) noexcept {
		if (IsLocalVariable() || IsLocalVariable(sizeof(T))) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const lhsTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - sizeof(T));
		if (!lhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (*lhsTypePtr != PointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		Type* const targetType = static_cast<Type*>(reinterpret_cast<PointerObject*>(lhsTypePtr)->Value);
		if (*targetType != *rhsTypePtr) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		reinterpret_cast<T*>(targetType)->Value = reinterpret_cast<T*>(rhsTypePtr)->Value;
		m_Stack.Pop<T>();
		m_Stack.Pop<PointerObject>();
	}
	template<typename T>
	bool Interpreter::GetTwoSameType(Type rhsType, T*& lhs) noexcept {
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

		lhs = reinterpret_cast<T*>(lhsTypePtr);
		return true;
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretPush(std::uint32_t operand) {
#define ConstantPool m_ByteFile.GetConstantPool()
		if (operand >= ConstantPool.GetAllCount()) {
			OccurException(SVM_IEC_CONSTANTPOOL_OUTOFRANGE);
			return;
		}

		const Type constType = ConstantPool.GetConstantType(operand);
		bool isSuccess = false;
		if (constType == IntType) {
			isSuccess = m_Stack.Push(ConstantPool.GetConstant<IntObject>(operand));
		} else if (constType == LongType) {
			isSuccess = m_Stack.Push(ConstantPool.GetConstant<LongObject>(operand));
		} else if (constType == DoubleType) {
			isSuccess = m_Stack.Push(ConstantPool.GetConstant<DoubleObject>(operand));
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
#undef ConstantPool
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretPop() {
		if (IsLocalVariable()) {
			m_LocalVariables.erase(m_LocalVariables.end() - 1);
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			m_Stack.Pop<IntObject>();
		} else if (type == LongType) {
			m_Stack.Pop<LongObject>();
		} else if (type == DoubleType) {
			m_Stack.Pop<DoubleObject>();
		} else if (type == PointerType) {
			m_Stack.Pop<PointerObject>();
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretLoad(std::uint32_t operand) {
		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);
		if (operand >= m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_OUTOFRANGE);
			return;
		}

		const Type& type = *m_Stack.Get<Type>(m_LocalVariables[operand]);
		bool isSuccess = false;
		if (type == IntType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const IntObject&>(type));
		} else if (type == LongType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const LongObject&>(type));
		} else if (type == DoubleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const DoubleObject&>(type));
		} else if (type == PointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const PointerObject&>(type));
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretStore(std::uint32_t operand) {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);
		if (operand > m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_INVALIDINDEX);
			return;
		} else if (operand == m_LocalVariables.size()) {
			Type* const typePtr = m_Stack.GetTopType();
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (!typePtr->IsFundamentalType()) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			m_LocalVariables.push_back(m_Stack.GetUsedSize());
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		Type& varType = *m_Stack.Get<Type>(m_LocalVariables[operand]);
		if (type != varType) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		if (type == IntType) {
			reinterpret_cast<IntObject&>(varType) = *m_Stack.Pop<IntObject>();
		} else if (type == LongType) {
			reinterpret_cast<LongObject&>(varType) = *m_Stack.Pop<LongObject>();
		} else if (type == DoubleType) {
			reinterpret_cast<DoubleObject&>(varType) = *m_Stack.Pop<DoubleObject>();
		} else if (type == PointerType) {
			reinterpret_cast<PointerObject&>(varType) = *m_Stack.Pop<PointerObject>();
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretLea(std::uint32_t operand) {
		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);
		if (operand >= m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_OUTOFRANGE);
			return;
		}

		if (!m_Stack.Push<PointerObject>(m_Stack.Get<Type>(m_LocalVariables[operand]))) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretTLoad() {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto ptr = m_Stack.Pop<PointerObject>();
		if (!ptr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (ptr->GetType() != PointerType) {
			m_Stack.Push(*ptr);
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		Type* const varTypePtr = static_cast<Type*>(ptr->Value);
		if (!varTypePtr) {
			m_Stack.Push(*ptr);
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		}

		const Type varType = *varTypePtr;
		bool isSuccess = false;
		if (varType == IntType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<IntObject*>(varTypePtr));
		} else if (varType == LongType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<LongObject*>(varTypePtr));
		} else if (varType == DoubleType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<DoubleObject*>(varTypePtr));
		} else if (varType == PointerType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<PointerObject*>(varTypePtr));
		}

		if (!isSuccess) {
			m_Stack.Push(*ptr);
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretTStore() {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type rhsType = *rhsTypePtr;
		if (rhsType == IntType) {
			DRefAndAssign<IntObject>(rhsTypePtr);
		} else if (rhsType == LongType) {
			DRefAndAssign<LongObject>(rhsTypePtr);
		} else if (rhsType == DoubleType) {
			DRefAndAssign<DoubleObject>(rhsTypePtr);
		} else if (rhsType == PointerType) {
			DRefAndAssign<PointerObject>(rhsTypePtr);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCopy() {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type& type = *typePtr;
		bool isSuccess = false;
		if (type == IntType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const IntObject&>(type));
		} else if (type == LongType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const LongObject&>(type));
		} else if (type == DoubleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const DoubleObject&>(type));
		} else if (type == PointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const PointerObject&>(type));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSwap() {
		Type* const firstTypePtr = m_Stack.GetTopType();
		if (!firstTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type firstType = *firstTypePtr;
		if (firstType == IntType) {
			IntObject* second = nullptr;
			if (!GetTwoSameType(firstType, second)) return;
			std::iter_swap(reinterpret_cast<IntObject*>(firstTypePtr), second);
		} else if (firstType == LongType) {
			LongObject* second = nullptr;
			if (!GetTwoSameType(firstType, second)) return;
			std::iter_swap(reinterpret_cast<LongObject*>(firstTypePtr), second);
		} else if (firstType == DoubleType) {
			DoubleObject* second = nullptr;
			if (!GetTwoSameType(firstType, second)) return;
			std::iter_swap(reinterpret_cast<DoubleObject*>(firstTypePtr), second);
		} else if (firstType == PointerType) {
			PointerObject* second = nullptr;
			if (!GetTwoSameType(firstType, second)) return;
			std::iter_swap(reinterpret_cast<PointerObject*>(firstTypePtr), second);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToI() {
		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (!m_LocalVariables.empty() && m_Stack.Get<Type>(m_LocalVariables.back()) == typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			return;
		} else if (type == LongType) {
			const LongObject value = *m_Stack.Pop<LongObject>();
			m_Stack.Push<IntObject>(static_cast<std::uint32_t>(value.Value));
		} else if (type == DoubleType) {
			const DoubleObject value = *m_Stack.Pop<DoubleObject>();
			m_Stack.Push<IntObject>(static_cast<std::uint32_t>(value.Value));
		} else if (type == PointerType) {
			const PointerObject value = *m_Stack.Pop<PointerObject>();
			m_Stack.Push<IntObject>(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value.Value)));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToL() {
		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (!m_LocalVariables.empty() && m_Stack.Get<Type>(m_LocalVariables.back()) == typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			const IntObject value = *m_Stack.Pop<IntObject>();
			m_Stack.Push<LongObject>(value.Value);
		} else if (type == LongType) {
			return;
		} else if (type == DoubleType) {
			const DoubleObject value = *m_Stack.Pop<DoubleObject>();
			m_Stack.Push<LongObject>(static_cast<std::uint64_t>(value.Value));
		} else if (type == PointerType) {
			const PointerObject value = *m_Stack.Pop<PointerObject>();
			m_Stack.Push<LongObject>(static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(value.Value)));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToD() {
		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (!m_LocalVariables.empty() && m_Stack.Get<Type>(m_LocalVariables.back()) == typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			const IntObject value = *m_Stack.Pop<IntObject>();
			m_Stack.Push<DoubleObject>(value.Value);
		} else if (type == LongType) {
			const LongObject value = *m_Stack.Pop<LongObject>();
			m_Stack.Push<DoubleObject>(static_cast<double>(value.Value));
		} else if (type == DoubleType) {
			return;
		} else if (type == PointerType) {
			const PointerObject value = *m_Stack.Pop<PointerObject>();
			m_Stack.Push<DoubleObject>(static_cast<double>(reinterpret_cast<std::uintptr_t>(value.Value)));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToP() {
		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (!m_LocalVariables.empty() && m_Stack.Get<Type>(m_LocalVariables.back()) == typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			const IntObject value = *m_Stack.Pop<IntObject>();
			m_Stack.Push<PointerObject>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(value.Value)));
		} else if (type == LongType) {
			const LongObject value = *m_Stack.Pop<LongObject>();
			m_Stack.Push<PointerObject>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(value.Value)));
		} else if (type == DoubleType) {
			const DoubleObject value = *m_Stack.Pop<DoubleObject>();
			m_Stack.Push<PointerObject>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(value.Value)));
		} else if (type == PointerType) {
			return;
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}