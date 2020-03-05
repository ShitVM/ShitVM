#include <svm/Structure.hpp>

#include <svm/IO.hpp>

#include <utility>

namespace svm {
	StructureInfo::StructureInfo(StructureInfo&& structure) noexcept
		: FieldTypes(std::move(structure.FieldTypes)) {}

	StructureInfo& StructureInfo::operator=(StructureInfo&& structure) noexcept {
		FieldTypes = std::move(structure.FieldTypes);
		return *this;
	}

	std::ostream& operator<<(std::ostream& stream, const StructureInfo& structureInfo) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "Structure: " << structureInfo.Type.Size << "B\n"
			   << defIndent << "\tFields: " << structureInfo.FieldTypes.size();
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(structureInfo.FieldTypes.size()); ++i) {
			stream << '\n' << defIndent << "\t\t[" << i << "]: " << structureInfo.FieldTypes[i]->Name << '(' << structureInfo.FieldTypes[i]->Size << "B)";
		}
		return stream;
	}
}

namespace svm {
	std::ostream& operator<<(std::ostream& stream, const Structure& structure) {
		return stream << structure.GetReference();
	}
}

namespace svm {
	Structures::Structures(std::vector<StructureInfo>&& structures) noexcept
		: m_Structures(std::move(structures)) {}
	Structures::Structures(Structures&& structures) noexcept
		: m_Structures(std::move(structures.m_Structures)) {}

	Structures& Structures::operator=(Structures&& structures) noexcept {
		m_Structures = std::move(structures.m_Structures);
		return *this;
	}
	Structure Structures::operator[](std::uint32_t index) const noexcept {
		return m_Structures[index];
	}

	void Structures::Clear() noexcept {
		m_Structures.clear();
	}
	bool Structures::IsEmpty() const noexcept {
		return m_Structures.empty();
	}

	Structure Structures::Get(std::uint32_t index) const noexcept {
		return m_Structures[index];
	}
	std::uint32_t Structures::GetCount() const noexcept {
		return static_cast<std::uint32_t>(m_Structures.size());
	}

	std::ostream& operator<<(std::ostream& stream, const Structures& structures) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "Structures: " << structures.GetCount() << Indent << Indent;
		for (std::uint32_t i = 0; i < structures.GetCount(); ++i) {
			stream << '\n' << defIndent << "\t[" << i << "]:\n" << structures[i];
		}
		stream << UnIndent << UnIndent;
		return stream;
	}
}