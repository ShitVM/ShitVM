#include <svm/IO.hpp>

#include <ios>

namespace svm {
	namespace detail {
		int ByteModeIndex() {
			static const int index = std::ios_base::xalloc();
			return index;
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
}