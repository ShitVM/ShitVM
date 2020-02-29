#include <svm/jit/x86/Memory.hpp>

#ifdef SVM_X86

#include <cassert>
#include <limits>

namespace svm::jit::x86 {
	Memory::Memory(Register base, MemorySize size) noexcept
		: m_Base(base), m_Size(size) {}
	Memory::Memory(Register base, std::uint8_t disp, MemorySize size) noexcept
		: m_Base(base), m_Displacement(disp), m_Size(size) {}
	Memory::Memory(Register base, std::uint32_t disp, MemorySize size) noexcept
		: m_Base(base), m_Displacement(disp), m_Size(size) {}
	Memory::Memory(Register base, std::uint8_t scale, Register index, MemorySize size) noexcept
		: m_Base(base), m_Scale(scale), m_Index(index), m_Size(size) {}
	Memory::Memory(Register base, std::uint8_t scale, Register index, std::uint8_t disp, MemorySize size) noexcept
		: m_Base(base), m_Scale(scale), m_Index(index), m_Displacement(disp), m_Size(size) {}
	Memory::Memory(Register base, std::uint8_t scale, Register index, std::uint32_t disp, MemorySize size) noexcept
		: m_Base(base), m_Scale(scale), m_Index(index), m_Displacement(disp), m_Size(size) {}
	Memory::Memory(std::uint32_t base, MemorySize size) noexcept
		: m_Base(base), m_Size(size) {}
	Memory::Memory(const Memory& address) noexcept
		: m_Base(address.m_Base), m_Scale(address.m_Scale), m_Index(address.m_Index), m_Displacement(address.m_Displacement), m_Size(address.m_Size) {}
	Memory::Memory(const Memory& address, MemorySize size) noexcept
		: m_Base(address.m_Base), m_Scale(address.m_Scale), m_Index(address.m_Index), m_Displacement(address.m_Displacement), m_Size(size) {}

	Memory& Memory::operator=(const Memory& address) noexcept {
		m_Base = address.m_Base;
		m_Scale = address.m_Scale;
		m_Index = address.m_Index;
		m_Displacement = address.m_Displacement;
		return *this;
	}
	AddressingMode Memory::GetMode() const noexcept {
		return static_cast<AddressingMode>(m_Base.index());
	}
	bool Memory::IsBaseRegister() const noexcept {
		return std::holds_alternative<Register>(m_Base);
	}
	bool Memory::IsBaseAddress() const noexcept {
		return std::holds_alternative<std::uint32_t>(m_Base);
	}
	Register Memory::GetBaseRegister() const noexcept {
		return std::get<Register>(m_Base);
	}
	std::uint32_t Memory::GetBaseAddress() const noexcept {
		return std::get<std::uint32_t>(m_Base);
	}
	void Memory::SetBase(Register newBase) noexcept {
		m_Base = newBase;
	}
	void Memory::SetBase(std::uint32_t newBase) noexcept {
		m_Base = newBase;
	}

	bool Memory::HasSIB() const noexcept {
		return m_Scale != 0;
	}
	void Memory::RemoveSBI() noexcept {
		m_Scale = 0;
		m_Index.reset();
	}
	std::uint8_t Memory::GetScale() const noexcept {
		return m_Scale;
	}
	void Memory::SetScale(std::uint8_t newScale) noexcept {
		m_Scale = newScale;
	}
	Register Memory::GetIndex() const noexcept {
		return m_Index.value();
	}
	void Memory::SetIndex(Register newIndex) noexcept {
		m_Index = newIndex;
	}
	bool Memory::HasDisplacement() const noexcept {
		return !std::holds_alternative<std::monostate>(m_Displacement);
	}
	void Memory::RemoveDisplacement() noexcept {
		m_Displacement = std::monostate();
	}
	bool Memory::IsDisplacementByte() const noexcept {
		return std::holds_alternative<std::uint8_t>(m_Displacement);
	}
	bool Memory::IsDisplacementDWord() const noexcept {
		return std::holds_alternative<std::uint32_t>(m_Displacement);
	}
	std::uint8_t Memory::GetDisplacementByte() const noexcept {
		return std::get<std::uint8_t>(m_Displacement);
	}
	std::uint32_t Memory::GetDisplacementDWord() const noexcept {
		return std::get<std::uint32_t>(m_Displacement);
	}
	void Memory::SetDisplacement(std::uint8_t newDisplacement) noexcept {
		m_Displacement = newDisplacement;
	}
	void Memory::SetDisplacement(std::uint32_t newDisplacement) noexcept {
		m_Displacement = newDisplacement;
	}

	MemorySize Memory::GetSize() const noexcept {
		return m_Size;
	}
	void Memory::SetSize(MemorySize newSize) noexcept {
		m_Size = newSize;
	}
}

namespace svm::jit::x86 {
	detail::IncompleteMemory operator*(Register index, std::uint8_t scale) noexcept {
		Memory addr;
		addr.SetScale(scale);
		addr.SetIndex(index);
		return { addr };
	}
	detail::IncompleteMemory operator*(std::uint8_t scale, Register index) noexcept {
		return index * scale;
	}
	detail::IncompleteMemory operator+(Register base, std::uint8_t disp) noexcept {
		Memory addr;
		addr.SetBase(base);
		addr.SetDisplacement(disp);
		return { addr };
	}
	detail::IncompleteMemory operator+(Register base, std::uint32_t disp) noexcept {
		Memory addr;
		addr.SetBase(base);
		addr.SetDisplacement(disp);
		return { addr };
	}
	detail::IncompleteMemory operator+(std::uint8_t disp, Register base) noexcept {
		return base + disp;
	}
	detail::IncompleteMemory operator+(std::uint32_t disp, Register base) noexcept {
		return base + disp;
	}

	detail::IncompleteMemory operator+(Register base, detail::IncompleteMemory&& other) noexcept {
		assert(other.Memory.GetMode() != AddressingMode::RegisterIndirect);
		other.Memory.SetBase(base);
		return other;
	}
	detail::IncompleteMemory operator+(detail::IncompleteMemory&& other, Register base) noexcept {
		return base + std::move(other);
	}
	detail::IncompleteMemory operator+(std::uint8_t disp, detail::IncompleteMemory&& other) noexcept {
		if (other.Memory.IsDisplacementByte()) {
			const int temp = other.Memory.GetDisplacementByte() + disp;
			if (temp > std::numeric_limits<std::uint8_t>::max()) {
				other.Memory.SetDisplacement(static_cast<std::uint32_t>(temp));
			} else {
				other.Memory.SetDisplacement(static_cast<std::uint8_t>(temp));
			}
		} else if (other.Memory.IsDisplacementDWord()) {
			other.Memory.SetDisplacement(other.Memory.GetDisplacementDWord() + disp);
		} else {
			other.Memory.SetDisplacement(disp);
		}
		return other;
	}
	detail::IncompleteMemory operator+(std::uint32_t disp, detail::IncompleteMemory&& other) noexcept {
		if (other.Memory.IsDisplacementByte()) {
			const std::uint32_t temp = other.Memory.GetDisplacementByte() + disp;
			if (temp > std::numeric_limits<std::uint8_t>::max()) {
				other.Memory.SetDisplacement(temp);
			} else {
				other.Memory.SetDisplacement(static_cast<std::uint8_t>(temp));
			}
		} else if (other.Memory.IsDisplacementDWord()) {
			other.Memory.SetDisplacement(other.Memory.GetDisplacementDWord() + disp);
		} else {
			other.Memory.SetDisplacement(disp);
		}
		return other;
	}
	detail::IncompleteMemory operator+(detail::IncompleteMemory&& other, std::uint8_t disp) noexcept {
		return disp + std::move(other);
	}
	detail::IncompleteMemory operator+(detail::IncompleteMemory&& other, std::uint32_t disp) noexcept {
		return disp + std::move(other);
	}
	detail::IncompleteMemory operator+(detail::IncompleteMemory&& a, detail::IncompleteMemory&& b) noexcept {
		assert(a.Memory.GetMode() == AddressingMode::None || b.Memory.GetMode() == AddressingMode::None);
		assert(!a.Memory.HasSIB() || !b.Memory.HasSIB());
		assert(!a.Memory.HasDisplacement() || !b.Memory.HasDisplacement());

		if (a.Memory.GetMode() == AddressingMode::None) {
			if (b.Memory.GetMode() == AddressingMode::RegisterIndirect) {
				a.Memory.SetBase(b.Memory.GetBaseRegister());
			} else if (b.Memory.GetMode() != AddressingMode::DirectMemory) {
				a.Memory.SetBase(b.Memory.GetBaseAddress());
			}
		}

		if (!a.Memory.HasSIB()) {
			a.Memory.SetScale(b.Memory.GetScale());
			a.Memory.SetIndex(b.Memory.GetIndex());
		}

		if (!a.Memory.HasDisplacement()) {
			if (b.Memory.IsDisplacementByte()) {
				a.Memory.SetDisplacement(b.Memory.GetDisplacementByte());
			} else if (b.Memory.IsDisplacementDWord()) {
				a.Memory.SetDisplacement(b.Memory.GetDisplacementDWord());
			}
		}

		return a;
	}
}

namespace svm::jit::x86 {
	namespace detail {
		Memory Byte::operator[](Register reg) const noexcept {
			return Memory(reg, MemorySize::Byte);
		}
		Memory Byte::operator[](std::uint32_t addr) const noexcept {
			return Memory(addr, MemorySize::Byte);
		}
		Memory Byte::operator[](IncompleteMemory&& addr) const noexcept {
			return { addr.Memory, MemorySize::Byte };
		}
		Memory DWord::operator[](Register reg) const noexcept {
			return Memory(reg, MemorySize::DWord);
		}
		Memory DWord::operator[](std::uint32_t addr) const noexcept {
			return Memory(addr, MemorySize::DWord);
		}
		Memory DWord::operator[](IncompleteMemory&& addr) const noexcept {
			return { addr.Memory, MemorySize::DWord };
		}
#ifdef SVM_X64
		Memory QWord::operator[](Register reg) const noexcept {
			return Memory(reg, MemorySize::QWord);
		}
		Memory QWord::operator[](std::uint32_t addr) const noexcept {
			return Memory(addr, MemorySize::QWord);
		}
		Memory QWord::operator[](IncompleteMemory&& addr) const noexcept {
			return { addr.Memory, MemorySize::QWord };
		}
#endif
	}
	const detail::Byte Byte;
	const detail::DWord DWord;
#ifdef SVM_X64
	const detail::QWord QWord;
#endif
}

#endif