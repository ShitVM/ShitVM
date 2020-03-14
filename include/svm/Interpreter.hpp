#pragma once

#include <svm/ByteFile.hpp>
#include <svm/Exception.hpp>
#include <svm/Macro.hpp>
#include <svm/Stack.hpp>
#include <svm/Type.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

namespace svm {
	struct StackFrame final {
		svm::Type Type = NoneType;
		std::size_t StackBegin = 0;
		std::size_t VariableBegin = 0;
		std::size_t Caller = std::numeric_limits<std::size_t>::max();
		const svm::Function* Function = nullptr;
		const svm::Instructions* Instructions = nullptr;
	};
}

namespace svm {
	class Interpreter final {
	public:
		using Result = std::variant<std::monostate, std::uint32_t, std::uint64_t, double, void*, const StructureObject*>;

	private:
		ByteFile m_ByteFile;

		Stack m_Stack;
		StackFrame m_StackFrame;
		std::size_t m_Depth = 0;
		std::uint64_t m_InstructionIndex = 0;

		std::vector<std::size_t> m_LocalVariables;

		std::optional<InterpreterException> m_Exception;

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
		void AllocateStack(std::size_t size = 1 * 1024 * 1024);
		void ReallocateStack(std::size_t newSize);
		const ByteFile& GetByteFile() const noexcept;

		bool Interpret();
		const InterpreterException& GetException() const noexcept;
		Result GetResult() const noexcept;
		std::vector<StackFrame> GetCallStacks() const;

	private:
		void OccurException(std::uint32_t code) noexcept;
		bool IsLocalVariable(std::size_t delta = 0) const noexcept;

		void PushStructure(std::uint32_t code) noexcept;
		void InitStructure(Structure structure, Type* type) const noexcept;
		void CopyStructure(const Type& type) noexcept;
		void CopyStructure(const Type& from, Type& to) const noexcept;
		template<typename T>
		void DRefAndAssign(Type* rhsTypePtr) noexcept;
		template<typename T>
		bool GetTwoSameType(Type rhsType, T*& lhs) noexcept;

		template<typename T>
		bool PopTwoSameType(Type& rhsType, T& lhs, T& rhs) noexcept;
		template<typename T>
		IntObject CompareTwoSameType(T lhs, T rhs) noexcept;
		template<typename T>
		void JumpCondition(std::uint32_t operand);

		void InterpretPush(std::uint32_t operand);
		void InterpretPop();
		void InterpretLoad(std::uint32_t operand);
		void InterpretStore(std::uint32_t operand);
		void InterpretLea(std::uint32_t operand);
		void InterpretFLea(std::uint32_t operand);
		void InterpretTLoad();
		void InterpretTStore();
		void InterpretCopy();
		void InterpretSwap();

		void InterpretAdd();
		void InterpretSub();
		void InterpretMul();
		void InterpretIMul();
		void InterpretDiv();
		void InterpretIDiv();
		void InterpretMod();
		void InterpretIMod();
		void InterpretNeg();
		void InterpretIncDec(int delta);

		void InterpretAnd();
		void InterpretOr();
		void InterpretXor();
		void InterpretNot();
		void InterpretShl();
		void InterpretSal();
		void InterpretShr();
		void InterpretSar();

		void InterpretCmp();
		void InterpretICmp();
		void InterpretJmp(std::uint32_t operand);
		void InterpretJe(std::uint32_t operand);
		void InterpretJne(std::uint32_t operand);
		void InterpretJa(std::uint32_t operand);
		void InterpretJae(std::uint32_t operand);
		void InterpretJb(std::uint32_t operand);
		void InterpretJbe(std::uint32_t operand);
		void InterpretCall(std::uint32_t operand);
		void InterpretRet();

		void InterpretToI();
		void InterpretToL();
		void InterpretToD();
		void InterpretToP();

		void InterpretNull() noexcept;
		void InterpretNew(std::uint32_t operand) noexcept;
		void InterpretDelete() noexcept;
	};
}