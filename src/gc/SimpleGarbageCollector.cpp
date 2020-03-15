#include <svm/gc/SimpleGarbageCollector.hpp>

#include <svm/Macro.hpp>

#include <algorithm>
#include <cassert>
#include <utility>

namespace {
	using Any = std::uint8_t;
}

namespace svm {
	ManagedHeapGeneration::ManagedHeapGeneration(std::size_t defaultBlockSize) {
		Initialize(defaultBlockSize);
	}
	ManagedHeapGeneration::ManagedHeapGeneration(ManagedHeapGeneration&& generation) noexcept
		: m_Blocks(std::move(generation.m_Blocks)), m_CurrentBlock(generation.m_CurrentBlock), m_DefaultBlockSize(generation.m_DefaultBlockSize) {}

	ManagedHeapGeneration& ManagedHeapGeneration::operator=(ManagedHeapGeneration&& generation) noexcept {
		m_Blocks = std::move(generation.m_Blocks);
		m_CurrentBlock = generation.m_CurrentBlock;
		m_DefaultBlockSize = generation.m_DefaultBlockSize;
		return *this;
	}

	void ManagedHeapGeneration::Reset() noexcept {
		m_Blocks.clear();
	}
	void ManagedHeapGeneration::Initialize(std::size_t defaultBlockSize) {
		assert(!IsInitalized());

		m_Blocks.emplace_back(defaultBlockSize);
		m_CurrentBlock = m_Blocks.begin();
		m_DefaultBlockSize = defaultBlockSize;
	}
	bool ManagedHeapGeneration::IsInitalized() const noexcept {
		return !m_Blocks.empty();
	}

	void* ManagedHeapGeneration::Allocate(std::size_t size) noexcept {
		if (!m_CurrentBlock->Expand(size)) return nullptr;
		else return m_CurrentBlock->GetTop<Any>();
	}

	void* ManagedHeapGeneration::CreateNewBlock(std::size_t size) {
		Stack newBlock(std::max(size, m_DefaultBlockSize));
		newBlock.SetUsedSize(size);

		std::list<Stack>::iterator iter = m_CurrentBlock;
		if (size <= m_DefaultBlockSize) {
			++iter;
		}

		const auto newBlockIter = m_Blocks.insert(iter, std::move(newBlock));
		if (size <= m_DefaultBlockSize) {
			m_CurrentBlock = newBlockIter;
		}

		return newBlockIter->GetTop<Any>();
	}

	std::size_t ManagedHeapGeneration::GetCurrentBlockSize() const noexcept {
		return m_CurrentBlock->GetSize();
	}
	std::size_t ManagedHeapGeneration::GetCurrnetBlockUsedSize() const noexcept {
		return m_CurrentBlock->GetUsedSize();
	}
	std::size_t ManagedHeapGeneration::GetCurrentBlockFreeSize() const noexcept {
		return m_CurrentBlock->GetFreeSize();
	}

	std::size_t ManagedHeapGeneration::GetDefaultBlockSize() const noexcept {
		return m_DefaultBlockSize;
	}
}

namespace svm {
	SimpleGarbageCollector::SimpleGarbageCollector(std::size_t youngGenerationSize, std::size_t oldGenerationSize) {
		Initialize(youngGenerationSize, oldGenerationSize);
	}
	SimpleGarbageCollector::SimpleGarbageCollector(SimpleGarbageCollector&& gc) noexcept
		: m_YoungGeneration(std::move(gc.m_YoungGeneration)), m_OldGeneration(std::move(gc.m_OldGeneration)) {}
	SimpleGarbageCollector::~SimpleGarbageCollector() {
		Reset();
	}

	SimpleGarbageCollector& SimpleGarbageCollector::operator=(SimpleGarbageCollector&& gc) noexcept {
		Reset();

		m_YoungGeneration = std::move(gc.m_YoungGeneration);
		m_OldGeneration = std::move(gc.m_OldGeneration);
		return *this;
	}

	void SimpleGarbageCollector::Reset() noexcept {
		m_YoungGeneration.Reset();
		m_OldGeneration.Reset();
	}
	void SimpleGarbageCollector::Initialize(std::size_t youngGenerationSize, std::size_t oldGenerationSize) {
		assert(!IsInitialized());
		assert(youngGenerationSize % 512 == 0);
		assert(oldGenerationSize % 512 == 0);

		m_YoungGeneration.Initialize(youngGenerationSize);
		m_OldGeneration.Initialize(oldGenerationSize + oldGenerationSize / 512);
	}
	bool SimpleGarbageCollector::IsInitialized() const noexcept {
		return !m_YoungGeneration.IsInitalized() && m_YoungGeneration.IsInitalized();
	}

	void* SimpleGarbageCollector::Allocate(std::size_t size) {
		const std::size_t youngBlockSize = m_YoungGeneration.GetDefaultBlockSize();
		const std::size_t oldBlockSize = m_OldGeneration.GetDefaultBlockSize();

		if (size > youngBlockSize) {
			if (size > oldBlockSize) return m_OldGeneration.CreateNewBlock(size);

			if (size > m_OldGeneration.GetCurrentBlockFreeSize()) {
				MajorGC();
			}

			void* address = m_OldGeneration.Allocate(size);
			if (!address) {
				address = m_OldGeneration.CreateNewBlock(size);
			}
			return address;
		}

		if (size > m_YoungGeneration.GetCurrentBlockFreeSize()) {
			MinorGC();
		}

		void* address = m_YoungGeneration.Allocate(size);
		if (!address) {
			address = m_YoungGeneration.CreateNewBlock(size);
		}
		return address;
	}

	void SimpleGarbageCollector::MajorGC() noexcept {
		// TODO
	}
	void SimpleGarbageCollector::MinorGC() noexcept {
		// TODO
	}
}