#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

namespace svm {
	enum class OpCode : std::uint8_t {
		Nop,

		Push,
		Pop,
		Load,
		Store,
		Lea,
		FLea,
		TLoad,
		TStore,
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

		ToB,				// Not supported
		ToS,				// Not supported
		ToI,
		ToL,
		ToF,				// Not supported
		ToD,
		ToP,

		Null,
		New,
		Delete,
		GCNull,
		GCNew,

		APush,
		ANew,
		AGCNew,
		ALea,
		Count,
	};

	static constexpr const char* Mnemonics[] = {
		"nop",
		"push", "pop", "load", "store", "lea", "flea", "tload", "tstore", "copy", "swap",
		"add", "sub", "mul", "imul", "div", "idiv", "mod", "imod", "neg", "inc", "dec",
		"and", "or", "xor", "not", "shl", "shr", "sal", "sar",
		"cmp", "icmp", "jmp", "je", "jne", "ja", "jae", "jb", "jbe", "call", "ret",
		"tob", "tos", "toi", "tol", "tof", "tod", "top",
		"null", "new", "delete", "gcnull", "gcnew",
		"apush", "anew", "agcnew", "alea", "count",
	};

	static constexpr bool HasOperand[] = {
		false/*nop*/,
		true/*push*/, false/*pop*/, true/*load*/, true/*store*/, true/*lea*/, true/*flea*/, false/*tload*/, false/*tstore*/, false/*copy*/, false/*swap*/,
		false/*add*/, false/*sub*/, false/*mul*/, false/*imul*/, false/*div*/, false/*idiv*/, false/*mod*/, false/*imod*/, false/*neg*/, false/*inc*/, false/*dec*/,
		false/*and*/, false/*or*/, false/*xor*/, false/*not*/, false/*shl*/, false/*shr*/, false/*sal*/, false/*sar*/,
		false/*cmp*/, false/*icmp*/, true/*jmp*/, true/*je*/, true/*jne*/, true/*ja*/, true/*jae*/, true/*jb*/, true/*jbe*/, true/*call*/, false/*ret*/,
		false/*tob*/, false/*tos*/, false/*toi*/, false/*tol*/, false/*tof*/, false/*tod*/, false/*top*/,
		false/*null*/, true/*new*/, false/*delete*/, false/*gcnull*/, true/*gcnew*/,
		true/*apush*/, true/*anew*/, true/*agcnew*/, false/*alea*/, false/*count*/,
	};
}

namespace svm {
	class Instruction final {
	public:
		svm::OpCode OpCode = OpCode::Nop;
		std::uint32_t Operand = 0;
		std::uint64_t Offset = 0;

	public:
		Instruction() noexcept = default;
		Instruction(svm::OpCode opCode, std::uint64_t offset) noexcept;
		Instruction(svm::OpCode opCode, std::uint32_t operand, std::uint64_t offset) noexcept;
		Instruction(const Instruction& instruction) noexcept;
		~Instruction() = default;

	public:
		Instruction& operator=(const Instruction& instruction) noexcept;
		bool operator==(const Instruction& instruction) const noexcept;
		bool operator!=(const Instruction& instruction) const noexcept;

	public:
		bool HasOperand() const noexcept;
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

		const std::vector<std::uint64_t>& GetLabels() const noexcept;
		const std::vector<Instruction>& GetInstructions() const noexcept;
		std::uint32_t GetLabelCount() const noexcept;
		std::uint64_t GetInstructionCount() const noexcept;

		std::uint64_t GetLabel(std::uint32_t index) const noexcept;
		const Instruction& GetInstruction(std::uint64_t offset) const noexcept;
	};

	std::ostream& operator<<(std::ostream& stream, const Instructions& instructions);
}