#include <svm/Interpreter.hpp>

#include <svm/Macro.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <algorithm>
#include <cstring>

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::PushStructure(std::uint32_t code) noexcept {
		const Structures& structures = m_ByteFile.GetStructures();
		if (code >= structures.GetStructureCount()) {
			OccurException(SVM_IEC_CONSTANTPOOL_OUTOFRANGE);
			return;
		}

		const Structure structure = structures[code];
		if (!m_Stack.Expand(structure->Type.Size)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		InitStructure(structures, structure, m_Stack.GetTopType());
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InitStructure(const Structures& structures, Structure structure, Type* type) noexcept {
		const std::size_t fieldCount = structure->FieldTypes.size();

		*type = structure->Type;

		for (std::size_t i = 0; i < fieldCount; ++i) {
			const Type fieldType = structure->FieldTypes[i];
			Type* const pointer = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(type) + structure->FieldOffsets[i]);

			if (fieldType.IsStructure()) {
				InitStructure(structures, structures[static_cast<std::uint32_t>(fieldType->Code) - 10], pointer);
			} else {
				*pointer = fieldType;
			}
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::CopyStructure(const Type& type) noexcept {
		CopyStructure(type, *m_Stack.GetTopType());
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::CopyStructure(const Type& from, Type& to) noexcept {
		std::memcpy(&to, &from, from->Size);
	}
}

namespace svm {
	template<typename T>
	SVM_NOINLINE_FOR_PROFILING void Interpreter::DRefAndAssign(const Type* rhsTypePtr) noexcept {
		if (IsLocalVariable() || IsLocalVariable(sizeof(T))) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const lhsTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - sizeof(T));
		if (!lhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (*lhsTypePtr != PointerType && *lhsTypePtr != GCPointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		Type* const targetType = static_cast<Type*>(reinterpret_cast<const PointerObject*>(lhsTypePtr)->Value);
		if (!targetType) {
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (*targetType != *rhsTypePtr) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		reinterpret_cast<T*>(targetType)->Value = reinterpret_cast<const T*>(rhsTypePtr)->Value;
		m_Stack.Reduce(sizeof(PointerObject) + sizeof(T));
	}
	template<>
	SVM_NOINLINE_FOR_PROFILING void Interpreter::DRefAndAssign<StructureObject>(const Type* rhsTypePtr) noexcept {
		const std::size_t structSize = rhsTypePtr->GetReference().Size;

		if (IsLocalVariable() || IsLocalVariable(structSize)) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const lhsTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - structSize);
		if (!lhsTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (*lhsTypePtr != PointerType && *lhsTypePtr != GCPointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		Type* const targetType = static_cast<Type*>(reinterpret_cast<const PointerObject*>(lhsTypePtr)->Value);
		if (!targetType) {
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (*targetType != *rhsTypePtr) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		CopyStructure(*rhsTypePtr, *targetType);
		m_Stack.Reduce(sizeof(PointerObject) + structSize);
	}
}

namespace svm {
	template<typename T>
	SVM_NOINLINE_FOR_PROFILING bool Interpreter::GetTwoSameType(Type rhsType, T*& lhs) noexcept {
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
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretPush(std::uint32_t operand) noexcept {
		const ConstantPool& constantPool = m_ByteFile.GetConstantPool();
		const std::uint32_t constCount = constantPool.GetAllCount();

		if (operand >= constCount) {
			PushStructure(operand - constCount);
			return;
		}

		const Type constType = constantPool.GetConstantType(operand);
		bool isSuccess = false;
		if (constType == IntType) {
			isSuccess = m_Stack.Push(constantPool.GetConstant<IntObject>(operand));
		} else if (constType == LongType) {
			isSuccess = m_Stack.Push(constantPool.GetConstant<LongObject>(operand));
		} else if (constType == DoubleType) {
			isSuccess = m_Stack.Push(constantPool.GetConstant<DoubleObject>(operand));
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretPop() noexcept {
		if (IsLocalVariable()) {
			m_LocalVariables.erase(m_LocalVariables.end() - 1);
		}

		const Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		if (type.IsValidType()) {
			m_Stack.Reduce(type->Size);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretLoad(std::uint32_t operand) noexcept {
		operand += m_StackFrame.VariableBegin;
		if (operand >= m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_OUTOFRANGE);
			return;
		}

		const Type* const typePtr = m_Stack.Get<Type>(m_LocalVariables[operand]);
		const Type type = *typePtr;
		bool isSuccess = false;
		if (type == IntType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const IntObject&>(*typePtr));
		} else if (type == LongType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const LongObject&>(*typePtr));
		} else if (type == DoubleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const DoubleObject&>(*typePtr));
		} else if (type == PointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const PointerObject&>(*typePtr));
		} else if (type == GCPointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const GCPointerObject&>(*typePtr));
		} else if (type.IsStructure() && (isSuccess = m_Stack.Expand(type->Size))) {
			CopyStructure(*typePtr);
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

		operand += m_StackFrame.VariableBegin;
		if (operand > m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_INVALIDINDEX);
			return;
		} else if (operand == m_LocalVariables.size()) {
			const Type* const typePtr = m_Stack.GetTopType();
			if (!typePtr || !typePtr->IsValidType()) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			}

			m_LocalVariables.push_back(m_Stack.GetUsedSize());
			return;
		}

		const Type* const typePtr = m_Stack.GetTopType();
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
			reinterpret_cast<IntObject&>(varType) = reinterpret_cast<const IntObject&>(*typePtr);
		} else if (type == LongType) {
			reinterpret_cast<LongObject&>(varType) = reinterpret_cast<const LongObject&>(*typePtr);
		} else if (type == DoubleType) {
			reinterpret_cast<DoubleObject&>(varType) = reinterpret_cast<const DoubleObject&>(*typePtr);
		} else if (type == PointerType) {
			reinterpret_cast<PointerObject&>(varType) = reinterpret_cast<const PointerObject&>(*typePtr);
		} else if (type == GCPointerType) {
			reinterpret_cast<GCPointerObject&>(varType) = reinterpret_cast<const GCPointerObject&>(*typePtr);
		} else if (type.IsStructure()) {
			CopyStructure(*typePtr, varType);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		m_Stack.Reduce(type->Size);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretLea(std::uint32_t operand) noexcept {
		operand += m_StackFrame.VariableBegin;
		if (operand >= m_LocalVariables.size()) {
			OccurException(SVM_IEC_LOCALVARIABLE_OUTOFRANGE);
			return;
		}

		if (!m_Stack.Push<PointerObject>(m_Stack.Get<Type>(m_LocalVariables[operand]))) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretFLea(std::uint32_t operand) noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto ptr = m_Stack.Pop<PointerObject>();
		if (!ptr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (ptr->GetType() != PointerType) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		Type* const targetTypePtr = static_cast<Type*>(ptr->Value);
		if (!targetTypePtr) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (!targetTypePtr->IsStructure()) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_STRUCTURE_NOTSTRUCTURE);
			return;
		}

		const Structure structure =
			m_ByteFile.GetStructures()[static_cast<std::uint32_t>(targetTypePtr->GetReference().Code) - 10];
		if (operand >= structure->FieldTypes.size()) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_STRUCTURE_FIELD_OUTOFRANGE);
			return;
		}

		m_Stack.Push<PointerObject>(reinterpret_cast<std::uint8_t*>(targetTypePtr) + structure->FieldOffsets[operand]);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretTLoad() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto ptr = m_Stack.Pop<PointerObject>();
		if (!ptr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (ptr->GetType() != PointerType || ptr->GetType() != GCPointerType) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		const Type* const targetTypePtr = static_cast<Type*>(ptr->Value);
		if (!targetTypePtr) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		}

		const Type targetType = *targetTypePtr;
		bool isSuccess = false;
		if (targetType == IntType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const IntObject&>(*targetTypePtr));
		} else if (targetType == LongType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const LongObject&>(*targetTypePtr));
		} else if (targetType == DoubleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const DoubleObject&>(*targetTypePtr));
		} else if (targetType == PointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const PointerObject&>(*targetTypePtr));
		} else if (targetType == GCPointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const GCPointerObject&>(*targetTypePtr));
		} else if (targetType.IsStructure() && (isSuccess = m_Stack.Expand(targetType->Size))) {
			CopyStructure(*targetTypePtr);
		}

		if (!isSuccess) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretTStore() noexcept {
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
		} else if (rhsType == GCPointerType) {
			DRefAndAssign<GCPointerObject>(rhsTypePtr);
		} else if (rhsType.IsStructure()) {
			DRefAndAssign<StructureObject>(rhsTypePtr);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCopy() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type type = *typePtr;
		bool isSuccess = false;
		if (type == IntType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const IntObject&>(*typePtr));
		} else if (type == LongType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const LongObject&>(*typePtr));
		} else if (type == DoubleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const DoubleObject&>(*typePtr));
		} else if (type == PointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const PointerObject&>(*typePtr));
		} else if (type == GCPointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const GCPointerObject&>(*typePtr));
		} else if (type.IsStructure()) {
			if (isSuccess = m_Stack.Expand(type->Size)) {
				CopyStructure(type);
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}

		if (!isSuccess) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretSwap() noexcept {
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
		} else if (firstType == GCPointerType) {
			GCPointerObject* second = nullptr;
			if (!GetTwoSameType(firstType, second)) return;
			std::iter_swap(reinterpret_cast<GCPointerObject*>(firstTypePtr), second);
		} else if (firstType.IsStructure()) {
			if (IsLocalVariable() || IsLocalVariable(firstType->Size)) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (!m_Stack.Get<Type>(m_Stack.GetUsedSize() - firstType->Size)->IsStructure()) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			for (std::size_t i = sizeof(Type); i < firstType->Size; i += sizeof(void*)) {
				std::iter_swap(m_Stack.Get<void*>(m_Stack.GetUsedSize() - i), m_Stack.Get<void*>(m_Stack.GetUsedSize() - firstType->Size - i));
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}