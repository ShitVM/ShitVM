#pragma once

#include <cstddef>

namespace svm {
	struct ManagedHeapInfo final {
		std::size_t Size = 0;
		int Age = 0;

		explicit ManagedHeapInfo(std::size_t size) noexcept;
		ManagedHeapInfo(const ManagedHeapInfo& info) noexcept;
		~ManagedHeapInfo() = default;

		ManagedHeapInfo& operator=(const ManagedHeapInfo& info) noexcept;
		bool operator==(const ManagedHeapInfo&) = delete;
		bool operator!=(const ManagedHeapInfo&) = delete;
	};
}

namespace svm {
	class GarbageCollector {
	protected:
		GarbageCollector() noexcept = default;
		GarbageCollector(const GarbageCollector&) = delete;

	public:
		virtual ~GarbageCollector() = default;

	public:
		GarbageCollector& operator=(const GarbageCollector&) = delete;
		bool operator==(const GarbageCollector&) = delete;
		bool operator!=(const GarbageCollector&) = delete;

	public:
		virtual void* Allocate(std::size_t size) = 0;
	};
}