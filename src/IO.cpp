#include <svm/IO.hpp>

namespace svm {
	namespace detail {
		int ByteModeIndex() {
			static const int index = std::ios_base::xalloc();
			return index;
		}
		int InstLabelModeIndex() {
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

	std::ostream& InstOnly(std::ostream& stream) {
		stream.iword(detail::InstLabelModeIndex()) = 0;
		return stream;
	}
	std::ostream& LabelOnly(std::ostream& stream) {
		stream.iword(detail::InstLabelModeIndex()) = 1;
		return stream;
	}
	std::ostream& InstLabel(std::ostream& stream) {
		stream.iword(detail::InstLabelModeIndex()) = 2;
		return stream;
	}
}