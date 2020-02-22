#pragma once

#include <ostream>

namespace svm {
	namespace detail {
		int ByteModeIndex();
		int InstLabelModeIndex();
	}

	std::ostream& Byte(std::ostream& stream);
	std::ostream& Text(std::ostream& stream);

	std::ostream& InstOnly(std::ostream& stream);
	std::ostream& LabelOnly(std::ostream& stream);
	std::ostream& InstLabel(std::ostream& stream);
}