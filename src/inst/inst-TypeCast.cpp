#include <svm/Interpreter.hpp>

#include <svm/detail/InterpreterExceptionCode.hpp>

#include <type_traits>

namespace svm {
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToI() noexcept {
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
			TypeCast<IntObject, IntObject>(typePtr);
		} else if (type == LongType) {
			TypeCast<IntObject, LongObject>(typePtr);
		} else if (type == SingleType) {
			TypeCast<IntObject, SingleObject>(typePtr);
		} else if (type == DoubleType) {
			TypeCast<IntObject, DoubleObject>(typePtr);
		} else if (type == PointerType) {
			TypeCast<IntObject, PointerObject>(typePtr);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToL() noexcept {
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
			TypeCast<LongObject, IntObject>(typePtr);
		} else if (type == LongType) {
			TypeCast<LongObject, LongObject>(typePtr);
		} else if (type == SingleType) {
			TypeCast<LongObject, SingleObject>(typePtr);
		} else if (type == DoubleType) {
			TypeCast<LongObject, DoubleObject>(typePtr);
		} else if (type == PointerType) {
			TypeCast<LongObject, PointerObject>(typePtr);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToSi() noexcept {
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
			TypeCast<SingleObject, IntObject>(typePtr);
		} else if (type == LongType) {
			TypeCast<SingleObject, LongObject>(typePtr);
		} else if (type == SingleType) {
			TypeCast<SingleObject, SingleObject>(typePtr);
		} else if (type == DoubleType) {
			TypeCast<SingleObject, DoubleObject>(typePtr);
		} else if (type == PointerType) {
			TypeCast<SingleObject, PointerObject>(typePtr);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToD() noexcept {
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
			TypeCast<DoubleObject, IntObject>(typePtr);
		} else if (type == LongType) {
			TypeCast<DoubleObject, LongObject>(typePtr);
		} else if (type == SingleType) {
			TypeCast<DoubleObject, SingleObject>(typePtr);
		} else if (type == DoubleType) {
			TypeCast<DoubleObject, DoubleObject>(typePtr);
		} else if (type == PointerType) {
			TypeCast<DoubleObject, PointerObject>(typePtr);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
	SVM_NOINLINE_FOR_PROFILING void Interpreter::InterpretToP(std::uint32_t operand) noexcept {
		if (IsLocalVariable()) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		Type* const typePtr = m_Stack.GetTopType();
		if (!typePtr) {
			OccurException(SVM_IEC_STACK_EMPTY);
			return;
		}

		const Type targetType = GetType(static_cast<TypeCode>(operand & 0x7FFFFFFF));
		std::size_t targetCount = 0;
		std::size_t countSize = 0;

		if (operand >> 31) {
			const Type* const countTypePtr = m_Stack.Get<Type>(m_Stack.GetUsedSize() - (*typePtr)->Size);
			if (!countTypePtr || IsLocalVariable((*typePtr)->Size)) {
				OccurException(SVM_IEC_STACK_EMPTY);
				return;
			} else if (*countTypePtr == IntType) {
				targetCount = static_cast<std::size_t>(
					reinterpret_cast<const IntObject*>(countTypePtr)->RawObject.Value);
			} else if (*countTypePtr == LongType) {
				targetCount = static_cast<std::size_t>(
					reinterpret_cast<const LongObject*>(countTypePtr)->RawObject.Value);
			} else {
				OccurException(SVM_IEC_STACK_DIFFERENTTYPE);
				return;
			}

			countSize = (*countTypePtr)->Size;
		}

		const Type type = *typePtr;
		if (type == IntType) {
			PointerCast<IntObject>(typePtr, targetType, targetCount, countSize);
		} else if (type == LongType) {
			PointerCast<LongObject>(typePtr, targetType, targetCount, countSize);
		} else if (type == SingleType) {
			PointerCast<SingleObject>(typePtr, targetType, targetCount, countSize);
		} else if (type == DoubleType) {
			PointerCast<DoubleObject>(typePtr, targetType, targetCount, countSize);
		} else if (type == PointerType) {
			PointerCast<PointerObject>(typePtr, targetType, targetCount, countSize);
		} else if (type == GCPointerType) {
			OccurException(SVM_IEC_POINTER_INVALIDFORPOINTER);
		} else if (type.IsStructure()) {
			OccurException(SVM_IEC_STRUCTURE_INVALIDFORSTRUCTURE);
		} else if (type.IsArray()) {
			OccurException(SVM_IEC_ARRAY_INVALIDFORARRAY);
		} else {
			OccurException(SVM_IEC_STACK_EMPTY);
		}
	}
}

namespace svm {
	template<typename T, typename F>
	SVM_NOINLINE_FOR_PROFILING void Interpreter::TypeCast(Type* typePtr) noexcept {
		if constexpr (std::is_same_v<F, T>) return;
		else if constexpr (sizeof(T) > sizeof(F)) {
			if (!m_Stack.Expand(sizeof(T) - sizeof(F))) {
				OccurException(SVM_IEC_STACK_OVERFLOW);
				return;
			}
		} else if constexpr (sizeof(T) < sizeof(F)) {
			m_Stack.Reduce(sizeof(F) - sizeof(T));
		}

		*m_Stack.GetTop<T>() = reinterpret_cast<F*>(typePtr)->template Cast<T>();
	}
	template<typename F>
	SVM_NOINLINE_FOR_PROFILING void Interpreter::PointerCast(
		Type* typePtr, Type targetType, std::size_t targetCount, std::size_t countSize) noexcept {
		if (sizeof(PointerObject) > sizeof(F) + countSize) {
			if (!m_Stack.Expand(sizeof(PointerObject) - sizeof(F) - countSize)) {
				OccurException(SVM_IEC_STACK_OVERFLOW);
				return;
			}
		} else if (sizeof(PointerObject) < sizeof(F) + countSize) {
			m_Stack.Reduce(sizeof(F) + countSize - sizeof(PointerObject));
		}

		*m_Stack.GetTop<PointerObject>() = reinterpret_cast<F*>(typePtr)->CastToPointer(targetType, targetCount);
	}
}