#pragma once

#include <svm/Structure.hpp>
#include <svm/virtual/VirtualObject.hpp>
#include <svm/virtual/VirtualStack.hpp>

#include <cstdint>
#include <optional>

namespace svm {
	class Interpreter;

	class VirtualContext final {
	private:
		Interpreter& m_Interpreter;
		VirtualStack& m_Stack;

	public:
		VirtualContext(Interpreter& interpreter, VirtualStack& stack) noexcept;
		VirtualContext(const VirtualContext&) = delete;
		~VirtualContext() = default;

	public:
		VirtualContext& operator=(const VirtualContext&) = delete;
		bool operator==(const VirtualContext&) = delete;
		bool operator!=(const VirtualContext&) = delete;

	public:
		Structure GetStructure(std::uint32_t structure);

		VirtualObject GetField(const VirtualObject& structure, std::uint32_t index);
		VirtualObject GetElement(const VirtualObject& array, std::uint64_t index);

		VirtualObject GetParameter(std::uint16_t index);

		VirtualObject PushFundamental(const Object& object, std::uint64_t count = 0);
		VirtualObject PushStructure(Structure structure, std::uint64_t count = 0);
		bool Pop();
	};
}