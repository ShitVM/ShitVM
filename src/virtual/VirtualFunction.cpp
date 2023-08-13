#include <svm/virtual/VirtualFunction.hpp>

namespace svm {
	VirtualFunctionInfo::VirtualFunctionInfo(VirtualFunctionInfo&& functionInfo) noexcept
		: core::VirtualFunctionInfo(std::move(functionInfo)), m_Function(std::move(functionInfo.m_Function)) {}

	VirtualFunctionInfo& VirtualFunctionInfo::operator=(VirtualFunctionInfo&& functionInfo) noexcept {
		core::VirtualFunctionInfo::operator=(std::move(functionInfo));

		m_Function = std::move(functionInfo.m_Function);
		return *this;
	}
	void VirtualFunctionInfo::operator()(VirtualContext& context) const {
		m_Function(context);
	}
}