#pragma once

#include <svm/GarbageCollector.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace svm {
	class Interpreter;

	class Heap final {
	private:
		std::unordered_map<void*, std::size_t> m_UnmanagedHeap;
		std::unique_ptr<GarbageCollector> m_GarbageCollector;

	public:
		Heap() = default;
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

		void SetGarbageCollector(std::unique_ptr<GarbageCollector>&& gc) noexcept;
		void* AllocateManagedHeap(Interpreter& interpreter, std::size_t size);
	};
}