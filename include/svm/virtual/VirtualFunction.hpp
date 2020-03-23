#pragma once

#include <svm/virtual/VirtualObject.hpp>
#include <svm/virtual/VirtualStack.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace svm {
	class VirtualFunction final {
	private:
		std::uint16_t m_Arity = 0;
		bool m_HasResult = false;
		std::function<VirtualObject(VirtualStack&)> m_Function;

	public:
		VirtualFunction() noexcept = default;
		template<typename F>
		VirtualFunction(std::uint16_t arity, bool hasResult, F&& function)
			noexcept(noexcept(std::is_nothrow_constructible_v<decltype(m_Function), decltype(std::forward<F>(function))>));
		VirtualFunction(VirtualFunction&& function) noexcept;
		~VirtualFunction() = default;

	public:
		VirtualFunction& operator=(VirtualFunction&& function) noexcept;
		bool operator==(const VirtualFunction&) = delete;
		bool operator!=(const VirtualFunction&) = delete;
		VirtualObject operator()(VirtualStack stack) const;

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;
	};

	using VirtualFunctions = std::vector<VirtualFunction>;
}

#include "detail/impl/VirtualFunction.hpp"