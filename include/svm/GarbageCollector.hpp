#pragma once

#include <cstddef>

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