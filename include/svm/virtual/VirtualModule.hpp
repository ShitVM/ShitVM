#pragma once

#include <svm/core/virtual/VirtualModule.hpp>
#include <svm/virtual/VirtualFunction.hpp>

namespace svm {
	class VirtualModule final : public core::VirtualModule<VirtualFunctionInfo> {
	public:
		using core::VirtualModule<VirtualFunctionInfo>::VirtualModule;
	};
}