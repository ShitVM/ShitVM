#include <svm/Heap.hpp>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <utility>

namespace svm {
	HeapInfo::HeapInfo(void* address, std::size_t size) noexcept
		: Address(address), Size(size) {}

	bool HeapInfo::operator==(void* address) const noexcept {
		return Address == address;
	}
	bool HeapInfo::operator==(const HeapInfo& info) const noexcept {
		return Address == info.Address;
	}
	bool HeapInfo::operator!=(void* address) const noexcept {
		return Address == address;
	}
	bool HeapInfo::operator!=(const HeapInfo& info) const noexcept {
		return Address != info.Address;
	}
}

namespace svm {
	Heap::Heap(Heap&& heap) noexcept
		: m_UnmanagedHeap(std::move(heap.m_UnmanagedHeap)) {}
	Heap::~Heap() {
		Deallocate();
	}

	Heap& Heap::operator=(Heap&& heap) noexcept {
		m_UnmanagedHeap = std::move(heap.m_UnmanagedHeap);
		return *this;
	}

	void Heap::Deallocate() noexcept {
		for (const auto& info : m_UnmanagedHeap) {
			std::free(info.Address);
		}

		m_UnmanagedHeap.clear();
	}

	void* Heap::AllocateUnmanagedHeap(std::size_t size) noexcept {
		struct Deleter final {
			void operator()(void* address) noexcept {
				std::free(address);
			}
		};
		std::unique_ptr<void, Deleter> memory(std::calloc(1, size));

		if (memory) {
			m_UnmanagedHeap.emplace_back(memory.get(), size);
		}

		return memory.release();
	}
	bool Heap::DeallocateUnmanagedHeap(void* address) noexcept {
		const auto iter = std::find(m_UnmanagedHeap.begin(), m_UnmanagedHeap.end(), address);
		if (iter == m_UnmanagedHeap.end()) return false;

		std::free(iter->Address);
		m_UnmanagedHeap.erase(iter);
		return true;
	}
	void Heap::DeallocateLastUnmanagedHeap() noexcept {
		assert(m_UnmanagedHeap.size() != 0);

		const auto iter = m_UnmanagedHeap.end() - 1;
		std::free(iter->Address);
		m_UnmanagedHeap.erase(iter);
	}
}