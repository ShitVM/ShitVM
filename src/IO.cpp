#include <svm/IO.hpp>

#include <ios>

namespace svm {
	namespace detail {
		int ByteModeIndex() {
			static const int index = std::ios_base::xalloc();
			return index;
		}
		int DepthIndex() {
			static const int index = std::ios_base::xalloc();
			return index;
		}

		std::string MakeTabs(std::ostream& stream) {
			return std::string(stream.iword(DepthIndex()), '\t');
		}
	}

	std::ostream& Byte(std::ostream& stream) {
		stream.iword(detail::ByteModeIndex()) = 1;
		return stream;
	}
	std::ostream& Text(std::ostream& stream) {
		stream.iword(detail::ByteModeIndex()) = 0;
		return stream;
	}

	std::ostream& Indent(std::ostream& stream) {
		++stream.iword(detail::DepthIndex());
		return stream;
	}
	std::ostream& UnIndent(std::ostream& stream) {
		--stream.iword(detail::DepthIndex());
		return stream;
	}
}