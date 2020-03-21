#include <svm/Heap.hpp>

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace svm {
	Heap::Heap(Heap&& heap) noexcept
		: m_UnmanagedHeap(std::move(heap.m_UnmanagedHeap)), m_GarbageCollector(std::move(heap.m_GarbageCollector)) {}
	Heap::~Heap() {
		Deallocate();
	}

	Heap& Heap::operator=(Heap&& heap) noexcept {
		m_UnmanagedHeap = std::move(heap.m_UnmanagedHeap);
		m_GarbageCollector = std::move(heap.m_GarbageCollector);
		return *this;
	}

	void Heap::Deallocate() noexcept {
		for (const auto& [address, size] : m_UnmanagedHeap) {
			std::free(address);
		}

		m_UnmanagedHeap.clear();
		m_GarbageCollector.reset();
	}

	void* Heap::AllocateUnmanagedHeap(std::size_t size) {
		struct Deleter final {
			void operator()(void* address) noexcept {
				std::free(address);
			}
		};
		std::unique_ptr<void, Deleter> memory(std::calloc(1, size));

		if (memory) {
			m_UnmanagedHeap[memory.get()] = size;
		}

		return memory.release();
	}
	bool Heap::DeallocateUnmanagedHeap(void* address) noexcept {
		const auto iter = m_UnmanagedHeap.find(address);
		if (iter == m_UnmanagedHeap.end()) return false;

		std::free(iter->first);
		m_UnmanagedHeap.erase(iter);
		return true;
	}

	void Heap::SetGarbageCollector(std::unique_ptr<GarbageCollector>&& gc) noexcept {
		m_GarbageCollector = std::move(gc);
	}
	void* Heap::AllocateManagedHeap(Interpreter& interpreter, std::size_t size) {
		if (!m_GarbageCollector) return nullptr;
		else return m_GarbageCollector->Allocate(interpreter, size);
	}
}