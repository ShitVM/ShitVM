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
		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structureInfo.FieldTypes.size());

		stream << defIndent << "Structure: " << structureInfo.Type.Size << "B\n"
			   << defIndent << "\tFields: " << fieldCount;
		for (std::uint32_t i = 0; i < fieldCount; ++i) {
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

	Structure Structures::GetStructure(std::uint32_t index) const noexcept {
		return m_Structures[index];
	}
	std::uint32_t Structures::GetStructureCount() const noexcept {
		return static_cast<std::uint32_t>(m_Structures.size());
	}

	std::ostream& operator<<(std::ostream& stream, const Structures& structures) {
		const std::string defIndent = detail::MakeTabs(stream);
		const std::uint32_t structCount = structures.GetStructureCount();

		stream << defIndent << "Structures: " << structCount << Indent << Indent;
		for (std::uint32_t i = 0; i < structCount; ++i) {
			stream << '\n' << defIndent << "\t[" << i << "]:\n" << structures[i];
		}
		stream << UnIndent << UnIndent;
		return stream;
	}
}