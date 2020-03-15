#include <svm/gc/SimpleGarbageCollector.hpp>

#include <cassert>
#include <cstdlib>
#include <utility>

namespace svm {
	SimpleGarbageCollector::SimpleGarbageCollector(std::size_t youngGenerationSize) {
		Initialize(youngGenerationSize);
	}
	SimpleGarbageCollector::SimpleGarbageCollector(SimpleGarbageCollector&& gc) noexcept
		: m_YoungGenerations(std::move(gc.m_YoungGenerations)), m_Hospital(gc.m_Hospital), m_OldGeneration(std::move(gc.m_OldGeneration)) {}
	SimpleGarbageCollector::~SimpleGarbageCollector() {
		Reset();
	}

	SimpleGarbageCollector& SimpleGarbageCollector::operator=(SimpleGarbageCollector&& gc) noexcept {
		Reset();

		m_YoungGenerations = std::move(gc.m_YoungGenerations);
		m_Hospital = gc.m_Hospital;
		m_OldGeneration = std::move(gc.m_OldGeneration);
		return *this;
	}

	void SimpleGarbageCollector::Reset() noexcept {
		for (const auto& [address, info] : m_OldGeneration) {
			std::free(address);
		}

		m_YoungGenerations.clear();
		m_OldGeneration.clear();
	}
	void SimpleGarbageCollector::Initialize(std::size_t youngGenerationSize) {
		assert(IsEmpty());

		m_YoungGenerations.emplace_back(youngGenerationSize);
		m_YoungGenerations.emplace_back(youngGenerationSize);
		m_Hospital = m_YoungGenerations.begin();
	}
	bool SimpleGarbageCollector::IsEmpty() const noexcept {
		return m_YoungGenerations.size() == 0;
	}

	void* SimpleGarbageCollector::Allocate(std::size_t size) {
		size += sizeof(ManagedHeapInfo);

		if (m_Hospital->GetFreeSize() < size) {
			GC(size);
		}

		m_Hospital->Expand(size);
		ManagedHeapInfo* const address = m_Hospital->GetTop<ManagedHeapInfo>();
		address->Size = size;
		address->Age = 0;

		return address;
	}

	void SimpleGarbageCollector::GC(std::size_t size) {
		// TODO
	}
	void SimpleGarbageCollector::MinorGC() {

	}
	void SimpleGarbageCollector::MajorGC() {
		// TODO
	}

	std::list<Stack>::iterator SimpleGarbageCollector::GetNextHosptial() noexcept {
		auto iter = m_Hospital;
		++iter;

		if (iter == m_YoungGenerations.end()) return m_YoungGenerations.begin();
		else return iter;
	}
}