#include <svm/Structure.hpp>

#include <svm/IO.hpp>

#include <utility>

namespace svm {
	StructureData::StructureData(std::vector<Type> fieldTypes, std::vector<std::size_t> fieldOffsets) noexcept
		: m_FieldTypes(std::move(fieldTypes)), m_FieldOffsets(std::move(fieldOffsets)) {}
	StructureData::StructureData(StructureData&& structure) noexcept
		: m_FieldTypes(std::move(structure.m_FieldTypes)), m_FieldOffsets(std::move(structure.m_FieldOffsets)) {}

	StructureData& StructureData::operator=(StructureData&& structure) noexcept {
		m_FieldTypes = std::move(structure.m_FieldTypes);
		m_FieldOffsets = std::move(structure.m_FieldOffsets);
		return *this;
	}

	const std::vector<Type>& StructureData::GetFieldTypes() const noexcept {
		return m_FieldTypes;
	}
	const std::vector<std::size_t>& StructureData::GetFieldOffsets() const noexcept {
		return m_FieldOffsets;
	}
}

namespace svm {
	std::ostream& operator<<(std::ostream& stream, const Structure& structure) {
		const std::string defIndent = detail::MakeTabs(stream);

		const auto& types = structure->GetFieldTypes();

		stream << defIndent << "Structure:\n"
			   << defIndent << "\tFields:";
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(types.size()); ++i) {
			stream << '\n' << defIndent << "\t\t[" << i << "]: " << types[i]->Name;
		}
		return stream;
	}
	std::ostream& operator<<(std::ostream& stream, const Structures& structures) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "Structures:" << Indent << Indent;
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(structures.size()); ++i) {
			stream << '\n' << defIndent << "\t[" << i << "]:\n" << structures[i];
		}
		stream << UnIndent << UnIndent;
		return stream;
	}
}