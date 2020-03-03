#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

#include <utility>

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretPush(std::uint32_t operand) {
#define ConstantPool m_ByteFile.GetConstantPool()
		if (operand >= ConstantPool.GetAllCount()) {
			OccurException(SVM_IEC_CONSTANTPOOL_OUTOFRANGE);
			return;
		}

		const Type* const constType = ConstantPool.GetConstantType(operand);
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
		if (!m_LocalVariables.empty() && m_Stack.GetUsedSize() == m_LocalVariables.back()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
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

		const Type* const& type = *m_Stack.Get<const Type*>(m_LocalVariables[operand]);
		bool isSuccess = false;
		if (type == IntType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<const IntObject*>(&type));
		} else if (type == LongType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<const LongObject*>(&type));
		} else if (type == DoubleType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<const DoubleObject*>(&type));
		} else if (type == PointerType) {
			isSuccess = m_Stack.Push(*reinterpret_cast<const PointerObject*>(&type));
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretStore(std::uint32_t operand) {
		operand += static_cast<std::uint32_t>(m_StackFrame.VariableBegin);
		if (operand > m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_INVALIDINDEX);
			return;
		} else if (operand == m_LocalVariables.size()) {
			const Type** const typePtr = m_Stack.GetTopType();
			if (!typePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type* const type = *typePtr;
			if (type != IntType && type != LongType && type != DoubleType && type != PointerType) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			m_LocalVariables.push_back(m_Stack.GetUsedSize());
			return;
		}

		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
		const Type*& varType = *m_Stack.Get<const Type*>(m_LocalVariables[operand]);
		if (type != varType) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		if (type == IntType) {
			reinterpret_cast<IntObject&>(varType) = m_Stack.Pop<IntObject>().value();
		} else if (type == LongType) {
			reinterpret_cast<LongObject&>(varType) = m_Stack.Pop<LongObject>().value();
		} else if (type == DoubleType) {
			reinterpret_cast<DoubleObject&>(varType) = m_Stack.Pop<DoubleObject>().value();
		} else if (type == PointerType) {
			reinterpret_cast<PointerObject&>(varType) = m_Stack.Pop<PointerObject>().value();
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

		if (!m_Stack.Push<PointerObject>(m_Stack.Get<const Type*>(m_LocalVariables[operand]))) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretDRef() {
		const auto ptr = m_Stack.Pop<PointerObject>();
		if (!ptr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (ptr->GetType() != PointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		const Type** const varTypePtr = static_cast<const Type**>(ptr->Value);
		if (!varTypePtr) {
			m_Stack.Push<PointerObject>(*ptr);
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		}

		const Type* const varType = *varTypePtr;
		bool isSuccess = false;
		if (varType == IntType) {
			isSuccess = m_Stack.Push<IntObject>(*reinterpret_cast<IntObject*>(varTypePtr));
		} else if (varType == LongType) {
			isSuccess = m_Stack.Push<LongObject>(*reinterpret_cast<LongObject*>(varTypePtr));
		} else if (varType == DoubleType) {
			isSuccess = m_Stack.Push<DoubleObject>(*reinterpret_cast<DoubleObject*>(varTypePtr));
		} else if (varType == PointerType) {
			isSuccess = m_Stack.Push<PointerObject>(*reinterpret_cast<PointerObject*>(varTypePtr));
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCopy() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const& type = *typePtr;
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
		const Type** const firstTypePtr = m_Stack.GetTopType();
		if (!firstTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type*& firstType = *firstTypePtr;
		if (firstType == IntType) {
			const Type** secondTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(IntObject));
			if (!secondTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type*& secondType = *secondTypePtr;
			if (firstType != secondType) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			std::swap(reinterpret_cast<IntObject&>(firstType), reinterpret_cast<IntObject&>(secondType));
		} else if (firstType == LongType) {
			const Type** secondTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(LongObject));
			if (!secondTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type*& secondType = *secondTypePtr;
			if (firstType != secondType) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			std::swap(reinterpret_cast<LongObject&>(firstType), reinterpret_cast<LongObject&>(secondType));
		} else if (firstType == DoubleType) {
			const Type** secondTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(DoubleObject));
			if (!secondTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type*& secondType = *secondTypePtr;
			if (firstType != secondType) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			std::swap(reinterpret_cast<DoubleObject&>(firstType), reinterpret_cast<DoubleObject&>(secondType));
		} else if (firstType == PointerType) {
			const Type** secondTypePtr = m_Stack.Get<const Type*>(m_Stack.GetUsedSize() - sizeof(PointerObject));
			if (!secondTypePtr) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			const Type*& secondType = *secondTypePtr;
			if (firstType != secondType) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			std::swap(reinterpret_cast<PointerObject&>(firstType), reinterpret_cast<PointerObject&>(secondType));
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToI() {
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (!m_LocalVariables.empty() && m_Stack.Get<const Type*>(m_LocalVariables.back()) == typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
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
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (!m_LocalVariables.empty() && m_Stack.Get<const Type*>(m_LocalVariables.back()) == typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
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
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (!m_LocalVariables.empty() && m_Stack.Get<const Type*>(m_LocalVariables.back()) == typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
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
		const Type** const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (!m_LocalVariables.empty() && m_Stack.Get<const Type*>(m_LocalVariables.back()) == typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const type = *typePtr;
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