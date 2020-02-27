#include <svm/jit/x86/Builder.hpp>

#ifdef SVM_X86

#include <cassert>

namespace svm::jit::x86 {
	Builder::Builder(Builder&& builder) noexcept
		: m_Instructions(std::move(builder.m_Instructions)) {}

	Builder& Builder::operator=(Builder&& builder) noexcept {
		m_Instructions = std::move(builder.m_Instructions);
		return *this;
	}

	std::size_t Builder::GetResult(void* buffer) noexcept {
		std::uint8_t* byte = static_cast<std::uint8_t*>(buffer);
		for (const auto& inst : m_Instructions) {
			if (inst.LegacyPrefixGroup1.has_value()) {
				*byte++ = inst.LegacyPrefixGroup1.value();
			}
			if (inst.LegacyPrefixGroup2.has_value()) {
				*byte++ = inst.LegacyPrefixGroup2.value();
			}
			if (inst.LegacyPrefixGroup3.has_value()) {
				*byte++ = inst.LegacyPrefixGroup3.value();
			}
			if (inst.LegacyPrefixGroup4.has_value()) {
				*byte++ = inst.LegacyPrefixGroup4.value();
			}

#ifdef SVM_X64
			if (inst.REXPrefix.has_value()) {
				*byte++ = inst.REXPrefix.value().Byte;
			}
#endif
			if (inst.OpCodePrefix1.has_value()) {
				*byte++ = inst.OpCodePrefix1.value();
			}
			if (inst.OpCodePrefix2.has_value()) {
				*byte++ = inst.OpCodePrefix2.value();
			}
			*byte++ = inst.OpCode;
			if (inst.ModRM.has_value()) {
				*byte++ = inst.ModRM.value().Byte;
			}
			if (inst.SIB.has_value()) {
				*byte++ = inst.SIB.value().Byte;
			}

			switch (inst.DisplacementSize) {
			case 1:
				*byte++ = static_cast<std::uint8_t>(inst.DispImm.Fields.Displacement);
				break;

			case 4:
				*reinterpret_cast<std::uint32_t*>(byte) = inst.DispImm.Fields.Displacement;
				byte += sizeof(std::uint32_t);
				break;

			case 8:
				*reinterpret_cast<std::uint64_t*>(byte) = inst.DispImm.Displacement64;
				byte += sizeof(std::uint64_t);
				break;
			}

			switch (inst.ImmediateSize) {
			case 1:
				*byte++ = static_cast<std::uint8_t>(inst.DispImm.Fields.Immediate);
				break;

			case 4:
				*reinterpret_cast<std::uint32_t*>(byte) = inst.DispImm.Fields.Immediate;
				byte += sizeof(std::uint32_t);
				break;

			case 8:
				*reinterpret_cast<std::uint64_t*>(byte) = inst.DispImm.Immediate64;
				byte += sizeof(std::uint64_t);
				break;
			}
		}

		m_Instructions.clear();
		return byte - static_cast<std::uint8_t*>(buffer);
	}

	void Builder::GenerateModRM(Register reg, REX& rex, ModRM& modRM) noexcept {
		modRM.Fields.Mod = 0b11;

		rex.Fields.B = reg->Code >> 3;
		modRM.Fields.RM = reg->Code & 0b111;
	}
	void Builder::GenerateModRM(const Address& addr, REX& rex, ModRM& modRM) noexcept {
		switch (addr.GetMode()) {
		case AddressingMode::RegisterIndirect:
			if (addr.HasDisplacement()) {
				if (addr.IsDisplacementByte()) {
					modRM.Fields.Mod = 0b01;
				} else if (addr.IsDisplacementDWord()) {
					modRM.Fields.Mod = 0b10;
				}
			}
			if (addr.HasSIB()) {
				modRM.Fields.RM = 0b100;
			} else {
				rex.Fields.B = addr.GetBaseRegister()->Code >> 3;
				modRM.Fields.RM = addr.GetBaseRegister()->Code & 0b111;
			}
			break;

		case AddressingMode::DirectMemory:
			modRM.Fields.RM = 0b100;
			break;
		}
	}
	void Builder::GenerateModRM(Register reg1, Register reg2, REX& rex, ModRM& modRM) noexcept {
		GenerateModRM(reg2, rex, modRM);
		rex.Fields.R = reg1->Code >> 3;
		modRM.Fields.Reg = reg1->Code & 0b111;
	}
	void Builder::GenerateModRM(Register reg, const Address& addr, REX& rex, ModRM& modRM) noexcept {
		rex.Fields.R = reg->Code >> 3;
		modRM.Fields.Reg = reg->Code & 0b111;

		GenerateModRM(addr, rex, modRM);
	}
	bool Builder::GenerateSIB(const Address& addr, REX& rex, SIB& sib) noexcept {
		if (addr.HasSIB()) {
			switch (addr.GetScale()) {
			case 1: sib.Fields.Scale = 0; break;
			case 2: sib.Fields.Scale = 1; break;
			case 4: sib.Fields.Scale = 2; break;
			case 8: sib.Fields.Scale = 3; break;
			default: assert(false); break;
			}

			rex.Fields.X = addr.GetIndex()->Code >> 3;
			sib.Fields.Index = addr.GetIndex()->Code & 0b111;
			rex.Fields.B = addr.GetBaseRegister()->Code >> 3;
			sib.Fields.Base = addr.GetBaseRegister()->Code & 0b111;

			return true;
		} else if (addr.IsBaseAddress()) {
			sib.Byte = 0x25_b;

			return true;
		}

		return false;
	}
	void Builder::GenerateDisplacement(const Address& addr, DispImm& disp, std::uint8_t& dispSize) noexcept {
		if (addr.HasDisplacement()) {
			if (addr.IsDisplacementByte()) {
				disp.Fields.Displacement = addr.GetDisplacement8();
				dispSize = sizeof(std::uint8_t);
			} else if (addr.IsDisplacementDWord()) {
				disp.Fields.Displacement = addr.GetDisplacement32();
				dispSize = sizeof(std::uint32_t);
			}
		} else if (addr.IsBaseAddress()) {
			disp.Fields.Displacement = addr.GetBaseAddress();
			dispSize = sizeof(std::uint32_t);
		}
	}
}

#endif