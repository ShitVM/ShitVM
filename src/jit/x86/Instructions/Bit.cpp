#include <svm/jit/x86/Builder.hpp>

#ifdef SVM_X86

#include <cassert>

namespace svm::jit::x86 {
	void Builder::BitShiftInternal(std::uint8_t opCode, std::uint8_t opCodeExt, const RM& a) {
		Instruction& instruction = m_Instructions.emplace_back();
		instruction.OpCode = opCode;
		if (a.GetSize() != RMSize::Byte) {
			instruction.OpCode |= 0b1; // s=1
		}

		REX rex;
		rex.Fields.W = a.GetSize() == RMSize::QWord;

		ModRM modRM;
		GenerateModRM(a, rex, modRM);
		modRM.Fields.Reg = opCodeExt;

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
	void Builder::BitShiftInternal(std::uint8_t opCode, const RM& a) {
		BitShiftInternal(0xD0, opCode, a);
	}
	void Builder::BitShiftInternal(std::uint8_t opCode, const RM& a, std::uint8_t b) {
		BitShiftInternal(0xC0, opCode, a);
		m_Instructions.back().DispImm.Fields.Immediate = b;
		m_Instructions.back().ImmediateSize = 1;
	}
	void Builder::BitShiftInternal(std::uint8_t opCode, const RM& a, Register b) {
		assert(b == Cl);

		BitShiftInternal(0xD2, opCode, a);
	}
}

namespace svm::jit::x86 {
	void Builder::Shl(const RM& a) {
		BitShiftInternal(0b100, a);
	}
	void Builder::Shl(const RM& a, std::uint8_t b) {
		if (b == 1) {
			Shl(a);
		} else {
			BitShiftInternal(0b100, a, b);
		}
	}
	void Builder::Shl(const RM& a, Register b) {
		BitShiftInternal(0b100, a, b);
	}
	void Builder::Sal(const RM& a) {
		Shl(a);
	}
	void Builder::Sal(const RM& a, std::uint8_t b) {
		Shl(a, b);
	}
	void Builder::Sal(const RM& a, Register b) {
		Shl(a, b);
	}
	void Builder::Shr(const RM& a) {
		BitShiftInternal(0b101, a);
	}
	void Builder::Shr(const RM& a, std::uint8_t b) {
		if (b == 1) {
			Shr(a);
		} else {
			BitShiftInternal(0b101, a, b);
		}
	}
	void Builder::Shr(const RM& a, Register b) {
		BitShiftInternal(0b101, a, b);
	}
	void Builder::Sar(const RM& a) {
		BitShiftInternal(0b111, a);
	}
	void Builder::Sar(const RM& a, std::uint8_t b) {
		if (b == 1) {
			Sar(a);
		} else {
			BitShiftInternal(0b111, a, b);
		}
	}
	void Builder::Sar(const RM& a, Register b) {
		BitShiftInternal(0b111, a, b);
	}
}

#endif