#pragma once

#include <cstdint>
#include <limits>
#include <ostream>
#include <string>

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
	};

	static constexpr const char* Mnemonics[] = {
		"nop",
		"push", "pop", "load", "store",
		"add", "sub", "mul", "imul", "div", "idiv", "mod", "imod", "neg",
		"and", "or", "xor", "not", "shl", "shr", "sal", "sar",
		"cmp", "icmp", "jmp", "je", "jne", "ja", "jae", "jb", "jbe", "call", "ret",
	};

	class Instruction final {
	public:
		static constexpr std::uint32_t NoOperand = std::numeric_limits<std::uint32_t>::max();

	public:
		svm::OpCode OpCode = OpCode::Nop;
		std::uint32_t Operand = NoOperand;

	public:
		Instruction() noexcept = default;
		Instruction(svm::OpCode opCode) noexcept;
		Instruction(svm::OpCode opCode, std::uint32_t operand) noexcept;
		Instruction(const Instruction& instruction) noexcept;
		~Instruction() = default;

	public:
		Instruction& operator=(const Instruction& instruction) noexcept;
		bool operator==(const Instruction& instruction) const noexcept;
		bool operator!=(const Instruction& instruction) const noexcept;

	public:
		bool HasOperand() const noexcept;
		std::string ToString() const;
	};

	std::ostream& operator<<(std::ostream& stream, const Instruction& instruction);
}