#include <svm/Module.hpp>

#include <utility>

namespace svm {
	ModuleInfo::ModuleInfo(ModuleInfo&& moduleInfo) noexcept
		: Module(std::move(moduleInfo.Module)) {}

	ModuleInfo& ModuleInfo::operator=(ModuleInfo&& moduleInfo) noexcept {
		Module = std::move(moduleInfo.Module);
		return *this;
	}
}