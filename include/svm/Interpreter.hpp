#pragma once

#include <svm/ByteFile.hpp>
#include <svm/Exception.hpp>
#include <svm/Function.hpp>
#include <svm/GarbageCollector.hpp>
#include <svm/Heap.hpp>
#include <svm/Instruction.hpp>
#include <svm/Object.hpp>
#include <svm/Stack.hpp>
#include <svm/Type.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <variant>
#include <vector>

namespace svm {
	struct StackFrame final {
		svm::Type Type = NoneType;
		std::size_t StackBegin = 0;
		std::uint32_t VariableBegin = 0;
		std::uint64_t Caller = 0;
		const svm::Function* Function = nullptr;
		const svm::Instructions* Instructions = nullptr;
	};
}

namespace svm {
	class Interpreter final {
	private:
		ByteFile m_ByteFile;
		std::optional<InterpreterException> m_Exception;

		Stack m_Stack;
		StackFrame m_StackFrame;
		std::size_t m_Depth = 0;

		std::vector<std::size_t> m_LocalVariables;

		Heap m_Heap;

	public:
		Interpreter() noexcept = default;
		explicit Interpreter(ByteFile&& byteFile) noexcept;
		Interpreter(Interpreter&& interpreter) noexcept;
		~Interpreter() = default;

	public:
		Interpreter& operator=(Interpreter&& interpreter) noexcept;
		bool operator==(const ByteFile&) = delete;
		bool operator!=(const ByteFile&) = delete;

	public:
		void Clear() noexcept;
		void Load(ByteFile&& byteFile) noexcept;
		const ByteFile& GetByteFile() const noexcept;

		void AllocateStack(std::size_t size = 1 * 1024 * 1024);
		void ReallocateStack(std::size_t newSize);
		void SetGarbageCollector(std::unique_ptr<GarbageCollector>&& gc) noexcept;

		bool Interpret();
		const Object* GetResult() const noexcept;
		void PrintObject(std::ostream& stream, const Object& object) const;
		void PrintObject(std::ostream& stream, const Object& object, bool printPointerTarget) const;
		void PrintObject(std::ostream& stream, const Object* object) const;
		void PrintObject(std::ostream& stream, const Object* object, bool printPointerTarget) const;

		bool HasException() const noexcept;
		const InterpreterException& GetException() const noexcept;
		std::vector<StackFrame> GetCallStacks() const;

	public:
		const Type* GetLocalVariable(std::uint32_t index) const noexcept;
		Type* GetLocalVariable(std::uint32_t index) noexcept;
		std::uint32_t GetLocalVariableCount() const noexcept;

	private:
		void PrintPointerTaget(std::ostream& stream, const Object& object) const;

	private:
		void OccurException(std::uint32_t code) noexcept;

		bool IsLocalVariable(std::size_t delta = 0) const noexcept;

	private: // Stack
		void PushStructure(std::uint32_t code) noexcept;
		void InitStructure(const Structures& structures, Structure structure, Type* type) noexcept;
		void CopyStructure(const Type& type) noexcept;
		void CopyStructure(const Type& from, Type& to) noexcept;

		template<typename T>
		void DRefAndAssign(const Type* rhsTypePtr) noexcept;

		template<typename T>
		bool GetTwoSameType(Type rhsType, T*& lhs) noexcept;

	private:
		void InterpretPush(std::uint32_t operand) noexcept;
		void InterpretPop() noexcept;
		void InterpretLoad(std::uint32_t operand) noexcept;
		void InterpretStore(std::uint32_t operand);
		void InterpretLea(std::uint32_t operand) noexcept;
		void InterpretFLea(std::uint32_t operand) noexcept;
		void InterpretTLoad() noexcept;
		void InterpretTStore() noexcept;
		void InterpretCopy() noexcept;
		void InterpretSwap() noexcept;

	private: // Type-cast
		template<typename F, typename T>
		void TypeCast(Type* typePtr) noexcept;

	private:
		void InterpretToI() noexcept;
		void InterpretToL() noexcept;
		void InterpretToD() noexcept;
		void InterpretToP() noexcept;

	private: // Memory
		void InterpretNull() noexcept;
		void InterpretNew(std::uint32_t operand);
		void InterpretDelete() noexcept;
		void InterpretGCNull() noexcept;
		void InterpretGCNew(std::uint32_t operand);

	private: // Operation
		template<typename T>
		bool PopTwoSameTypeAndPushOne(const Type* rhsTypePtr, T*& lhs, const T*& rhs) noexcept;

		template<typename T>
		bool PopTwoSameType(const Type* rhsTypePtr, T& lhs, T& rhs) noexcept;
		template<typename T>
		IntObject CompareTwoSameType(T lhs, T rhs) noexcept;

	private:
		void InterpretAdd() noexcept;
		void InterpretSub() noexcept;
		void InterpretMul() noexcept;
		void InterpretIMul() noexcept;
		void InterpretDiv() noexcept;
		void InterpretIDiv() noexcept;
		void InterpretMod() noexcept;
		void InterpretIMod() noexcept;
		void InterpretNeg() noexcept;
		void InterpretIncDec(int delta) noexcept;

		void InterpretAnd() noexcept;
		void InterpretOr() noexcept;
		void InterpretXor() noexcept;
		void InterpretNot() noexcept;
		void InterpretShl() noexcept;
		void InterpretSal() noexcept;
		void InterpretShr() noexcept;
		void InterpretSar() noexcept;

		void InterpretCmp() noexcept;
		void InterpretICmp() noexcept;

	private: // Control
		template<typename T>
		void JumpCondition(std::uint32_t operand) noexcept;

	private:
		void InterpretJmp(std::uint32_t operand) noexcept;
		void InterpretJe(std::uint32_t operand) noexcept;
		void InterpretJne(std::uint32_t operand) noexcept;
		void InterpretJa(std::uint32_t operand) noexcept;
		void InterpretJae(std::uint32_t operand) noexcept;
		void InterpretJb(std::uint32_t operand) noexcept;
		void InterpretJbe(std::uint32_t operand) noexcept;
		void InterpretCall(std::uint32_t operand);
		void InterpretRet() noexcept;
	};
}