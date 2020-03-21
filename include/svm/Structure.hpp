#pragma once

#include <svm/Type.hpp>
#include <svm/detail/ReferenceWrapper.hpp>

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <vector>

namespace svm {
	struct Field final {
		std::size_t Offset = 0;
		svm::Type Type;
		std::size_t Count = 0;

		bool IsArray() const noexcept;
	};
}

namespace svm {
	class StructureInfo final {
	public:
		std::vector<Field> Fields;
		TypeInfo Type;

	public:
		StructureInfo() noexcept = default;
		StructureInfo(StructureInfo&& structure) noexcept;
		~StructureInfo() = default;

	public:
		StructureInfo& operator=(StructureInfo&& structure) noexcept;
		bool operator==(const StructureInfo&) = delete;
		bool operator!=(const StructureInfo&) = delete;
	};

	std::ostream& operator<<(std::ostream& stream, const StructureInfo& structureInfo);
}

namespace svm {
	class Structure final : public detail::ReferenceWrapper<StructureInfo> {
	public:
		using detail::ReferenceWrapper<StructureInfo>::ReferenceWrapper;
	};

	std::ostream& operator<<(std::ostream& stream, const Structure& structure);
}

namespace svm {
	class Structures final {
	private:
		std::vector<StructureInfo> m_Structures;

	public:
		Structures() noexcept = default;
		Structures(std::vector<StructureInfo>&& structures) noexcept;
		Structures(Structures&& structures) noexcept;
		~Structures() = default;

	public:
		Structures& operator=(Structures&& structures) noexcept;
		bool operator==(const Structures&) = delete;
		bool operator!=(const Structures&) = delete;
		Structure operator[](std::uint32_t index) const noexcept;

	public:
		void Clear() noexcept;
		bool IsEmpty() const noexcept;

		Structure GetStructure(std::uint32_t index) const noexcept;
		std::uint32_t GetStructureCount() const noexcept;
	};

	std::ostream& operator<<(std::ostream& stream, const Structures& structures);
}