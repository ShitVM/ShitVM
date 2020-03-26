#pragma once

#include <svm/virtual/VirtualObject.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace svm {
	class Stack;
	struct StackFrame;

	class VirtualStack final {
	private:
		Stack* m_Stack = nullptr;
		const StackFrame* m_StackFrame = nullptr;
		const std::vector<std::size_t>* m_LocalVariables = nullptr;

	public:
		VirtualStack(Stack* stack, const StackFrame* stackFrame, const std::vector<std::size_t>* localVariables) noexcept;
		VirtualStack(const VirtualStack&) = delete;
		~VirtualStack() = default;

	public:
		VirtualStack& operator=(const VirtualStack&) = delete;
		bool operator==(const VirtualStack&) = delete;
		bool operator!=(const VirtualStack&) = delete;

	public:
		bool IsEmpty() const noexcept;
		VirtualObject GetTop() noexcept;
		bool Push(const VirtualObject& object);
		bool Pop();

		VirtualObject Parameter(std::uint16_t index) noexcept;
	};
}