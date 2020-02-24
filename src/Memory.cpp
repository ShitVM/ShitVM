#include <svm/Memory.hpp>

namespace svm {
#ifndef SVM_LITTLE
	Endian GetEndian() noexcept {
		static const std::uint32_t tester = 1;
		static const Endian endian = *reinterpret_cast<const std::uint8_t*>(&tester) == 1 ? Endian::Little : Endian::Big;
		return endian;
	}
#endif
}