#include <svm/GarbageCollector.hpp>

namespace svm {
	ManagedHeapInfo::ManagedHeapInfo(std::size_t size) noexcept
		: Size(size) {}
	ManagedHeapInfo::ManagedHeapInfo(std::size_t size, Type type) noexcept
		: Size(size), Object(type) {}
	ManagedHeapInfo::ManagedHeapInfo(const ManagedHeapInfo& info) noexcept
		: Size(info.Size), Age(info.Age), Object(info.Object) {}

	ManagedHeapInfo& ManagedHeapInfo::operator=(const ManagedHeapInfo& info) noexcept {
		Size = info.Size;
		Age = info.Age;
		Object = info.Object;
		return *this;
	}
}