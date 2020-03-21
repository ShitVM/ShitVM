#include <svm/Interpreter.hpp>

#include <svm/Macro.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <cstring>

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

		Type* targetType = static_cast<Type*>(reinterpret_cast<const PointerObject*>(lhsTypePtr)->Value);
		if (!targetType) {
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (*lhsTypePtr == GCPointerType) {
			targetType = reinterpret_cast<Type*>(reinterpret_cast<ManagedHeapInfo*>(targetType) + 1);
		}

		if (*targetType != *rhsTypePtr) {
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

		Type* targetType = static_cast<Type*>(reinterpret_cast<const PointerObject*>(lhsTypePtr)->Value);
		if (!targetType) {
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (*lhsTypePtr == GCPointerType) {
			targetType = reinterpret_cast<Type*>(reinterpret_cast<ManagedHeapInfo*>(targetType) + 1);
		}

		if (*targetType != *rhsTypePtr) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		CopyStructure(*rhsTypePtr, *targetType);
		m_Stack.Reduce(sizeof(PointerObject) + structSize);
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretFLea(std::uint32_t operand) noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto ptr = m_Stack.Pop<PointerObject>();
		if (!ptr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (ptr->GetType() != PointerType && ptr->GetType() != GCPointerType) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		Type* targetTypePtr = static_cast<Type*>(ptr->Value);
		if (!targetTypePtr) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (ptr->GetType() == GCPointerType) {
			targetTypePtr = reinterpret_cast<Type*>(reinterpret_cast<ManagedHeapInfo*>(targetTypePtr) + 1);
		}

		if (!targetTypePtr->IsStructure()) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_STRUCTURE_NOTSTRUCTURE);
			return;
		}

		const Structure structure =
			m_ByteFile.GetStructures()[static_cast<std::uint32_t>(targetTypePtr->GetReference().Code) - 10];
		if (operand >= structure->Fields.size()) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_STRUCTURE_FIELD_OUTOFRANGE);
			return;
		}

		m_Stack.Push<PointerObject>(reinterpret_cast<std::uint8_t*>(targetTypePtr) + structure->Fields[operand].Offset);
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
		} else if (ptr->GetType() != PointerType && ptr->GetType() != GCPointerType) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		const Type* targetTypePtr = static_cast<Type*>(ptr->Value);
		if (!targetTypePtr) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (ptr->GetType() == GCPointerType) {
			targetTypePtr = reinterpret_cast<const Type*>(reinterpret_cast<const ManagedHeapInfo*>(targetTypePtr) + 1);
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
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNull() noexcept {
		if (!m_Stack.Push<PointerObject>(nullptr)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNew(std::uint32_t operand) {
		const Structures& structures = m_ByteFile.GetStructures();

		if (operand >= structures.GetStructureCount() + 10) {
			OccurException(SVM_IEC_TYPE_OUTOFRANGE);
			return;
		}

		if (m_Stack.GetFreeSize() < sizeof(PointerObject)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		const Type type = GetTypeFromTypeCode(structures, static_cast<TypeCode>(operand));
		void* const address = m_Heap.AllocateUnmanagedHeap(type->Size);

		m_Stack.Push<PointerObject>(address);

		if (!address) return;
		else if (type.IsFundamentalType()) {
			*static_cast<Type*>(address) = type;
		} else if (type.IsStructure()) {
			InitStructure(structures, structures[operand - 10], static_cast<Type*>(address));
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretDelete() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (*typePtr != PointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		void* const address = reinterpret_cast<const PointerObject*>(typePtr)->Value;
		if (address && !m_Heap.DeallocateUnmanagedHeap(reinterpret_cast<const PointerObject*>(typePtr)->Value)) {
			OccurException(SVM_IEC_POINTER_UNKNOWNADDRESS);
			return;
		}

		m_Stack.Reduce(sizeof(PointerObject));
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretGCNull() noexcept {
		if (!m_Stack.Push<GCPointerObject>(nullptr)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretGCNew(std::uint32_t operand) {
		const Structures& structures = m_ByteFile.GetStructures();

		if (operand >= structures.GetStructureCount() + 10) {
			OccurException(SVM_IEC_TYPE_OUTOFRANGE);
			return;
		}

		if (m_Stack.GetFreeSize() < sizeof(PointerObject)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		const Type type = GetTypeFromTypeCode(structures, static_cast<TypeCode>(operand));
		void* const address = m_Heap.AllocateManagedHeap(*this, type->Size);
		Type* const addressReal = reinterpret_cast<Type*>(static_cast<ManagedHeapInfo*>(address) + 1);

		m_Stack.Push<GCPointerObject>(address);

		if (!address) return;
		else if (type.IsFundamentalType()) {
			*addressReal = type;
		} else if (type.IsStructure()) {
			InitStructure(structures, structures[operand - 10], addressReal);
		}
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretANew(std::uint32_t operand) noexcept {
		detail::ArrayInfo info;
		if (!GetArrayInfo(info, operand)) {
			return;
		}

		if (m_Stack.GetFreeSize() < sizeof(PointerObject)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		void* const address = m_Heap.AllocateUnmanagedHeap(info.Size);
		if (address) {
			InitArray(info, static_cast<Type*>(address));
		}

		m_Stack.Push<PointerObject>(address);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAGCNew(std::uint32_t operand) noexcept {
		detail::ArrayInfo info;
		if (!GetArrayInfo(info, operand)) {
			return;
		}

		if (m_Stack.GetFreeSize() < sizeof(PointerObject)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		void* const address = m_Heap.AllocateManagedHeap(*this, info.Size);
		Type* const addressReal = reinterpret_cast<Type*>(static_cast<ManagedHeapInfo*>(address) + 1);
		if (address) {
			InitArray(info, addressReal);
		}

		m_Stack.Push<GCPointerObject>(address);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretALea() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const indexTypePtr = m_Stack.GetTopType();
		if (!indexTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type indexType = *indexTypePtr;
		std::uint64_t index = 0;
		if (indexType == IntType) {
			index = reinterpret_cast<IntObject*>(indexTypePtr)->Value;
		} else if (indexType == LongType) {
			index = reinterpret_cast<LongObject*>(indexTypePtr)->Value;
		} else {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		Type* const pointerTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - indexType->Size);
		if (!pointerTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (*pointerTypePtr != PointerType && *pointerTypePtr != GCPointerType) {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		const PointerObject* const pointer = reinterpret_cast<PointerObject*>(pointerTypePtr);
		Type* targetTypePtr = static_cast<Type*>(pointer->Value);
		if (!targetTypePtr) {
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (pointer->GetType() == GCPointerType) {
			targetTypePtr = reinterpret_cast<Type*>(reinterpret_cast<ManagedHeapInfo*>(targetTypePtr) + 1);
		}

		if (!targetTypePtr->IsArray()) {
			OccurException(SVM_IEC_ARRAY_NOTARRAY);
			return;
		}

		ArrayObject* array = reinterpret_cast<ArrayObject*>(targetTypePtr);
		if (index >= array->Count) {
			OccurException(SVM_IEC_ARRAY_INDEX_OUTOFRANGE);
			return;
		}

		Type* const elementType = reinterpret_cast<Type*>(array + 1);
		m_Stack.Reduce(indexType->Size + sizeof(PointerObject));
		m_Stack.Push<PointerObject>(reinterpret_cast<std::uint8_t*>(elementType) + index * elementType->GetReference().Size);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCount() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto ptr = m_Stack.Pop<PointerObject>();
		if (!ptr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (ptr->GetType() != PointerType && ptr->GetType() != GCPointerType) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return;
		}

		const Type* targetTypePtr = static_cast<Type*>(ptr->Value);
		if (!targetTypePtr) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_POINTER_NULLPOINTER);
			return;
		} else if (ptr->GetType() == GCPointerType) {
			targetTypePtr = reinterpret_cast<const Type*>(reinterpret_cast<const ManagedHeapInfo*>(targetTypePtr) + 1);
		}

		const std::uint64_t count = reinterpret_cast<const ArrayObject*>(targetTypePtr)->Count;
		if (!m_Stack.Push<LongObject>(count)) {
			m_Stack.Expand(sizeof(*ptr));
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
}