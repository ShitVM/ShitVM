#pragma once
#include <svm/Macro.hpp>

#ifdef SVM_X86

#include <optional>

namespace svm::jit::x86 {
	union ModRM final {
		std::uint8_t Byte = 0;
		struct {
			std::uint8_t Mod : 2;
			std::uint8_t Reg : 3;
			std::uint8_t RM : 3;
		};
	};

	union SIB final {
		std::uint8_t Byte = 0;
		struct {
			std::uint8_t Scale : 2;
			std::uint8_t Index : 3;
			std::uint8_t Base : 3;
		};
	};

	union DispImm final {
		struct {
			std::uint32_t Displacement;
			std::uint32_t Immediate;
		};
		std::uint64_t Displacement64;
		std::uint64_t Immediate64;
	};

	struct Instruction final {
		std::optional<std::uint8_t> LegacyPrefixGroup1;
		std::optional<std::uint8_t> LegacyPrefixGroup2;
		std::optional<std::uint8_t> LegacyPrefixGroup3;
		std::optional<std::uint8_t> LegacyPrefixGroup4;

		std::optional<std::uint8_t> REXPrefix;
		std::optional<std::uint8_t> OpCodePrefix1;
		std::optional<std::uint8_t> OpCodePrefix2;
		std::optional<std::uint8_t> OpCode;
		std::optional<x86::ModRM> ModRM;
		std::optional<x86::SIB> SIB;

		std::optional<x86::DispImm> DispImm;
	};
}

#endif