#include <svm/GarbageCollector.hpp>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>
#include <vector>

namespace svm {
	ManagedHeapInfo::ManagedHeapInfo(std::size_t size) noexcept
		: Size(size) {}
	ManagedHeapInfo::ManagedHeapInfo(const ManagedHeapInfo& info) noexcept
		: Size(info.Size), Age(info.Age) {}

	ManagedHeapInfo& ManagedHeapInfo::operator=(const ManagedHeapInfo& info) noexcept {
		Size = info.Size;
		Age = info.Age;
		return *this;
	}
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
		else return m_CurrentBlock->GetTop<std::uint8_t>();
	}

	void* ManagedHeapGeneration::CreateNewBlock(std::size_t size) {
		try {
			Stack newBlock(std::max(size, m_DefaultBlockSize));
			newBlock.SetUsedSize(size);

			void* const result = m_Blocks.insert(std::next(m_CurrentBlock), std::move(newBlock))->GetTop<std::uint8_t>();
			return ++m_CurrentBlock, result;
		} catch (...) {
			return nullptr;
		}
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::GetEmptyBlock() {
		const Block iter = Next(m_CurrentBlock);

		if (iter->GetUsedSize() != 0) return m_Blocks.insert(iter, Stack(m_DefaultBlockSize));
		else return iter;
	}
	void ManagedHeapGeneration::DeleteEmptyBlocks() {
		std::vector<Block> blocks;
		for (Block iter = m_Blocks.begin(); iter != m_Blocks.end(); ++iter) {
			if (iter->GetUsedSize() == 0) {
				blocks.push_back(iter);
			}
		}

		for (std::size_t i = 8; i < blocks.size(); ++i) {
			m_Blocks.erase(blocks[i]);
		}
	}

	ManagedHeapGeneration::Block ManagedHeapGeneration::GetCurrentBlock() noexcept {
		return m_CurrentBlock;
	}
	void ManagedHeapGeneration::SetCurrentBlock(Block newCurrentBlock) noexcept {
		m_CurrentBlock = newCurrentBlock;
	}
	std::size_t ManagedHeapGeneration::GetCurrentBlockSize() const noexcept {
		return m_CurrentBlock->GetSize();
	}
	std::size_t ManagedHeapGeneration::GetCurrentBlockUsedSize() const noexcept {
		return m_CurrentBlock->GetUsedSize();
	}
	std::size_t ManagedHeapGeneration::GetCurrentBlockFreeSize() const noexcept {
		return m_CurrentBlock->GetFreeSize();
	}

	ManagedHeapGeneration::Block ManagedHeapGeneration::Begin() noexcept {
		return m_Blocks.begin();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::End() noexcept {
		return m_Blocks.end();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::Prev(Block iterator) noexcept {
		if (iterator == Begin()) return std::prev(End());
		else return std::prev(iterator);
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::Next(Block iterator) noexcept {
		const auto result = std::next(iterator);
		if (result == End()) return Begin();
		else return result;
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::FindBlock(const void* address) noexcept {
		const std::uint8_t* ptr = static_cast<const std::uint8_t*>(address);

		for (Block iter = m_Blocks.begin(); iter != m_Blocks.end(); ++iter) {
			const std::uint8_t* begin = iter->Begin();
			const std::uint8_t* last = iter->Last();
			if (begin <= ptr && ptr <= last) return iter;
		}

		return m_Blocks.end();
	}

	std::size_t ManagedHeapGeneration::GetDefaultBlockSize() const noexcept {
		return m_DefaultBlockSize;
	}
	std::size_t ManagedHeapGeneration::GetBlockCount() const noexcept {
		return m_Blocks.size();
	}
}