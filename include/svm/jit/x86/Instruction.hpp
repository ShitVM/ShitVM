#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_X86

#include <cstdint>
#include <optional>

namespace svm::jit::x86 {
	union REX final {
		std::uint8_t Byte = 0b0100'0000;
		struct {
			std::uint8_t B : 1;
			std::uint8_t X : 1;
			std::uint8_t R : 1;
			std::uint8_t W : 1;
		} Fields;
	};

	union ModRM final {
		std::uint8_t Byte = 0;
		struct {
			std::uint8_t RM : 3;
			std::uint8_t Reg : 3;
			std::uint8_t Mod : 2;
		} Fields;
	};

	union SIB final {
		std::uint8_t Byte = 0;
		struct {
			std::uint8_t Base : 3;
			std::uint8_t Index : 3;
			std::uint8_t Scale : 2;
		} Fields;
	};

	union DispImm final {
		struct {
			std::uint32_t Displacement;
			std::uint32_t Immediate;
		} Fields;
		std::uint64_t Displacement64 = 0;
		std::uint64_t Immediate64;
	};

	struct Instruction final {
		std::optional<std::uint8_t> LegacyPrefixGroup1;
		std::optional<std::uint8_t> LegacyPrefixGroup2;
		std::optional<std::uint8_t> LegacyPrefixGroup3;
		std::optional<std::uint8_t> LegacyPrefixGroup4;

		std::optional<REX> REXPrefix;
		std::optional<std::uint8_t> OpCodePrefix1;
		std::optional<std::uint8_t> OpCodePrefix2;
		std::uint8_t OpCode;
		std::optional<x86::ModRM> ModRM;
		std::optional<x86::SIB> SIB;

		x86::DispImm DispImm;
		std::uint8_t DisplacementSize = 0;
		std::uint8_t ImmediateSize = 0;
	};
}

#endif