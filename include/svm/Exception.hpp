#pragma once

#include <svm/Function.hpp>
#include <svm/Instruction.hpp>

#include <cstdint>
#include <string>

namespace svm {
	struct InterpreterException final {
		svm::Function* Function = nullptr;
		svm::Instructions* Instructions = nullptr;
		std::uint64_t InstructionIndex = 0;

		std::uint32_t Code = 0;
	};
}