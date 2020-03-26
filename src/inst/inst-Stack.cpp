#include <svm/Interpreter.hpp>

#include <svm/ConstantPool.hpp>
#include <svm/detail/InterpreterExceptionCode.hpp>

#include <algorithm>
#include <cstring>

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::PushStructure(std::uint32_t code) noexcept {
		if (code >= GetStructureCount()) {
			OccurException(SVM_IEC_CONSTANTPOOL_OUTOFRANGE);
			return;
		}

		const Structure structure = GetStructure(static_cast<TypeCode>(code));
		if (!m_Stack.Expand(structure->Type.Size)) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
			return;
		}

		InitStructure(structure, m_Stack.GetTopType());
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InitStructure(Structure structure, Type* type) noexcept {
		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->Fields.size());

		*type = structure->Type;

		for (std::uint32_t i = 0; i < fieldCount; ++i) {
			const Field& field = structure->Fields[i];
			Type* const pointer = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(type) + field.Offset);

			if (field.IsArray()) {
				*pointer = ArrayType;
				reinterpret_cast<ArrayObject*>(pointer)->Count = field.Count;

				detail::ArrayInfo info;
				info.ElementType = field.Type;
				info.Count = field.Count;
				InitArray(info, pointer);
			} else if (field.Type.IsStructure()) {
				InitStructure(GetStructure(field.Type), pointer);
			} else {
				*pointer = field.Type;
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
		const ConstantPool& constantPool = m_Program->GetConstantPool();
		if (operand >= constantPool.GetAllCount()) {
			PushStructure(operand);
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
				std::memcpy(m_Stack.GetTopType(), typePtr, size);
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
		} else if (type.IsArray()) {
			const std::size_t size = CalcArraySize(reinterpret_cast<const ArrayObject*>(typePtr));

			if (reinterpret_cast<ArrayObject&>(varType).Count != reinterpret_cast<const ArrayObject*>(typePtr)->Count) {
				OccurException(SVM_IEC_ARRAY_COUNT_DIFFERENTCOUNT);
				return;
			}

			std::memcpy(&varType, typePtr, size);
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

		if (!m_Stack.Push<PointerObject>(m_Stack.Get<Type>(m_LocalVariables[operand]))) {
			OccurException(SVM_IEC_STACK_OVERFLOW);
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
		} else if (type.IsArray()) {
			const std::size_t size = CalcArraySize(reinterpret_cast<const ArrayObject*>(typePtr));
			if (isSuccess = m_Stack.Expand(type->Size)) {
				std::memcpy(m_Stack.GetTopType(), typePtr, size);
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
		} else if (firstType.IsArray()) {
			const std::size_t size = CalcArraySize(reinterpret_cast<ArrayObject*>(firstTypePtr));

			if (IsLocalVariable() || IsLocalVariable(size)) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (!m_Stack.Get<Type>(m_Stack.GetUsedSize() - size)->IsArray()) {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			} else if (m_Stack.Get<ArrayObject>(m_Stack.GetUsedSize() - size)->Count != reinterpret_cast<ArrayObject*>(firstTypePtr)->Count) {
				OccurException(SVM_IEC_ARRAY_COUNT_DIFFERENTCOUNT);
				return;
			}

			for (std::size_t i = sizeof(ArrayObject); i < size; i += sizeof(void*)) {
				std::iter_swap(m_Stack.Get<void*>(m_Stack.GetUsedSize() - i), m_Stack.Get<void*>(m_Stack.GetUsedSize() - size - i));
			}
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
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

		info.ElementType = GetType(m_Program->GetStructures(), static_cast<TypeCode>(operand));
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
			info.Count = reinterpret_cast<IntObject*>(typePtr)->Value;
		} else if (type == LongType) {
			info.Count = reinterpret_cast<LongObject*>(typePtr)->Value;
		} else {
			OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
			return false;
		}

		if (info.Count == 0) {
			OccurException(SVM_IEC_ARRAY_COUNT_CANNOTBEZERO);
			return false;
		}

		info.Size = static_cast<std::size_t>(info.ElementType->Size * info.Count + sizeof(ArrayObject));
		return true;
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InitArray(const detail::ArrayInfo& info, Type* type) noexcept {
		std::memset(type, 0, info.Size);

		Structure structure;
		if (info.ElementType.IsStructure()) {
			structure = GetStructure(info.ElementType);
		}

		*type = ArrayType;
		reinterpret_cast<ArrayObject*>(type)->Count = static_cast<std::size_t>(info.Count);
		type = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(type) + sizeof(ArrayObject));

		for (std::uint64_t i = 0; i < info.Count; ++i) {
			if (info.ElementType.IsStructure()) {
				InitStructure(structure, type);
			} else {
				*type = info.ElementType;
			}
			type = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(type) + info.ElementType->Size);
		}
	}
	SVM_NOINLINE_FOR_PROFILING std::size_t Interpreter::CalcArraySize(const ArrayObject* array) const noexcept {
		const std::size_t elementSize = reinterpret_cast<const Type*>(array + 1)->GetReference().Size;
		return static_cast<std::size_t>(array->Count * elementSize + sizeof(ArrayObject));
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