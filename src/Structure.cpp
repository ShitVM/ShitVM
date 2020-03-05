#include <svm/Structure.hpp>

#include <svm/IO.hpp>

#include <utility>

namespace svm {
	StructureInfo::StructureInfo(StructureInfo&& structure) noexcept
		: Fields(std::move(structure.Fields)) {}

	StructureInfo& StructureInfo::operator=(StructureInfo&& structure) noexcept {
		Fields = std::move(structure.Fields);
		return *this;
	}

	std::ostream& operator<<(std::ostream& stream, const StructureInfo& structureInfo) {
		const std::string defIndent = detail::MakeTabs(stream);

		stream << defIndent << "Structure:\n"
			   << defIndent << "\tFields:";
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(structureInfo.Fields.size()); ++i) {
			stream << '\n' << defIndent << "\t\t[" << i << "]: " << structureInfo.Fields[i]->Name;
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

		stream << defIndent << "Structures:" << Indent << Indent;
		for (std::uint32_t i = 0; i < structures.GetCount(); ++i) {
			stream << '\n' << defIndent << "\t[" << i << "]:\n" << structures[i];
		}
		stream << UnIndent << UnIndent;
		return stream;
	}
}