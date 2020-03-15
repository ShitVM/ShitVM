#include <svm/GarbageCollector.hpp>

namespace svm {
	ManagedHeapInfo::ManagedHeapInfo(std::size_t size) noexcept
		: Size(size) {}
	ManagedHeapInfo::ManagedHeapInfo(const ManagedHeapInfo& info) noexcept
		: Size(info.Size), Age(info.Age) {}

	ManagedHeapInfo& ManagedHeapInfo::operator=(const ManagedHeapInfo& info) noexcept {
		Size = info.Size;
		Age = info.Age;
		return *this;
	}
}