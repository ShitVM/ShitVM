#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_X86

#include <svm/jit/x86/Instruction.hpp>
#include <svm/jit/x86/Register.hpp>

#include <cstdint>
#include <optional>
#include <variant>

namespace svm::jit::x86 {
	enum class AddressingMode {
		None,

		RegisterIndirect,
		DirectMemory,
	};

	using MemorySize = RegisterSize;

	class Address final {
	private:
		std::variant<std::monostate, Register, std::uint32_t> m_Base;
		std::uint8_t m_Scale = 0;
		std::optional<Register> m_Index;
		std::variant<std::monostate, std::uint8_t, std::uint32_t> m_Displacement;
		MemorySize m_Size;

	public:
		Address() noexcept = default;
		Address(Register base, MemorySize size) noexcept;
		Address(Register base, std::uint8_t disp, MemorySize size) noexcept;
		Address(Register base, std::uint32_t disp, MemorySize size) noexcept;
		Address(Register base, std::uint8_t scale, Register index, MemorySize size) noexcept;
		Address(Register base, std::uint8_t scale, Register index, std::uint8_t disp, MemorySize size) noexcept;
		Address(Register base, std::uint8_t scale, Register index, std::uint32_t disp, MemorySize size) noexcept;
		Address(std::uint32_t base, MemorySize size) noexcept;
		Address(const Address& address) noexcept;
		Address(const Address& address, MemorySize size) noexcept;
		~Address() = default;

	public:
		Address& operator=(const Address& address) noexcept;
		bool operator==(const Address&) = delete;
		bool operator!=(const Address&) = delete;

	public:
		AddressingMode GetMode() const noexcept;
		bool IsBaseRegister() const noexcept;
		bool IsBaseAddress() const noexcept;
		Register GetBaseRegister() const noexcept;
		std::uint32_t GetBaseAddress() const noexcept;
		void SetBase(Register newBase) noexcept;
		void SetBase(std::uint32_t newBase) noexcept;

		bool HasSIB() const noexcept;
		void RemoveSBI() noexcept;
		std::uint8_t GetScale() const noexcept;
		void SetScale(std::uint8_t newScale) noexcept;
		Register GetIndex() const noexcept;
		void SetIndex(Register newIndex) noexcept;
		bool HasDisplacement() const noexcept;
		void RemoveDisplacement() noexcept;
		bool IsDisplacementByte() const noexcept;
		bool IsDisplacementDWord() const noexcept;
		std::uint8_t GetDisplacement8() const noexcept;
		std::uint32_t GetDisplacement32() const noexcept;
		void SetDisplacement(std::uint8_t newDisplacement) noexcept;
		void SetDisplacement(std::uint32_t newDisplacement) noexcept;

		MemorySize GetSize() const noexcept;
		void SetSize(MemorySize newSize) noexcept;
	};
}

namespace svm::jit::x86 {
	namespace detail {
		struct IncompleteAddress final {
			x86::Address Address;
		};
	}

	detail::IncompleteAddress operator*(Register index, std::uint8_t scale) noexcept;
	detail::IncompleteAddress operator*(std::uint8_t scale, Register index) noexcept;
	detail::IncompleteAddress operator+(Register base, std::uint8_t disp) noexcept;
	detail::IncompleteAddress operator+(Register base, std::uint32_t disp) noexcept;
	detail::IncompleteAddress operator+(std::uint8_t disp, Register base) noexcept;
	detail::IncompleteAddress operator+(std::uint32_t disp, Register base) noexcept;

	detail::IncompleteAddress operator+(Register base, detail::IncompleteAddress&& other) noexcept;
	detail::IncompleteAddress operator+(detail::IncompleteAddress&& other, Register base) noexcept;
	detail::IncompleteAddress operator+(std::uint8_t disp, detail::IncompleteAddress&& other) noexcept;
	detail::IncompleteAddress operator+(std::uint32_t disp, detail::IncompleteAddress&& other) noexcept;
	detail::IncompleteAddress operator+(detail::IncompleteAddress&& other, std::uint8_t disp) noexcept;
	detail::IncompleteAddress operator+(detail::IncompleteAddress&& other, std::uint32_t disp) noexcept;
	detail::IncompleteAddress operator+(detail::IncompleteAddress&& a, detail::IncompleteAddress&& b) noexcept;

	constexpr std::uint8_t operator""_b(unsigned long long value) noexcept {
		return static_cast<std::uint8_t>(value);
	}
}

namespace svm::jit::x86 {
	namespace detail {
		struct DWordPtr final {
			Address operator[](Register reg) const noexcept;
			Address operator[](std::uint32_t addr) const noexcept;
			Address operator[](IncompleteAddress&& addr) const noexcept;
		};
		struct QWordPtr final {
			Address operator[](Register reg) const noexcept;
			Address operator[](std::uint32_t addr) const noexcept;
			Address operator[](IncompleteAddress&& addr) const noexcept;
		};
	}
	extern const detail::DWordPtr DWordPtr;
	extern const detail::QWordPtr QWordPtr;
}

#endif