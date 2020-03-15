#include <svm/gc/SimpleGarbageCollector.hpp>

#include <svm/Macro.hpp>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <utility>

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
}