#pragma once

#include <svm/GarbageCollector.hpp>
#include <svm/Stack.hpp>

#include <cstddef>
#include <list>
#include <unordered_map>

namespace svm {
	class SimpleGarbageCollector final : public GarbageCollector {
	private:
		std::list<Stack> m_YoungGenerations;
		std::list<Stack>::iterator m_Hospital;
		std::unordered_map<void*, ManagedHeapInfo> m_OldGeneration;

	public:
		SimpleGarbageCollector() = default;
		SimpleGarbageCollector(SimpleGarbageCollector&& gc) noexcept;
		~SimpleGarbageCollector();

	public:
		SimpleGarbageCollector& operator=(SimpleGarbageCollector&& gc) noexcept;
		bool operator==(const SimpleGarbageCollector&) = delete;
		bool operator!=(const SimpleGarbageCollector&) = delete;

	public:
		void Reset() noexcept;

		virtual void* Allocate(std::size_t size) override;

	private:
		void MinorGC();
		void MajorGC();
		void FullGC();
	};
}