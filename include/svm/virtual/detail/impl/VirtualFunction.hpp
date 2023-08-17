#pragma once
#include <svm/virtual/VirtualFunction.hpp>

namespace svm {
	template<typename F>
	VirtualFunctionInfo::VirtualFunctionInfo(std::string name, std::uint16_t arity, bool hasResult, F&& function)
		noexcept(noexcept(std::is_nothrow_constructible_v<decltype(m_Function), decltype(std::forward<F>(function))>))
		: core::VirtualFunctionInfo(std::move(name), arity, hasResult), m_Function(std::forward<F>(function)) {}
}