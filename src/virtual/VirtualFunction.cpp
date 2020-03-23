#include <svm/virtual/VirtualFunction.hpp>

namespace svm {
	VirtualFunction::VirtualFunction(VirtualFunction&& function) noexcept
		: m_Arity(function.m_Arity), m_HasResult(function.m_HasResult), m_Function(std::move(function.m_Function)) {}

	VirtualFunction& VirtualFunction::operator=(VirtualFunction&& function) noexcept {
		m_Arity = function.m_Arity;
		m_HasResult = function.m_HasResult;
		m_Function = std::move(function.m_Function);
		return *this;
	}
	VirtualObject VirtualFunction::operator()(VirtualStack stack) const {
		return m_Function(stack);
	}

	void VirtualFunction::Clear() noexcept {
		m_Function = nullptr;
	}
	bool VirtualFunction::IsEmpty() const noexcept {
		return m_Function == nullptr;
	}
}