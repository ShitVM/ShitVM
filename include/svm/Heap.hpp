#pragma once

#include <svm/GarbageCollector.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace svm {
	struct HeapInfo final {
		void* Address = nullptr;
		std::size_t Size = 0;

		HeapInfo() noexcept = default;
		HeapInfo(void* address, std::size_t size) noexcept;
		HeapInfo(const HeapInfo& info) noexcept = default;
		~HeapInfo() = default;

		HeapInfo& operator=(const HeapInfo& info) noexcept = default;
		bool operator==(void* address) const noexcept;
		bool operator==(const HeapInfo& info) const noexcept;
		bool operator!=(void* address) const noexcept;
		bool operator!=(const HeapInfo& info) const noexcept;
	};
}

namespace svm {
	class Heap final {
	private:
		std::vector<HeapInfo> m_UnmanagedHeap;
		std::unique_ptr<GarbageCollector> m_GarbageCollector;

	public:
		Heap() noexcept = default;
		Heap(Heap&& heap) noexcept;
		~Heap();

	public:
		Heap& operator=(Heap&& heap) noexcept;
		bool operator==(const Heap&) = delete;
		bool operator!=(const Heap&) = delete;

	public:
		void Deallocate() noexcept;

		void* AllocateUnmanagedHeap(std::size_t size);
		bool DeallocateUnmanagedHeap(void* address) noexcept;
		
		void* AllocateManagedHeap(std::size_t size);
	};
}