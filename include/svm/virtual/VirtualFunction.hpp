#pragma once

#include <svm/core/virtual/VirtualFunction.hpp>
#include <svm/virtual/VirtualContext.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace svm {
	class VirtualFunctionInfo final : public core::VirtualFunctionInfo {
	private:
		std::function<void(VirtualContext&)> m_Function;

	public:
		VirtualFunctionInfo() noexcept = default;
		template<typename F>
		VirtualFunctionInfo(std::string name, std::uint16_t arity, bool hasResult, F&& function)
			noexcept(noexcept(std::is_nothrow_constructible_v<decltype(m_Function), decltype(std::forward<F>(function))>));
		VirtualFunctionInfo(VirtualFunctionInfo&& functionInfo) noexcept;
		~VirtualFunctionInfo() = default;

	public:
		VirtualFunctionInfo& operator=(VirtualFunctionInfo&& functionInfo) noexcept;
		bool operator==(const VirtualFunctionInfo&) = delete;
		bool operator!=(const VirtualFunctionInfo&) = delete;
		void operator()(VirtualContext& context) const;
	};
}

namespace svm {
	using VirtualFunction = core::VirtualFunction<VirtualFunctionInfo>;
	using VirtualFunctions = core::VirtualFunctions<VirtualFunctionInfo>;
}

#include "detail/impl/VirtualFunction.hpp"