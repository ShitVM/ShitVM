#include <svm/Loader.hpp>

#include <svm/ByteFile.hpp>
#include <svm/Parser.hpp>
#include <svm/detail/FileSystem.hpp>

#include <utility>

namespace svm {
	Loader::Loader(Loader&& loader) noexcept
		: m_Modules(std::move(loader.m_Modules)) {}

	Loader& Loader::operator=(Loader&& loader) noexcept {
		m_Modules = std::move(loader.m_Modules);
		return *this;
	}

	void Loader::Clear() noexcept {
		m_Modules.clear();
	}
	Module Loader::Load(const std::string& path) {
		Parser parser;
		parser.Load(detail::fs::absolute(path).string());
		parser.Parse();

		return m_Modules.emplace_back(parser.GetResult());
	}
	VirtualModule& Loader::Create(const std::string& virtualPath) {
		ModuleInfo& module = m_Modules.emplace_back(VirtualModule(detail::fs::absolute(virtualPath).string()));
		return std::get<VirtualModule>(module.Module);
	}

	Module Loader::GetModule(std::uint32_t index) const noexcept {
		return m_Modules[index];
	}
	std::uint32_t Loader::GetModuleCount() const noexcept {
		return static_cast<std::uint32_t>(m_Modules.size());
	}
}