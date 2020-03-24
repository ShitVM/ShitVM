#pragma once

#include <svm/Module.hpp>
#include <svm/virtual/VirtualModule.hpp>

#include <string>
#include <vector>

namespace svm {
	class Loader final {
	private:
		std::vector<ModuleInfo> m_Modules;
		std::uint32_t m_StructureCodeOffset = 0;

	public:
		Loader() = default;
		Loader(Loader&& loader) noexcept;
		~Loader() = default;

	public:
		Loader& operator=(Loader&& loader) noexcept;
		bool operator==(const Loader&) = delete;
		bool operator!=(const Loader&) = delete;

	public:
		void Clear() noexcept;
		Module Load(const std::string& path);
		VirtualModule& Create(const std::string& virtualPath);
		void LoadDependencies(Module module);

		Module GetModule(std::uint32_t index) const noexcept;
		Module GetModule(const std::string& path) const noexcept;
		std::uint32_t GetModuleCount() const noexcept;
	};
}