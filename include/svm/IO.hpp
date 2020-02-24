#pragma once

#include <ostream>
#include <string>

namespace svm {
	namespace detail {
		int ByteModeIndex();
		int DepthIndex();

		std::string MakeTabs(std::ostream& stream);
	}

	std::ostream& Byte(std::ostream& stream);
	std::ostream& Text(std::ostream& stream);

	std::ostream& Indent(std::ostream& stream);
	std::ostream& UnIndent(std::ostream& stream);
}