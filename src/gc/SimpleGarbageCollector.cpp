#include <svm/gc/SimpleGarbageCollector.hpp>

#include <cstdlib>
#include <utility>

namespace svm {
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

	void* SimpleGarbageCollector::Allocate(std::size_t size) {
		return nullptr; // TODO
	}
}