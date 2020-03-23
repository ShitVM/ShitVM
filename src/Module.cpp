#include <svm/Module.hpp>

#include <utility>

namespace svm {
	ModuleInfo::ModuleInfo(ByteFile&& byteFile) noexcept
		: Module(std::move(byteFile)) {}
	ModuleInfo::ModuleInfo(VirtualModule&& virtualModule) noexcept
		: Module(std::move(virtualModule)) {}
	ModuleInfo::ModuleInfo(ModuleInfo&& moduleInfo) noexcept
		: Module(std::move(moduleInfo.Module)) {}

	ModuleInfo& ModuleInfo::operator=(ByteFile&& byteFile) noexcept {
		Module = std::move(byteFile);
		return *this;
	}
	ModuleInfo& ModuleInfo::operator=(VirtualModule&& virtualModule) noexcept {
		virtualModule = std::move(virtualModule);
		return *this;
	}
	ModuleInfo& ModuleInfo::operator=(ModuleInfo&& moduleInfo) noexcept {
		Module = std::move(moduleInfo.Module);
		return *this;
	}
}