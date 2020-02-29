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

	class Memory final {
	private:
		std::variant<std::monostate, Register, std::uint32_t> m_Base;
		std::uint8_t m_Scale = 0;
		std::optional<Register> m_Index;
		std::variant<std::monostate, std::uint8_t, std::uint32_t> m_Displacement;
		MemorySize m_Size;

	public:
		Memory() noexcept = default;
		Memory(Register base, MemorySize size) noexcept;
		Memory(Register base, std::uint8_t disp, MemorySize size) noexcept;
		Memory(Register base, std::uint32_t disp, MemorySize size) noexcept;
		Memory(Register base, std::uint8_t scale, Register index, MemorySize size) noexcept;
		Memory(Register base, std::uint8_t scale, Register index, std::uint8_t disp, MemorySize size) noexcept;
		Memory(Register base, std::uint8_t scale, Register index, std::uint32_t disp, MemorySize size) noexcept;
		Memory(std::uint32_t base, MemorySize size) noexcept;
		Memory(const Memory& address) noexcept;
		Memory(const Memory& address, MemorySize size) noexcept;
		~Memory() = default;

	public:
		Memory& operator=(const Memory& address) noexcept;
		bool operator==(const Memory&) = delete;
		bool operator!=(const Memory&) = delete;

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
		std::uint8_t GetDisplacementByte() const noexcept;
		std::uint32_t GetDisplacementDWord() const noexcept;
		void SetDisplacement(std::uint8_t newDisplacement) noexcept;
		void SetDisplacement(std::uint32_t newDisplacement) noexcept;

		MemorySize GetSize() const noexcept;
		void SetSize(MemorySize newSize) noexcept;
	};
}

namespace svm::jit::x86 {
	namespace detail {
		struct IncompleteMemory final {
			x86::Memory Memory;
		};
	}

	detail::IncompleteMemory operator*(Register index, std::uint8_t scale) noexcept;
	detail::IncompleteMemory operator*(std::uint8_t scale, Register index) noexcept;
	detail::IncompleteMemory operator+(Register base, std::uint8_t disp) noexcept;
	detail::IncompleteMemory operator+(Register base, std::uint32_t disp) noexcept;
	detail::IncompleteMemory operator+(std::uint8_t disp, Register base) noexcept;
	detail::IncompleteMemory operator+(std::uint32_t disp, Register base) noexcept;

	detail::IncompleteMemory operator+(Register base, detail::IncompleteMemory&& other) noexcept;
	detail::IncompleteMemory operator+(detail::IncompleteMemory&& other, Register base) noexcept;
	detail::IncompleteMemory operator+(std::uint8_t disp, detail::IncompleteMemory&& other) noexcept;
	detail::IncompleteMemory operator+(std::uint32_t disp, detail::IncompleteMemory&& other) noexcept;
	detail::IncompleteMemory operator+(detail::IncompleteMemory&& other, std::uint8_t disp) noexcept;
	detail::IncompleteMemory operator+(detail::IncompleteMemory&& other, std::uint32_t disp) noexcept;
	detail::IncompleteMemory operator+(detail::IncompleteMemory&& a, detail::IncompleteMemory&& b) noexcept;

	constexpr std::uint8_t operator""_b(unsigned long long value) noexcept {
		return static_cast<std::uint8_t>(value);
	}
}

namespace svm::jit::x86 {
	namespace detail {
		struct Byte final {
			Memory operator[](Register reg) const noexcept;
			Memory operator[](std::uint32_t addr) const noexcept;
			Memory operator[](IncompleteMemory&& addr) const noexcept;
		};
		struct DWord final {
			Memory operator[](Register reg) const noexcept;
			Memory operator[](std::uint32_t addr) const noexcept;
			Memory operator[](IncompleteMemory&& addr) const noexcept;
		};
#ifdef SVM_X64
		struct QWord final {
			Memory operator[](Register reg) const noexcept;
			Memory operator[](std::uint32_t addr) const noexcept;
			Memory operator[](IncompleteMemory&& addr) const noexcept;
		};
#endif
	}
	extern const detail::Byte Byte;
	extern const detail::DWord DWord;
#ifdef SVM_X64
	extern const detail::QWord QWord;
#endif
}

#endif