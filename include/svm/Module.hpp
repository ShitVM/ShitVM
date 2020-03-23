#pragma once

#include <svm/ByteFile.hpp>
#include <svm/VirtualModule.hpp>
#include <svm/detail/ReferenceWrapper.hpp>

#include <variant>

namespace svm {
	class ModuleInfo final {
	public:
		std::variant<std::monostate, ByteFile, VirtualModule> Module;

	public:
		ModuleInfo() noexcept = default;
		ModuleInfo(ModuleInfo&& moduleInfo) noexcept;
		~ModuleInfo() = default;

	public:
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