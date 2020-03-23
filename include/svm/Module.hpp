#pragma once

#include <svm/ByteFile.hpp>
#include <svm/virtual/VirtualModule.hpp>
#include <svm/detail/ReferenceWrapper.hpp>

#include <variant>

namespace svm {
	class ModuleInfo final {
	public:
		std::variant<std::monostate, ByteFile, VirtualModule> Module;

	public:
		ModuleInfo() noexcept = default;
		ModuleInfo(ByteFile&& byteFile) noexcept;
		ModuleInfo(VirtualModule&& virtualModule) noexcept;
		ModuleInfo(ModuleInfo&& moduleInfo) noexcept;
		~ModuleInfo() = default;

	public:
		ModuleInfo& operator=(ByteFile&& byteFile) noexcept;
		ModuleInfo& operator=(VirtualModule&& virtualModule) noexcept;
		ModuleInfo& operator=(ModuleInfo&& moduleInfo) noexcept;
		bool operator==(const ModuleInfo&) = delete;
		bool operator!=(const ModuleInfo&) = delete;
	};
}

namespace svm {
	class Module final : public detail::ReferenceWrapper<ModuleInfo> {
	public:
		using detail::ReferenceWrapper<ModuleInfo>::ReferenceWrapper;
	};
}