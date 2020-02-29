#include <svm/jit/x86/Builder.hpp>

#ifdef SVM_X86

#include <cassert>
#include <limits>

namespace svm::jit::x86 {
	void Builder::AddSubInternal(std::uint8_t opCode, Register a, const RM& b) {
		assert(a->Size == b.GetSize());

		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = opCode;
		if (a->Size != RegisterSize::Byte) {
			instruction.OpCode |= 0b1; //s=1
		}

		REX rex;
		rex.Fields.W = a->Size == RegisterSize::QWord;

		ModRM modRM;
		GenerateModRM(a, b, rex, modRM);

		if (b.IsAddress()) {
			SIB sib;
			if (GenerateSIB(b.GetAddress(), rex, sib)) {
				instruction.SIB = sib;
			}

			GenerateDisplacement(b.GetAddress(), instruction.DispImm, instruction.DisplacementSize);
		}

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;
	}
	void Builder::AddSubInternal(std::uint8_t opCode, const Memory& a, Register b) {
		assert(a.GetSize() == b->Size);

		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = opCode;
		if (a.GetSize() != MemorySize::Byte) {
			instruction.OpCode |= 0b1; //s=1
		}

		REX rex;
		rex.Fields.W = a.GetSize() == MemorySize::QWord;

		ModRM modRM;
		GenerateModRM(b, a, rex, modRM);

		SIB sib;
		const bool hasSIB = GenerateSIB(a, rex, sib);

		GenerateDisplacement(a, instruction.DispImm, instruction.DisplacementSize);

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;
		if (hasSIB) {
			instruction.SIB = sib;
		}
	}
	void Builder::AddSubInternal(std::uint8_t opCode, const RM& a, std::uint32_t b) {
		Instruction& instruction = m_Instructions.emplace_back();

		REX rex;
		rex.Fields.W = a.GetSize() == RMSize::QWord;

		ModRM modRM;
		GenerateModRM(a, rex, modRM);
		modRM.Fields.Reg = opCode;

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;

		if (b <= std::numeric_limits<std::uint8_t>::max()) {
			assert(a.GetSize() != RMSize::Byte);

			instruction.OpCode = 0x83;

			instruction.DispImm.Fields.Displacement = static_cast<std::uint8_t>(b);
			instruction.DisplacementSize = sizeof(std::uint8_t);
		} else {
			instruction.OpCode = 0x80;
			if (a.GetSize() != RegisterSize::Byte) {
				instruction.OpCode |= 0b1; //s=1
			}

			instruction.DispImm.Fields.Displacement = b;
			instruction.DisplacementSize = sizeof(std::uint32_t);
		}
	}
}

namespace svm::jit::x86 {
	void Builder::Add(Register a, const RM& b) {
		AddSubInternal(0x02, a, b);
	}
	void Builder::Add(const Memory& a, Register b) {
		AddSubInternal(0x00, a, b);
	}
	void Builder::Add(const RM& a, std::uint32_t b) {
		AddSubInternal(0b000, a, b);
	}

	void Builder::Sub(Register a, const RM& b) {
		AddSubInternal(0x2A, a, b);
	}
	void Builder::Sub(const Memory& a, Register b) {
		AddSubInternal(0x28, a, b);
	}
	void Builder::Sub(const RM& a, std::uint32_t b) {
		AddSubInternal(0b101, a, b);
	}
}

namespace svm::jit::x86 {
	void Builder::MulDivInternal(std::uint8_t opCode, const RM& a) {
		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = 0xF6_b;
		if (a.GetSize() != RMSize::Byte) {
			instruction.OpCode |= 0b1; //s=1
		}

		REX rex;
		rex.Fields.W = a.GetSize() == MemorySize::QWord;

		ModRM modRM;
		GenerateModRM(a, rex, modRM);
		modRM.Fields.Reg = opCode;

		if (a.IsAddress()) {
			SIB sib;
			if (GenerateSIB(a.GetAddress(), rex, sib)) {
				instruction.SIB = sib;
			}

			GenerateDisplacement(a.GetAddress(), instruction.DispImm, instruction.DisplacementSize);
		}

		instruction.REXPrefix = rex;
		instruction.ModRM = modRM;
	}
}

namespace svm::jit::x86 {
	void Builder::Mul(const RM& a) {
		MulDivInternal(0b100, a);
	}
	void Builder::IMul(const RM& a) {
		MulDivInternal(0b101, a);
	}
	void Builder::Div(const RM& a) {
		MulDivInternal(0b110, a);
	}
	void Builder::IDiv(const RM& a) {
		MulDivInternal(0b111, a);
	}
	void Builder::Neg(const RM& a) {
		MulDivInternal(0b011, a);
	}
}

#endif