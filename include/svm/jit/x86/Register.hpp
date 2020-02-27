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

	using Register = const RegisterData*;
}

#include <svm/jit/x86/Registers.hpp>

#endif