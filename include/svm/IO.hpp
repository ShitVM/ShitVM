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
}