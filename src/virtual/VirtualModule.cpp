#include <svm/virtual/VirtualModule.hpp>

#include <utility>

namespace svm {
	VirtualModule::VirtualModule(std::string path) noexcept
		: m_Path(std::move(path)) {}
	VirtualModule::VirtualModule(std::string path, Structures&& structures, VirtualFunctions&& functions) noexcept
		: m_Path(std::move(path)), m_Structures(std::move(structures)), m_Functions(std::move(functions)) {}
	VirtualModule::VirtualModule(VirtualModule&& module) noexcept
		: m_Structures(std::move(module.m_Structures)), m_Functions(std::move(module.m_Functions)) {}

	VirtualModule& VirtualModule::operator=(VirtualModule&& module) noexcept {
		m_Path = std::move(module.m_Path);
		m_Structures = std::move(module.m_Structures);
		m_Functions = std::move(module.m_Functions);
		return *this;
	}

	void VirtualModule::Clear() noexcept {
		m_Path.clear();
		m_Structures.Clear();
		m_Functions.clear();
	}
	bool VirtualModule::IsEmpty() const noexcept {
		return m_Path.empty() && m_Structures.IsEmpty() && m_Functions.empty();
	}

	std::string_view svm::VirtualModule::GetPath() const noexcept {
		return m_Path;
	}
	const Structures& VirtualModule::GetStructures() const noexcept {
		return m_Structures;
	}
	const VirtualFunctions& VirtualModule::GetFunctions() const noexcept {
		return m_Functions;
	}
}