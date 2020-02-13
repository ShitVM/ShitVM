#include <svm/Instruction.hpp>

#include <sstream>

namespace svm {
	Instruction::Instruction(svm::OpCode opCode) noexcept
		: OpCode(opCode) {}
	Instruction::Instruction(svm::OpCode opCode, std::uint32_t operand) noexcept
		: OpCode(opCode), Operand(operand) {}
	Instruction::Instruction(const Instruction& instruction) noexcept
		: OpCode(instruction.OpCode), Operand(instruction.Operand) {}

	Instruction& Instruction::operator=(const Instruction& instruction) noexcept {
		OpCode = instruction.OpCode;
		Operand = instruction.Operand;
		return *this;
	}
	bool Instruction::operator==(const Instruction& instruction) const noexcept {
		return OpCode == instruction.OpCode && Operand == instruction.Operand;
	}
	bool Instruction::operator!=(const Instruction& instruction) const noexcept {
		return OpCode != instruction.OpCode || Operand != instruction.Operand;
	}

	bool Instruction::HasOperand() const noexcept {
		return Operand != NoOperand;
	}
	std::string Instruction::ToString() const {
		std::ostringstream oss;
		oss << *this;
		return oss.str();
	}

	std::ostream& operator<<(std::ostream& stream, const Instruction& instruction) {
		stream << Mnemonics[static_cast<int>(instruction.OpCode)];
		if (instruction.HasOperand()) {
			stream << ' ' << std::showbase << std::hex << instruction.Operand;
		}
		return stream;
	}
}