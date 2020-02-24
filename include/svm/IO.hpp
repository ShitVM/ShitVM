#pragma once

#include <ostream>

namespace svm {
	namespace detail {
		int ByteModeIndex();
	}

	std::ostream& Byte(std::ostream& stream);
	std::ostream& Text(std::ostream& stream);
}