#pragma once

#include <cstdint>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

namespace svm {
	enum class OpCode : std::uint8_t {
		Nop,

		Push,
		Pop,
		Load,
		Store,
		Lea,
		DRef,
		Copy,
		Swap,

		Add,
		Sub,
		Mul,
		IMul,
		Div,
		IDiv,
		Mod,
		IMod,
		Neg,
		Inc,
		Dec,

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

		ToI,
		ToL,
		ToD,
		ToP,
	};

	static constexpr const char* Mnemonics[] = {
		"nop",
		"push", "pop", "load", "store", "lea", "dref", "copy", "swap",
		"add", "sub", "mul", "imul", "div", "idiv", "mod", "imod", "neg", "inc", "dec",
		"and", "or", "xor", "not", "shl", "shr", "sal", "sar",
		"cmp", "icmp", "jmp", "je", "jne", "ja", "jae", "jb", "jbe", "call", "ret",
		"toi", "tol", "tod", "top",
	};
}

namespace svm {
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
	};

	std::ostream& operator<<(std::ostream& stream, const Instruction& instruction);
}

namespace svm {
	class Instructions final {
	private:
		std::vector<std::uint64_t> m_Labels;
		std::vector<Instruction> m_Instructions;

	public:
		Instructions() noexcept = default;
		Instructions(std::vector<std::uint64_t> labels, std::vector<Instruction> instructions) noexcept;
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
		const std::vector<std::uint64_t>& GetLabels() const noexcept;
		const std::vector<Instruction>& GetInstructions() const noexcept;
		std::uint32_t GetLabelCount() const noexcept;
		std::uint64_t GetInstructionCount() const noexcept;
	};

	std::ostream& operator<<(std::ostream& stream, const Instructions& instructions);
}