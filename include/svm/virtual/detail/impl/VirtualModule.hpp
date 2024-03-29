#pragma once
#include <svm/virtual/VirtualModule.hpp>

namespace svm {
	template<typename F>
	void VirtualModule::AddFunction(std::string name, std::uint16_t arity, bool hasResult, F&& function) {
		GetFunctions().emplace_back(std::move(name), arity, hasResult, std::forward<F>(function));
	}
}