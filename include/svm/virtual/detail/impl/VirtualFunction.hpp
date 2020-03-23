#pragma once
#include <svm/virtual/VirtualFunction.hpp>

namespace svm {
	template<typename F>
	VirtualFunction::VirtualFunction(std::uint16_t arity, bool hasResult, F&& function)
		noexcept(noexcept(std::is_nothrow_constructible_v<decltype(m_Function), decltype(std::forward<F>(function))>))
		: m_Arity(arity), m_HasResult(hasResult), m_Function(std::forward<F>(function)) {}
}