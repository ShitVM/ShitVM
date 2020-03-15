#pragma once

#include <cstddef>
#include <cstdint>

namespace svm {
	struct ManagedHeapInfo final {
		std::size_t Size = 0;
		std::uint8_t Age = 0;

		explicit ManagedHeapInfo(std::size_t size) noexcept;
		ManagedHeapInfo(const ManagedHeapInfo& info) noexcept;
		~ManagedHeapInfo() = default;

		ManagedHeapInfo& operator=(const ManagedHeapInfo& info) noexcept;
		bool operator==(const ManagedHeapInfo&) = delete;
		bool operator!=(const ManagedHeapInfo&) = delete;
	};
}

namespace svm {
	class Interpreter;

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
		virtual void* Allocate(Interpreter& interpreter, std::size_t size) = 0;
		virtual void MakeDirty(const void* address) noexcept = 0;
	};
}