#pragma once

#include <cstdint>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

namespace svm {
	enum class OpCode {
		Nop,

		Push,
		Pop,
		Load,
		Store,

		Add,
		Sub,
		Mul,
		IMul,
		Div,
		IDiv,
		Mod,
		IMod,
		Neg,

		And,
		Or,
		Xor,
		Not,
		Shl,
		Sal,
		Shr,
		Sar,

		Cmp,
		ICmp,
		Jmp,
		Je,
		Jne,
		Ja,
		Jae,
		Jb,
		Jbe,
		Call,
		Ret,

		I2L,
		I2D,
		L2I,
		L2D,
		D2I,
		D2L,
	};

	static constexpr const char* Mnemonics[] = {
		"nop",
		"push", "pop", "load", "store",
		"add", "sub", "mul", "imul", "div", "idiv", "mod", "imod", "neg",
		"and", "or", "xor", "not", "shl", "shr", "sal", "sar",
		"cmp", "icmp", "jmp", "je", "jne", "ja", "jae", "jb", "jbe", "call", "ret",
		"i2l", "i2d", "l2i", "l2d", "d2i", "d2l",
	};

	class Instruction final {
	public:
		static constexpr std::uint32_t NoOperand = std::numeric_limits<std::uint32_t>::max();
		static constexpr std::uint64_t NoOffset = std::numeric_limits<std::uint64_t>::max();

	public:
		svm::OpCode OpCode = OpCode::Nop;
		std::uint32_t Operand = NoOperand;

		std::uint64_t Offset = NoOffset;

	public:
		Instruction() noexcept = default;
		Instruction(svm::OpCode opCode) noexcept;
		Instruction(svm::OpCode opCode, std::uint64_t offset) noexcept;
		Instruction(svm::OpCode opCode, std::uint32_t operand) noexcept;
		Instruction(svm::OpCode opCode, std::uint32_t operand, std::uint64_t offset) noexcept;
		Instruction(const Instruction& instruction) noexcept;
		~Instruction() = default;

	public:
		Instruction& operator=(const Instruction& instruction) noexcept;
		bool operator==(const Instruction& instruction) const noexcept;
		bool operator!=(const Instruction& instruction) const noexcept;

	public:
		bool HasOperand() const noexcept;
		bool HasOffset() const noexcept;
		std::string ToString() const;
	};

	std::ostream& operator<<(std::ostream& stream, const Instruction& instruction);

	class Instructions final {
	private:
		std::unique_ptr<std::uint64_t[]> m_Labels;
		std::uint32_t m_LabelCount = 0;
		std::unique_ptr<Instruction[]> m_Instructions;
		std::uint64_t m_InstructionCount = 0;

	public:
		Instructions() noexcept = default;
		Instructions(std::unique_ptr<std::uint64_t[]>&& labels, std::uint32_t labelCount,
					 std::unique_ptr<Instruction[]> instructions, std::uint64_t instructionCount) noexcept;
		Instructions(Instructions&& instructions) noexcept;
		~Instructions() = default;

	public:
		Instructions& operator=(Instructions&& instructions) noexcept;
		bool operator==(const Instructions&) = delete;
		bool operator!=(const Instructions&) = delete;
		const Instruction& operator[](std::uint64_t offset) const noexcept;

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;

		std::uint64_t GetLabel(std::uint32_t index) const noexcept;
		const Instruction& GetInstruction(std::uint64_t offset) const noexcept;
		const std::uint64_t* GetLabels() const noexcept;
		const Instruction* GetInstructions() const noexcept;
		std::uint32_t GetLabelCount() const noexcept;
		std::uint64_t GetInstructionCount() const noexcept;
	};

	std::ostream& operator<<(std::ostream& stream, const Instructions& instructions);
}