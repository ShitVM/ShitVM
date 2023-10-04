#include <svm/Interpreter.hpp>

#include <svm/ConstantPool.hpp>
#include <svm/Memory.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <algorithm>
#include <cstring>

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::PushStructure(std::uint32_t code) noexcept {
		if (code >= GetStructureCount()) {
			OccurException(SVM_IEC_CONSTANTPOOL_OUTOFRANGE);
			return;
		}

		const Structure structure = GetStructure(static_cast<TypeCode>(code + static_cast<std::uint32_t>(TypeCode::Structure)));
		if (!m_Stack.Expand(structure->Type.Size)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		InitStructure(structure, m_Stack.GetTopType());
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InitStructure(Structure structure, Type* type) noexcept {
		*type = structure->Type;

		InitStructure(structure, reinterpret_cast<StructureObject*>(type) + 1);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InitStructure(Structure structure, void* firstField) noexcept {
		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->Fields.size());

		for (std::uint32_t i = 0; i < fieldCount; ++i) {
			const Field& field = structure->Fields[i];
			void* const pointer = static_cast<std::uint8_t*>(firstField) + field.Offset;

			if (field.IsArray()) {
				detail::ArrayInfo info;
				info.ElementType = field.Type;
				info.Count = field.Count;
				InitArray(info, pointer);
			} else if (field.Type.IsStructure()) {
				InitStructure(GetStructure(field.Type), pointer);
			}
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::CopyStructure(const Type& type) noexcept {
		CopyStructure(type, *m_Stack.GetTopType());
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::CopyStructure(const Type& from, Type& to) noexcept {
		std::memcpy(&to, &from, from->Size);
	}

	SVM_NOINLINE_FOR_PROFILING void Interpreter::InitStructure(Object* object, Structure structure) noexcept {
		InitStructure(structure, reinterpret_cast<Type*>(object));
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretPush(std::uint32_t operand) noexcept {
		const ConstantPool& constantPool = static_cast<const ConstantPool&>(
			std::get<core::ByteFile>(m_StackFrame.Program->Module).GetConstantPool());
		if (operand >= constantPool.GetAllCount()) {
			PushStructure(operand - constantPool.GetAllCount());
			return;
		}

		const Type constType = constantPool.GetConstantType(operand);
		bool isSuccess = false;
		if (constType == IntType) {
			isSuccess = m_Stack.Push(constantPool.GetConstant<IntObject>(operand));
		} else if (constType == LongType) {
			isSuccess = m_Stack.Push(constantPool.GetConstant<LongObject>(operand));
		} else if (constType == SingleType) {
			isSuccess = m_Stack.Push(constantPool.GetConstant<SingleObject>(operand));
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
		if (type.IsArray()) {
			m_Stack.Reduce(CalcArraySize(reinterpret_cast<const ArrayObject*>(typePtr)));
		} else if (type.IsValidType()) {
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
		} else if (type == SingleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const SingleObject&>(*typePtr));
		} else if (type == DoubleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const DoubleObject&>(*typePtr));
		} else if (type == PointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const PointerObject&>(*typePtr));
		} else if (type == GCPointerType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const GCPointerObject&>(*typePtr));
		} else if (type.IsStructure() && (isSuccess = m_Stack.Expand(type->Size))) {
			CopyStructure(*typePtr);
		} else if (type.IsArray()) {
			const std::size_t size = CalcArraySize(reinterpret_cast<const ArrayObject*>(typePtr));
			if ((isSuccess = m_Stack.Expand(size))) {
				CopyArray(*typePtr, *m_Stack.GetTopType(), size);
			}
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
			Type* const typePtr = m_Stack.GetTopType();
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
		} else if (type == SingleType) {
			reinterpret_cast<SingleObject&>(varType) = reinterpret_cast<const SingleObject&>(*typePtr);
		} else if (type == DoubleType) {
			reinterpret_cast<DoubleObject&>(varType) = reinterpret_cast<const DoubleObject&>(*typePtr);
		} else if (type == PointerType) {
			reinterpret_cast<PointerObject&>(varType) = reinterpret_cast<const PointerObject&>(*typePtr);
		} else if (type == GCPointerType) {
			reinterpret_cast<GCPointerObject&>(varType) = reinterpret_cast<const GCPointerObject&>(*typePtr);
		} else if (type.IsStructure()) {
			CopyStructure(*typePtr, varType);
		} else if (type.IsArray()) {
			if (reinterpret_cast<ArrayObject&>(varType).ElementType != reinterpret_cast<const ArrayObject*>(typePtr)->ElementType) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			} else if (reinterpret_cast<ArrayObject&>(varType).Count != reinterpret_cast<const ArrayObject*>(typePtr)->Count) {
				OccurException(SVM_IEC_ARRAY_COUNT_DIFFERENTCOUNT);
				return;
			} 

			CopyArray(*typePtr, varType);
			return;
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

		Type* const typePtr = m_Stack.Get<Type>(m_LocalVariables[operand]);
		RawPointerObject rawPointer;

		if (typePtr->IsArray()) {
			ArrayObject* const arrayPtr = reinterpret_cast<ArrayObject*>(typePtr);

			rawPointer.Type = arrayPtr->ElementType;
			rawPointer.Count = arrayPtr->Count;
			rawPointer.Address = arrayPtr + 1;

			if (!m_Stack.Push<PointerObject>(rawPointer)) {
				OccurException(SVM_IEC_STACK_OVERFLOW);
			}
		} else {
			rawPointer.Type = *typePtr;
			rawPointer.Address = reinterpret_cast<Object*>(typePtr) + 1;

			if (!m_Stack.Push<PointerObject>(rawPointer)) {
				OccurException(SVM_IEC_STACK_OVERFLOW);
			}
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
		} else if (type == SingleType) {
			isSuccess = m_Stack.Push(reinterpret_cast<const SingleObject&>(*typePtr));
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
		} else if (type.IsArray()) {
			const std::size_t size = CalcArraySize(reinterpret_cast<const ArrayObject*>(typePtr));
			if (isSuccess = m_Stack.Expand(type->Size)) {
				CopyArray(type, size);
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
		std::size_t firstTypeSize = firstType->Size;
		if (firstType.IsArray()) {
			firstTypeSize = CalcArraySize(reinterpret_cast<ArrayObject*>(firstTypePtr));
		}

		Type* const secondTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - firstTypeSize);
		if (!secondTypePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type secondType = *secondTypePtr;
		std::size_t secondTypeSize = secondType->Size;
		if (secondType.IsArray()) {
			secondTypeSize = CalcArraySize(reinterpret_cast<ArrayObject*>(secondTypePtr));
		}

		if (firstTypeSize > secondTypeSize) {
			const auto buffer = std::make_unique<std::uint8_t[]>(secondTypeSize);
			std::memcpy(buffer.get(), secondTypePtr, secondTypeSize);
			std::memcpy(secondTypePtr, firstTypePtr, firstTypeSize);
			std::memcpy(firstTypePtr, buffer.get(), secondTypeSize);
		} else {
			const auto buffer = std::make_unique<std::uint8_t[]>(firstTypeSize);
			std::memcpy(buffer.get(), firstTypePtr, firstTypeSize);
			std::memcpy(firstTypePtr, secondTypePtr, secondTypeSize);
			std::memcpy(secondTypePtr, buffer.get(), firstTypeSize);
		}
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING bool Interpreter::GetArrayInfo(detail::ArrayInfo& info, std::uint32_t operand) noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return false;
		}

		if (operand >> 31 == 0) {
			OccurException(SVM_IEC_TYPE_OUTOFRANGE);
			return false;
		}
		operand &= 0x7FFFFFFF;

		info.ElementType = GetType(static_cast<TypeCode>(operand));
		if (info.ElementType == NoneType) {
			OccurException(SVM_IEC_TYPE_OUTOFRANGE);
			return false;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return false;
		}

		const Type type = *typePtr;
		info.CountSize = type->Size;
		if (type == IntType) {
			info.Count = reinterpret_cast<IntObject*>(typePtr)->RawObject.Value;
		} else if (type == LongType) {
			info.Count = reinterpret_cast<LongObject*>(typePtr)->RawObject.Value;
		} else {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return false;
		}

		if (info.Count == 0) {
			OccurException(SVM_IEC_ARRAY_COUNT_CANNOTBEZERO);
			return false;
		}

		info.Size = CalcArraySize(info.ElementType, info.Count);
		return true;
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InitArray(const detail::ArrayInfo& info, Type* type) noexcept {
		ArrayObject* const array = reinterpret_cast<ArrayObject*>(type);

		array->SetType(ArrayType);
		array->ElementType = info.ElementType;
		array->Count = static_cast<std::size_t>(info.Count);

		InitArray(info, array + 1);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InitArray(const detail::ArrayInfo& info, void* firstElement) noexcept {
		if (!info.ElementType.IsStructure()) return;

		const std::size_t rawSize = info.ElementType->RawSize;
		const Structure structure = GetStructure(info.ElementType);

		for (std::uint64_t i = 0; i < info.Count; ++i) {
			InitStructure(structure, static_cast<std::uint8_t*>(firstElement) + i * info.ElementType->RawSize);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::CopyArray(const Type& type, std::size_t size) noexcept {
		CopyArray(type, *m_Stack.GetTopType(), size);
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::CopyArray(const Type& from, Type& to, std::size_t size) noexcept {
		if (!size) {
			size = CalcArraySize(reinterpret_cast<const ArrayObject*>(&from);
		}

		std::memcpy(&to, &from, size);
	}
	SVM_NOINLINE_FOR_PROFILING std::size_t Interpreter::CalcArraySize(const ArrayObject* array) const noexcept {
		return CalcArraySize(array->ElementType, array->Count);
	}

	void Interpreter::InitArray(Object* object, Type type, std::uint64_t count) noexcept {
		detail::ArrayInfo info;
		info.ElementType = type;
		info.Count = count;
		info.Size = CalcArraySize(type, count);

		InitArray(info, reinterpret_cast<Type*>(object));
	}
	std::size_t Interpreter::CalcArraySize(Type type, std::uint64_t count) const noexcept {
		return CalcNearestMultiplier(
			static_cast<std::size_t>(count * type->RawSize + sizeof(ArrayObject)),
			ObjectAlignment);
	}
}

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretAPush(std::uint32_t operand) noexcept {
		detail::ArrayInfo info;
		if (!GetArrayInfo(info, operand)) {
			return;
		}

		if (m_Stack.GetFreeSize() < info.Size - info.CountSize) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		m_Stack.Expand(info.Size - info.CountSize);
		InitArray(info, m_Stack.GetTopType());
	}
}