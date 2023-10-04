#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

#include <algorithm>
#include <cstring>

namespace svm {
	std::optional<RawPointerObject> Interpreter::GetRawPointerObject(Type* typePtr) noexcept {
		if (*typePtr == PointerType) {
			const PointerObject& pointer = *reinterpret_cast<const PointerObject*>(typePtr);
			if (!pointer.RawObject.Address) {
				OccurException(SVM_IEC_POINTER_NULLPOINTER);
				return std::nullopt;
			}

			return pointer.RawObject;
		} else if (*typePtr == GCPointerType) {
			const GCPointerObject& pointer = *reinterpret_cast<const GCPointerObject*>(typePtr);
			if (!pointer.RawObject.Address) {
				OccurException(SVM_IEC_POINTER_NULLPOINTER);
				return std::nullopt;
			}

			Type* const targetTypePtr = reinterpret_cast<Type*>(
				static_cast<ManagedHeapInfo*>(pointer.RawObject.Address) + 1);
			if (*targetTypePtr == ArrayType) {
				ArrayObject* const arrayPtr = reinterpret_cast<ArrayObject*>(targetTypePtr);
				return RawPointerObject{ arrayPtr->GetType(), arrayPtr->Count, arrayPtr + 1 };
			} else {
				return RawPointerObject{ *targetTypePtr, 0, reinterpret_cast<Object*>(targetTypePtr) + 1 };
			}
		} else {
			OccurException(SVM_IEC_POINTER_NOTPOINTER);
			return std::nullopt;
		}
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretFLea(std::uint32_t operand) noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto rawPointer = GetRawPointerObject(typePtr);
		if (!rawPointer) return;
		else if (rawPointer->Count || !rawPointer->Type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_NOTSTRUCTURE);
			return;
		}

		const Structure structure = GetStructure(rawPointer->Type);
		if (operand >= structure->Fields.size()) {
			OccurException(SVM_IEC_STRUCTURE_FIELD_OUTOFRANGE);
			return;
		}

		m_Stack.Reduce((*typePtr)->Size);

		const Field& field = structure->Fields[operand];
		if (!m_Stack.Push<PointerObject>(RawPointerObject{
			field.Type, static_cast<std::size_t>(field.Count),
			static_cast<std::uint8_t*>(rawPointer->Address) + field.Offset })) {
			m_Stack.Expand((*typePtr)->Size);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretTLoad() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto rawPointer = GetRawPointerObject(typePtr);
		if (!rawPointer) return;

		if (rawPointer->Count) {
			const std::size_t size = rawPointer->Type->RawSize * rawPointer->Count;
			if (m_Stack.Expand(size - (*typePtr)->Size)) {
				ArrayObject* const arrayPtr = m_Stack.GetTop<ArrayObject>();
				arrayPtr->SetType(ArrayType);
				arrayPtr->ElementType = rawPointer->Type;
				arrayPtr->Count = rawPointer->Count;

				std::memcpy(arrayPtr + 1, rawPointer->Address, size);
			} else {
				OccurException(SVM_IEC_STACK_OVERFLOW);
			}
		} else {
			if (m_Stack.Expand(rawPointer->Type->Size - (*typePtr)->Size)) {
				Type* const destTypePtr = m_Stack.GetTopType();
				*destTypePtr = rawPointer->Type;
				std::memcpy(reinterpret_cast<Object*>(destTypePtr) + 1, rawPointer->Address, rawPointer->Type->RawSize);
			} else {
				OccurException(SVM_IEC_STACK_OVERFLOW);
			}
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretTStore() noexcept {
		Type* const rhsTypePtr = m_Stack.GetTopType();
		if (!rhsTypePtr || IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const lhsTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - (*rhsTypePtr)->Size);
		if (!lhsTypePtr || IsLocalVariable((*rhsTypePtr)->Size)) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto rawPointer = GetRawPointerObject(lhsTypePtr);
		if (!rawPointer) return;
		
		if (rawPointer->Count) {
			if (!rhsTypePtr->IsArray()) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			const ArrayObject* const array = reinterpret_cast<ArrayObject*>(rhsTypePtr);
			if (array->ElementType != rawPointer->Type) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			} else if (array->Count != rawPointer->Count) {
				OccurException(SVM_IEC_ARRAY_COUNT_DIFFERENTCOUNT);
				return;
			}
		} else if (rhsTypePtr->IsArray() || rawPointer->Type != *rhsTypePtr) {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		std::memcpy(rawPointer->Address, reinterpret_cast<Object*>(rhsTypePtr) + 1,
			rawPointer->Type->RawSize * std::max(static_cast<std::size_t>(1), rawPointer->Count));
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNull() noexcept {
		if (!m_Stack.Push<PointerObject>({})) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretNew(std::uint32_t operand) {
		if (operand >= GetStructureCount() + static_cast<std::uint32_t>(TypeCode::Structure)) {
			OccurException(SVM_IEC_TYPE_OUTOFRANGE);
			return;
		}

		if (m_Stack.GetFreeSize() < sizeof(PointerObject)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		const Type type = GetType(static_cast<TypeCode>(operand));
		void* const address = m_Heap.AllocateUnmanagedHeap(type->Size);

		m_Stack.Push<PointerObject>(RawPointerObject{ type, 0, address });

		if (!address) return;
		else if (type.IsStructure()) {
			InitStructure(GetStructure(type), address);
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

		void* const address = reinterpret_cast<const PointerObject*>(typePtr)->RawObject.Address;
		if (address && !m_Heap.DeallocateUnmanagedHeap(address)) {
			OccurException(SVM_IEC_POINTER_UNKNOWNADDRESS);
			return;
		}

		m_Stack.Reduce(sizeof(PointerObject));
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretGCNull() noexcept {
		if (!m_Stack.Push<GCPointerObject>({})) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretGCNew(std::uint32_t operand) {
		if (operand >= GetStructureCount() + static_cast<std::uint32_t>(TypeCode::Structure)) {
			OccurException(SVM_IEC_TYPE_OUTOFRANGE);
			return;
		}

		if (m_Stack.GetFreeSize() < sizeof(PointerObject)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		const Type type = GetType(static_cast<TypeCode>(operand));
		void* const address = m_Heap.AllocateManagedHeap(*this, type->Size);
		Type* const addressReal = reinterpret_cast<Type*>(static_cast<ManagedHeapInfo*>(address) + 1);

		m_Stack.Push<GCPointerObject>(RawGCPointerObject{ address });

		if (!address) return;
		else if (type.IsFundamentalType()) {
			*addressReal = type;
		} else if (type.IsStructure()) {
			InitStructure(GetStructure(type), addressReal);
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

		const std::size_t size = info.ElementType->Size * info.Count;
		void* const address = m_Heap.AllocateUnmanagedHeap(size);
		if (address) {
			InitArray(info, address);
		}

		m_Stack.Reduce(info.CountSize);
		m_Stack.Push<PointerObject>(RawPointerObject{ info.ElementType, static_cast<std::size_t>(info.CountSize), address });
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

		const std::size_t size = CalcArraySize(info.ElementType, info.Count);
		void* const address = m_Heap.AllocateManagedHeap(*this, size);
		Type* const addressReal = reinterpret_cast<Type*>(static_cast<ManagedHeapInfo*>(address) + 1);
		if (address) {
			InitArray(info, addressReal);
		}

		m_Stack.Reduce(info.CountSize);
		m_Stack.Push<GCPointerObject>(RawGCPointerObject{ address });
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretALea() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (m_Stack.GetFreeSize() < sizeof(PointerObject)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
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
			index = reinterpret_cast<IntObject*>(indexTypePtr)->RawObject.Value;
		} else if (indexType == LongType) {
			index = reinterpret_cast<LongObject*>(indexTypePtr)->RawObject.Value;
		} else {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return;
		}

		Type* const pointerTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - indexType->Size);
		if (!pointerTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto rawPointer = GetRawPointerObject(pointerTypePtr);
		if (!rawPointer) return;
		else if (!rawPointer->Count) {
			OccurException(SVM_IEC_ARRAY_NOTARRAY);
			return;
		} else if (rawPointer->Count <= index) {
			OccurException(SVM_IEC_ARRAY_INDEX_OUTOFRANGE);
			return;
		}

		m_Stack.Reduce(indexType->Size + (*pointerTypePtr)->Size);
		m_Stack.Push<PointerObject>(RawPointerObject{ rawPointer->Type, 0,
			static_cast<std::uint8_t*>(rawPointer->Address) + index * rawPointer->Type->RawSize });
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretCount() noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		} else if (m_Stack.GetFreeSize() < sizeof(LongObject)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const auto rawPointer = GetRawPointerObject(typePtr);
		if (!rawPointer) return;
		else if (!rawPointer->Count) {
			OccurException(SVM_IEC_ARRAY_NOTARRAY);
			return;
		}

		m_Stack.Reduce((*typePtr)->Size);
		m_Stack.Push<LongObject>(RawLongObject{ rawPointer->Count });
	}
}