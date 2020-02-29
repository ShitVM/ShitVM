#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_X86

#include <cstdint>

namespace svm::jit::x86 {
	enum class RegisterType : std::uint8_t {
		None,

		General,
	};

	enum class RegisterSize : std::uint8_t {
		None,

		Byte,
		DWord,
		QWord,
	};

	class RegisterData final {
	public:
		const char* Name = nullptr;
		RegisterType Type = RegisterType::None;
		RegisterSize Size = RegisterSize::None;
		std::uint8_t Code = 0;

	public:
		RegisterData() noexcept = default;
		RegisterData(const char* name, RegisterType type, RegisterSize size, std::uint8_t code) noexcept;
		RegisterData(const RegisterData&) = delete;
		~RegisterData() = default;

	public:
		RegisterData& operator=(const RegisterData&) = delete;
		bool operator==(const RegisterData&) = delete;
		bool operator!=(const RegisterData&) = delete;
	};
}

namespace svm::jit::x86 {
	class Register final {
	private:
		const RegisterData* m_Data = nullptr;

	public:
		Register() noexcept = default;
		Register(std::nullptr_t) noexcept;
		Register(const RegisterData* data) noexcept;
		Register(const Register& reg) noexcept;
		~Register() = default;

	public:
		Register& operator=(std::nullptr_t) noexcept;
		Register& operator=(const RegisterData* data) noexcept;
		Register& operator=(const Register& reg) noexcept;
		bool operator==(const Register& reg) const noexcept;
		bool operator!=(const Register& reg) const noexcept;

	public:
		const RegisterData& operator*() const noexcept;
		const RegisterData* operator->() const noexcept;
	};
}

#include <svm/jit/x86/Registers.hpp>

#endif