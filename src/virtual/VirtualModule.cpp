#include <svm/virtual/VirtualModule.hpp>

#include <utility>

namespace svm {
	VirtualModule::VirtualModule(Structures&& structures, VirtualFunctions&& functions) noexcept
		: m_Structures(std::move(structures)), m_Functions(std::move(functions)) {}
	VirtualModule::VirtualModule(VirtualModule&& module) noexcept
		: m_Structures(std::move(module.m_Structures)), m_Functions(std::move(module.m_Functions)) {}

	VirtualModule& VirtualModule::operator=(VirtualModule&& module) noexcept {
		m_Structures = std::move(module.m_Structures);
		m_Functions = std::move(module.m_Functions);
		return *this;
	}

	void VirtualModule::Clear() noexcept {
		m_Structures.Clear();
		m_Functions.clear();
	}
	bool VirtualModule::IsEmpty() const noexcept {
		return m_Structures.IsEmpty() && m_Functions.empty();
	}
}