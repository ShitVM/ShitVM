#include <svm/Loader.hpp>

#include <svm/ByteFile.hpp>
#include <svm/Parser.hpp>
#include <svm/detail/FileSystem.hpp>

#include <algorithm>
#include <utility>

namespace svm {
	Loader::Loader(Loader&& loader) noexcept
		: m_Modules(std::move(loader.m_Modules)), m_StructureCodeOffset(loader.m_StructureCodeOffset) {}

	Loader& Loader::operator=(Loader&& loader) noexcept {
		m_Modules = std::move(loader.m_Modules);
		m_StructureCodeOffset = loader.m_StructureCodeOffset;
		return *this;
	}

	void Loader::Clear() noexcept {
		m_Modules.clear();
		m_StructureCodeOffset = 0;
	}
	Module Loader::Load(const std::string& path) {
		Parser parser;
		parser.Load(detail::fs::absolute(path).string());
		parser.Parse();

		ByteFile byteFile = parser.GetResult();
		byteFile.UpdateStructureCodes(m_StructureCodeOffset);
		m_StructureCodeOffset += byteFile.GetStructures().GetStructureCount();

		return m_Modules.emplace_back(parser.GetResult());
	}
	VirtualModule& Loader::Create(const std::string& virtualPath) {
		ModuleInfo& module = m_Modules.emplace_back(VirtualModule(detail::fs::absolute(virtualPath).string()));
		return std::get<VirtualModule>(module.Module);
	}
	void Loader::LoadDependencies(Module module) {
		if (!std::holds_alternative<ByteFile>(module->Module)) return;

		const ByteFile& byteFile = std::get<ByteFile>(module->Module);
		const std::vector<std::string>& dependencies = byteFile.GetDependencies();

		for (const std::string& dependency : dependencies) {
			if (GetModule(dependency) != nullptr) continue;

			LoadDependencies(Load(dependency));
		}
	}

	Module Loader::GetModule(std::uint32_t index) const noexcept {
		return m_Modules[index];
	}
	Module Loader::GetModule(const std::string& path) const noexcept {
		const auto iter = std::find_if(m_Modules.begin(), m_Modules.end(), [path](const auto& module) {
			return module.GetPath() == path;
		});
		if (iter == m_Modules.end()) return nullptr;
		else return *iter;
	}
	std::uint32_t Loader::GetModuleCount() const noexcept {
		return static_cast<std::uint32_t>(m_Modules.size());
	}
}