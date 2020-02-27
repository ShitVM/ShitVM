#include <svm/jit/x86/Address.hpp>

#ifdef SVM_X86

#include <cassert>
#include <limits>

namespace svm::jit::x86 {
	Address::Address(Register base, MemorySize size) noexcept
		: m_Base(base), m_Size(size) {}
	Address::Address(Register base, std::uint8_t disp, MemorySize size) noexcept
		: m_Base(base), m_Displacement(disp), m_Size(size) {}
	Address::Address(Register base, std::uint32_t disp, MemorySize size) noexcept
		: m_Base(base), m_Displacement(disp), m_Size(size) {}
	Address::Address(Register base, std::uint8_t scale, Register index, MemorySize size) noexcept
		: m_Base(base), m_Scale(scale), m_Index(index), m_Size(size) {}
	Address::Address(Register base, std::uint8_t scale, Register index, std::uint8_t disp, MemorySize size) noexcept
		: m_Base(base), m_Scale(scale), m_Index(index), m_Displacement(disp), m_Size(size) {}
	Address::Address(Register base, std::uint8_t scale, Register index, std::uint32_t disp, MemorySize size) noexcept
		: m_Base(base), m_Scale(scale), m_Index(index), m_Displacement(disp), m_Size(size) {}
	Address::Address(std::uint32_t base, MemorySize size) noexcept
		: m_Base(base), m_Size(size) {}
	Address::Address(const Address& address) noexcept
		: m_Base(address.m_Base), m_Scale(address.m_Scale), m_Index(address.m_Index), m_Displacement(address.m_Displacement), m_Size(address.m_Size) {}
	Address::Address(const Address& address, MemorySize size) noexcept
		: m_Base(address.m_Base), m_Scale(address.m_Scale), m_Index(address.m_Index), m_Displacement(address.m_Displacement), m_Size(size) {}

	Address& Address::operator=(const Address& address) noexcept {
		m_Base = address.m_Base;
		m_Scale = address.m_Scale;
		m_Index = address.m_Index;
		m_Displacement = address.m_Displacement;
		return *this;
	}
	AddressingMode Address::GetMode() const noexcept {
		return static_cast<AddressingMode>(m_Base.index());
	}
	bool Address::IsBaseRegister() const noexcept {
		return std::holds_alternative<Register>(m_Base);
	}
	bool Address::IsBaseAddress() const noexcept {
		return std::holds_alternative<std::uint32_t>(m_Base);
	}
	Register Address::GetBaseRegister() const noexcept {
		return std::get<Register>(m_Base);
	}
	std::uint32_t Address::GetBaseAddress() const noexcept {
		return std::get<std::uint32_t>(m_Base);
	}
	void Address::SetBase(Register newBase) noexcept {
		m_Base = newBase;
	}
	void Address::SetBase(std::uint32_t newBase) noexcept {
		m_Base = newBase;
	}

	bool Address::HasSIB() const noexcept {
		return m_Scale != 0;
	}
	void Address::RemoveSBI() noexcept {
		m_Scale = 0;
		m_Index.reset();
	}
	std::uint8_t Address::GetScale() const noexcept {
		return m_Scale;
	}
	void Address::SetScale(std::uint8_t newScale) noexcept {
		m_Scale = newScale;
	}
	Register Address::GetIndex() const noexcept {
		return m_Index.value();
	}
	void Address::SetIndex(Register newIndex) noexcept {
		m_Index = newIndex;
	}
	bool Address::HasDisplacement() const noexcept {
		return !std::holds_alternative<std::monostate>(m_Displacement);
	}
	void Address::RemoveDisplacement() noexcept {
		m_Displacement = std::monostate();
	}
	bool Address::IsDisplacementByte() const noexcept {
		return std::holds_alternative<std::uint8_t>(m_Displacement);
	}
	bool Address::IsDisplacementDWord() const noexcept {
		return std::holds_alternative<std::uint32_t>(m_Displacement);
	}
	std::uint8_t Address::GetDisplacement8() const noexcept {
		return std::get<std::uint8_t>(m_Displacement);
	}
	std::uint32_t Address::GetDisplacement32() const noexcept {
		return std::get<std::uint32_t>(m_Displacement);
	}
	void Address::SetDisplacement(std::uint8_t newDisplacement) noexcept {
		m_Displacement = newDisplacement;
	}
	void Address::SetDisplacement(std::uint32_t newDisplacement) noexcept {
		m_Displacement = newDisplacement;
	}

	MemorySize Address::GetSize() const noexcept {
		return m_Size;
	}
	void Address::SetSize(MemorySize newSize) noexcept {
		m_Size = newSize;
	}
}

namespace svm::jit::x86 {
	detail::IncompleteAddress operator*(Register index, std::uint8_t scale) noexcept {
		Address addr;
		addr.SetScale(scale);
		addr.SetIndex(index);
		return { addr };
	}
	detail::IncompleteAddress operator*(std::uint8_t scale, Register index) noexcept {
		return index * scale;
	}
	detail::IncompleteAddress operator+(Register base, std::uint8_t disp) noexcept {
		Address addr;
		addr.SetBase(base);
		addr.SetDisplacement(disp);
		return { addr };
	}
	detail::IncompleteAddress operator+(Register base, std::uint32_t disp) noexcept {
		Address addr;
		addr.SetBase(base);
		addr.SetDisplacement(disp);
		return { addr };
	}
	detail::IncompleteAddress operator+(std::uint8_t disp, Register base) noexcept {
		return base + disp;
	}
	detail::IncompleteAddress operator+(std::uint32_t disp, Register base) noexcept {
		return base + disp;
	}

	detail::IncompleteAddress operator+(Register base, detail::IncompleteAddress&& other) noexcept {
		assert(other.Address.GetMode() != AddressingMode::RegisterIndirect);
		other.Address.SetBase(base);
		return other;
	}
	detail::IncompleteAddress operator+(detail::IncompleteAddress&& other, Register base) noexcept {
		return base + std::move(other);
	}
	detail::IncompleteAddress operator+(std::uint8_t disp, detail::IncompleteAddress&& other) noexcept {
		if (other.Address.IsDisplacementByte()) {
			const int temp = other.Address.GetDisplacement8() + disp;
			if (temp > std::numeric_limits<std::uint8_t>::max()) {
				other.Address.SetDisplacement(static_cast<std::uint32_t>(temp));
			} else {
				other.Address.SetDisplacement(static_cast<std::uint8_t>(temp));
			}
		} else if (other.Address.IsDisplacementDWord()) {
			other.Address.SetDisplacement(other.Address.GetDisplacement32() + disp);
		} else {
			other.Address.SetDisplacement(disp);
		}
		return other;
	}
	detail::IncompleteAddress operator+(std::uint32_t disp, detail::IncompleteAddress&& other) noexcept {
		if (other.Address.IsDisplacementByte()) {
			const std::uint32_t temp = other.Address.GetDisplacement8() + disp;
			if (temp > std::numeric_limits<std::uint8_t>::max()) {
				other.Address.SetDisplacement(temp);
			} else {
				other.Address.SetDisplacement(static_cast<std::uint8_t>(temp));
			}
		} else if (other.Address.IsDisplacementDWord()) {
			other.Address.SetDisplacement(other.Address.GetDisplacement32() + disp);
		} else {
			other.Address.SetDisplacement(disp);
		}
		return other;
	}
	detail::IncompleteAddress operator+(detail::IncompleteAddress&& other, std::uint8_t disp) noexcept {
		return disp + std::move(other);
	}
	detail::IncompleteAddress operator+(detail::IncompleteAddress&& other, std::uint32_t disp) noexcept {
		return disp + std::move(other);
	}
	detail::IncompleteAddress operator+(detail::IncompleteAddress&& a, detail::IncompleteAddress&& b) noexcept {
		assert(a.Address.GetMode() == AddressingMode::None || b.Address.GetMode() == AddressingMode::None);
		assert(!a.Address.HasSIB() || !b.Address.HasSIB());
		assert(!a.Address.HasDisplacement() || !b.Address.HasDisplacement());

		if (a.Address.GetMode() == AddressingMode::None) {
			if (b.Address.GetMode() == AddressingMode::RegisterIndirect) {
				a.Address.SetBase(b.Address.GetBaseRegister());
			} else if (b.Address.GetMode() != AddressingMode::DirectMemory) {
				a.Address.SetBase(b.Address.GetBaseAddress());
			}
		}

		if (!a.Address.HasSIB()) {
			a.Address.SetScale(b.Address.GetScale());
			a.Address.SetIndex(b.Address.GetIndex());
		}

		if (!a.Address.HasDisplacement()) {
			if (b.Address.IsDisplacementByte()) {
				a.Address.SetDisplacement(b.Address.GetDisplacement8());
			} else if (b.Address.IsDisplacementDWord()) {
				a.Address.SetDisplacement(b.Address.GetDisplacement32());
			}
		}

		return a;
	}
}

namespace svm::jit::x86 {
	namespace detail {
		Address DWordPtr::operator[](Register reg) const noexcept {
			return Address(reg, MemorySize::DWord);
		}
		Address DWordPtr::operator[](std::uint32_t addr) const noexcept {
			return Address(addr, MemorySize::DWord);
		}
		Address DWordPtr::operator[](IncompleteAddress&& addr) const noexcept {
			return { addr.Address, MemorySize::DWord };
		}
		Address QWordPtr::operator[](Register reg) const noexcept {
			return Address(reg, MemorySize::QWord);
		}
		Address QWordPtr::operator[](std::uint32_t addr) const noexcept {
			return Address(addr, MemorySize::QWord);
		}
		Address QWordPtr::operator[](IncompleteAddress&& addr) const noexcept {
			return { addr.Address, MemorySize::QWord };
		}
	}
	const detail::DWordPtr DWordPtr;
	const detail::QWordPtr QWordPtr;
}

#endif