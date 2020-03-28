#pragma once

#include <svm/core/virtual/VirtualFunction.hpp>
#include <svm/virtual/VirtualObject.hpp>
#include <svm/virtual/VirtualStack.hpp>

#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace svm {
	class VirtualFunctionInfo final : public core::VirtualFunctionInfo {
	private:
		std::function<VirtualObject(VirtualStack&)> m_Function;

	public:
		VirtualFunctionInfo() noexcept = default;
		template<typename F>
		VirtualFunctionInfo(std::uint16_t arity, bool hasResult, F&& function)
			noexcept(noexcept(std::is_nothrow_constructible_v<decltype(m_Function), decltype(std::forward<F>(function))>));
		inline VirtualFunctionInfo(VirtualFunctionInfo&& functionInfo) noexcept;
		~VirtualFunctionInfo() = default;

	public:
		inline VirtualFunctionInfo& operator=(VirtualFunctionInfo&& functionInfo) noexcept;
		bool operator==(const VirtualFunctionInfo&) = delete;
		bool operator!=(const VirtualFunctionInfo&) = delete;
		inline VirtualObject operator()(VirtualStack& stack) const;
	};
}

namespace svm {
	using VirtualFunction = core::VirtualFunction<VirtualFunctionInfo>;
	using VirtualFunctions = core::VirtualFunctions<VirtualFunctionInfo>;
}

#include "detail/impl/VirtualFunction.hpp"