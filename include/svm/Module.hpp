#pragma once

#include <svm/core/Module.hpp>
#include <svm/virtual/VirtualFunction.hpp>

namespace svm {
	using ModuleInfo = core::ModuleInfo<VirtualFunctionInfo>;
	using Module = core::Module<VirtualFunctionInfo>;
	using Modules = core::Modules<VirtualFunctionInfo>;
}