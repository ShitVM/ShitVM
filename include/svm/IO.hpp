#pragma once

#include <ostream>
#include <string>

namespace svm {
	namespace detail {
		int DepthIndex();
		std::string MakeTabs(std::ostream& stream);
	}
	std::ostream& Indent(std::ostream& stream);
	std::ostream& UnIndent(std::ostream& stream);

	namespace detail {
		struct QWord final {
			std::uint64_t Value;
		};
		struct Hex final {
			std::uint64_t Value;
		};
	}
	detail::QWord QWord(std::uint64_t value) noexcept;
	detail::Hex Hex(std::uint64_t value) noexcept;
	std::ostream& operator<<(std::ostream& stream, const detail::QWord& qword);
	std::ostream& operator<<(std::ostream& stream, const detail::Hex& hex);
}