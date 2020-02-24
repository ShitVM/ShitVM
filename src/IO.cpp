#include <svm/IO.hpp>

#include <ios>

namespace svm {
	namespace detail {
		int DepthIndex() {
			static const int index = std::ios_base::xalloc();
			return index;
		}

		std::string MakeTabs(std::ostream& stream) {
			return std::string(stream.iword(DepthIndex()), '\t');
		}
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