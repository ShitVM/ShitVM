#include <svm/gc/SimpleGarbageCollector.hpp>

#include <svm/Macro.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
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
		: m_YoungGeneration(std::move(gc.m_YoungGeneration)), m_OldGeneration(std::move(gc.m_OldGeneration)), m_CardTable(std::move(gc.m_CardTable)) {}
	SimpleGarbageCollector::~SimpleGarbageCollector() {
		Reset();
	}

	SimpleGarbageCollector& SimpleGarbageCollector::operator=(SimpleGarbageCollector&& gc) noexcept {
		Reset();

		m_YoungGeneration = std::move(gc.m_YoungGeneration);
		m_OldGeneration = std::move(gc.m_OldGeneration);
		m_CardTable = std::move(gc.m_CardTable);
		return *this;
	}

	void SimpleGarbageCollector::Reset() noexcept {
		m_YoungGeneration.Reset();
		m_OldGeneration.Reset();
		m_CardTable.clear();
	}
	void SimpleGarbageCollector::Initialize(std::size_t youngGenerationSize, std::size_t oldGenerationSize) {
		assert(!IsInitialized());
		assert(youngGenerationSize % 512 == 0);
		assert(oldGenerationSize % 512 == 0);

		m_YoungGeneration.Initialize(youngGenerationSize);
		m_OldGeneration.Initialize(oldGenerationSize + oldGenerationSize / 512);
		m_CardTable.resize(CalcCardTableSize(oldGenerationSize));
	}
	bool SimpleGarbageCollector::IsInitialized() const noexcept {
		return !m_YoungGeneration.IsInitalized() && m_YoungGeneration.IsInitalized();
	}

	void* SimpleGarbageCollector::Allocate(std::size_t size) {
		if (size > m_YoungGeneration.GetDefaultBlockSize()) return AllocateOnOldGeneration(size);
		else return AllocateOnYoungGeneration(size);
	}
	void SimpleGarbageCollector::MakeDirty(void* address) noexcept {
		const std::uintptr_t addressInt = reinterpret_cast<std::uintptr_t>(address);
		const std::size_t byte = static_cast<std::size_t>(addressInt / 512);
		const int bit = static_cast<int>((addressInt - byte) / 8);

		std::uint8_t& card = m_CardTable[reinterpret_cast<std::uintptr_t>(address) / 512];
		card |= (1 << bit);
	}

	void* SimpleGarbageCollector::AllocateOnYoungGeneration(std::size_t size) {
		if (size > m_YoungGeneration.GetCurrentBlockFreeSize()) {
			MinorGC();
		}

		void* address = m_YoungGeneration.Allocate(size);
		if (!address) {
			address = m_YoungGeneration.CreateNewBlock(size);
		}
		return address;
	}
	void* SimpleGarbageCollector::AllocateOnOldGeneration(std::size_t size) {
		if (size > m_OldGeneration.GetDefaultBlockSize()) {
			void* const address = m_OldGeneration.CreateNewBlock(size);
			m_CardTable.resize(m_CardTable.size() + CalcCardTableSize(size));
			return address;
		}

		if (size > m_OldGeneration.GetCurrentBlockFreeSize()) {
			MajorGC();
		}

		void* address = m_OldGeneration.Allocate(size);
		if (!address) {
			address = m_OldGeneration.CreateNewBlock(size);
			m_CardTable.emplace_back();
		}
		return address;
	}

	void SimpleGarbageCollector::MajorGC() noexcept {
		// TODO
	}
	void SimpleGarbageCollector::MinorGC() noexcept {
		// TODO
	}

	std::size_t SimpleGarbageCollector::CalcCardTableSize(std::size_t newBlockSize) const noexcept {
		const auto temp = newBlockSize / 512;
		if (newBlockSize == temp * 512) return temp;
		else return temp + 1;
	}
}