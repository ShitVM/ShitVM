#pragma once
#include <svm/virtual/VirtualFunction.hpp>

namespace svm {
	template<typename F>
	VirtualFunctionInfo::VirtualFunctionInfo(std::uint16_t arity, bool hasResult, F&& function)
		noexcept(noexcept(std::is_nothrow_constructible_v<decltype(m_Function), decltype(std::forward<F>(function))>))
		: core::VirtualFunctionInfo(arity, hasResult), m_Function(std::forward<F>(function)) {}
	inline VirtualFunctionInfo::VirtualFunctionInfo(VirtualFunctionInfo&& functionInfo) noexcept
		: core::VirtualFunctionInfo(std::move(functionInfo)), m_Function(std::move(functionInfo.m_Function)) {}

	inline VirtualFunctionInfo& VirtualFunctionInfo::operator=(VirtualFunctionInfo&& functionInfo) noexcept {
		core::VirtualFunctionInfo::operator=(std::move(functionInfo));

		m_Function = std::move(functionInfo.m_Function);
		return *this;
	}
	inline VirtualObject VirtualFunctionInfo::operator()(VirtualStack stack) const {
		return m_Function(stack);
	}
}