#pragma once

#include <svm/Structure.hpp>
#include <svm/Type.hpp>
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
		bool Expand(std::size_t size);
		bool Pop();

		VirtualObject GetParameter(std::uint16_t index) noexcept;

	private:
		void InitObject(Object* object, Type type);
	};
}