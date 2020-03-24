#include <svm/Module.hpp>

#include <cassert>
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

	void ModuleInfo::Clear() noexcept {
		Module = std::monostate();
	}
	bool ModuleInfo::IsEmpty() const noexcept {
		return std::holds_alternative<std::monostate>(Module);
	}
	bool ModuleInfo::IsVirtual() const noexcept {
		return std::holds_alternative<VirtualModule>(Module);
	}

	std::string_view svm::ModuleInfo::GetPath() const noexcept {
		assert(!IsEmpty());

		if (IsVirtual()) return std::get<VirtualModule>(Module).GetPath();
		else return std::get<ByteFile>(Module).GetPath();
	}
	Structure ModuleInfo::GetStructure(std::uint32_t index) const noexcept {
		assert(!IsEmpty());

		if (IsVirtual()) return std::get<VirtualModule>(Module).GetStructures()[index];
		else return std::get<ByteFile>(Module).GetStructures()[index];
	}
	std::uint32_t ModuleInfo::GetStructureCount() const noexcept {
		assert(!IsEmpty());

		if (IsVirtual()) return std::get<VirtualModule>(Module).GetStructures().GetStructureCount();
		else return std::get<ByteFile>(Module).GetStructures().GetStructureCount();
	}
	std::variant<const Function*, const VirtualFunction*> ModuleInfo::GetFunction(std::uint32_t index) const noexcept {
		assert(!IsEmpty());

		if (IsVirtual()) return &std::get<VirtualModule>(Module).GetFunctions()[index];
		else return &std::get<ByteFile>(Module).GetFunctions()[index];
	}
	std::uint32_t ModuleInfo::GetFunctionCount() const noexcept {
		assert(!IsEmpty());

		if (IsVirtual()) return static_cast<std::uint32_t>(std::get<VirtualModule>(Module).GetFunctions().size());
		else return static_cast<std::uint32_t>(std::get<ByteFile>(Module).GetFunctions().size());
	}
}