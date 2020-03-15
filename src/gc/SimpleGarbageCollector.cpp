#include <svm/gc/SimpleGarbageCollector.hpp>

#include <svm/Interpreter.hpp>
#include <svm/Macro.hpp>
#include <svm/Object.hpp>
#include <svm/Type.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <utility>

#include <iostream> // For developing

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

		return m_Blocks.insert(m_CurrentBlock, std::move(newBlock))->GetTop<Any>();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::GetEmptyBlock() {
		Block iter = std::next(m_CurrentBlock);
		if (iter == m_Blocks.end()) {
			iter = m_Blocks.begin();
		}

		if (iter->GetUsedSize() != 0) return m_Blocks.insert(iter, Stack(m_DefaultBlockSize));
		else return iter;
	}

	ManagedHeapGeneration::Block ManagedHeapGeneration::GetCurrentBlock() noexcept {
		return m_CurrentBlock;
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

	ManagedHeapGeneration::Block ManagedHeapGeneration::FirstBlock() noexcept {
		return m_Blocks.begin();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::NoBlock() noexcept {
		return m_Blocks.end();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::FindBlock(const void* address) noexcept {
		const Any* ptr = static_cast<const Any*>(address);

		for (Block iter = m_Blocks.begin(); iter != m_Blocks.end(); ++iter) {
			const Any* begin = iter->Get<Any>(iter->GetSize());
			const Any* last = iter->Get<Any>(0);
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

	void* SimpleGarbageCollector::Allocate(Interpreter& interpreter, std::size_t size) {
		size += sizeof(ManagedHeapInfo);

		ManagedHeapInfo* address = nullptr;
		if (size > m_YoungGeneration.GetDefaultBlockSize()) {
			address = static_cast<ManagedHeapInfo*>(AllocateOnOldGeneration(interpreter, size));
		} else {
			address = static_cast<ManagedHeapInfo*>(AllocateOnYoungGeneration(interpreter, size));
		}

		address->Size = size;
		address->Age = 0;
		return address;
	}
	void SimpleGarbageCollector::MakeDirty(const void* address) noexcept {
		const std::uintptr_t addressInt = reinterpret_cast<std::uintptr_t>(address);
		const std::size_t byte = static_cast<std::size_t>(addressInt / 512);
		const int bit = static_cast<int>((addressInt - byte) / 8);

		std::uint8_t& card = m_CardTable[reinterpret_cast<std::uintptr_t>(address) / 512];
		card |= (1 << bit);
	}

	void* SimpleGarbageCollector::AllocateOnYoungGeneration(Interpreter& interpreter, std::size_t size) {
		if (size > m_YoungGeneration.GetCurrentBlockFreeSize()) {
			MinorGC(interpreter);
		}

		void* address = m_YoungGeneration.Allocate(size);
		if (!address) {
			address = m_YoungGeneration.CreateNewBlock(size);
		}
		return address;
	}
	void* SimpleGarbageCollector::AllocateOnOldGeneration(Interpreter& interpreter, std::size_t size) {
		if (size > m_OldGeneration.GetDefaultBlockSize()) {
			void* const address = m_OldGeneration.CreateNewBlock(size);
			m_CardTable.resize(m_CardTable.size() + CalcCardTableSize(size));
			return address;
		}

		if (size > m_OldGeneration.GetCurrentBlockFreeSize()) {
			MajorGC(interpreter);
		}

		void* address = m_OldGeneration.Allocate(size);
		if (!address) {
			address = m_OldGeneration.CreateNewBlock(size);
			m_CardTable.emplace_back();
		}
		return address;
	}

	void SimpleGarbageCollector::MajorGC(Interpreter& interpreter) {
		std::cout << "MajorGC\n";
	}
	void SimpleGarbageCollector::MinorGC(Interpreter& interpreter) {
		std::cout << "MinorGC\n";

		// Mark
		const std::uint32_t varCount = interpreter.GetLocalVariableCount();
		const auto noBlock = m_YoungGeneration.NoBlock();

		std::unordered_map<void*, std::vector<void**>> pointerTable;

		for (std::uint32_t i = 0; i < varCount; ++i) {
			Type* const varTypePtr = interpreter.GetLocalVariable(varCount);
			if (*varTypePtr != GCPointerType) continue;

			GCPointerObject* const var = reinterpret_cast<GCPointerObject*>(varTypePtr);
			const auto block = m_YoungGeneration.FindBlock(var->Value);
			if (block == noBlock) continue;

			ManagedHeapInfo* const info = reinterpret_cast<ManagedHeapInfo*>(var->Value);
			info->Age |= 1 << 7;
			pointerTable[info].push_back(&var->Value);
		}

		for (auto block = m_OldGeneration.FirstBlock(); block != m_OldGeneration.NoBlock(); ++block) {
			std::size_t offset = block->GetUsedSize();
			while (offset) {
				ManagedHeapInfo* const info = block->Get<ManagedHeapInfo>(offset);
				if (!IsDirty(info)) continue;

				Type* const typePtr = reinterpret_cast<Type*>(info + 1);
				if (!typePtr->IsStructure()) continue;

				const Structure structure = interpreter.GetByteFile().GetStructures()[static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10];
				const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

				for (std::uint32_t i = 0; i < fieldCount; ++i) {
					Type* const fieldTypePtr = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
					if (*fieldTypePtr != GCPointerType) continue;

					GCPointerObject* const field = reinterpret_cast<GCPointerObject*>(fieldTypePtr);
					ManagedHeapInfo* const targetInfo = static_cast<ManagedHeapInfo*>(field->Value);
					if (!targetInfo || m_YoungGeneration.FindBlock(targetInfo) == noBlock) continue;

					targetInfo->Age |= 1 << 7;
					pointerTable[targetInfo].push_back(&field->Value);
				}
			}
		}

		for (auto block = m_YoungGeneration.FirstBlock(); block != noBlock; ++block) {
			std::size_t offset = block->GetUsedSize();
			while (offset) {
				ManagedHeapInfo* const info = block->Get<ManagedHeapInfo>(offset);
				offset -= MarkYoungGCObject(interpreter, pointerTable, info) + sizeof(ManagedHeapInfo);
			}
		}

		// Sweep
		auto emptyBlock = m_YoungGeneration.GetEmptyBlock();
		for (std::size_t i = 0; i < m_YoungGeneration.GetBlockCount(); ++i) {
			auto currentBlock = std::next(emptyBlock);
			if (currentBlock == noBlock) {
				currentBlock = m_YoungGeneration.FirstBlock();
			}

			while (currentBlock->GetUsedSize()) {
				ManagedHeapInfo* const info = currentBlock->GetTop<ManagedHeapInfo>();
				if (info->Age >> 7 == 0) {
					currentBlock->Reduce(info->Size);
					continue;
				}

				info->Age &= 0b00 << 6;
				info->Age += 1;

				void* newAddress = nullptr;
				if ((info->Age & 0b00111111) == 32) {
					newAddress = AllocateOnOldGeneration(interpreter, info->Size);
				} else {
					if (!emptyBlock->Expand(info->Size)) {
						emptyBlock = m_YoungGeneration.GetEmptyBlock();
						emptyBlock->Expand(info->Size);
					}
					newAddress = emptyBlock->GetTop<Any>();
				}

				const std::vector<void**>& pointers = pointerTable[info];
				for (void** pointer : pointers) {
					*pointer = newAddress;
				}

				std::memcpy(newAddress, info, info->Size);
				currentBlock->Reduce(info->Size);
			}
		}
	}

	std::size_t SimpleGarbageCollector::MarkYoungGCObject(Interpreter& interpreter, std::unordered_map<void*, std::vector<void**>>& pointerTable, ManagedHeapInfo* info) {
		Type* const typePtr = reinterpret_cast<Type*>(info + 1);
		if (info->Age >> 7 == 0 || info->Age >> 6 == 1 || !typePtr->IsStructure()) return typePtr->GetReference().Size;

		info->Age |= 1 << 7;

		const Structure structure = interpreter.GetByteFile().GetStructures()[static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10];
		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

		for (std::uint32_t i = 0; i < fieldCount; ++i) {
			Type* const fieldTypePtr = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
			if (*fieldTypePtr != GCPointerType) continue;

			GCPointerObject* const field = reinterpret_cast<GCPointerObject*>(fieldTypePtr);
			ManagedHeapInfo* const targetInfo = static_cast<ManagedHeapInfo*>(field->Value);
			if (!targetInfo || m_YoungGeneration.FindBlock(targetInfo) == m_YoungGeneration.NoBlock()) continue;

			targetInfo->Age |= 1 << 7;
			MarkYoungGCObject(interpreter, pointerTable, targetInfo);
			pointerTable[targetInfo].push_back(&field->Value);
		}

		info->Age |= 1 << 6;

		return typePtr->GetReference().Size;
	}

	std::size_t SimpleGarbageCollector::CalcCardTableSize(std::size_t newBlockSize) const noexcept {
		const auto temp = newBlockSize / 512;
		if (newBlockSize == temp * 512) return temp;
		else return temp + 1;
	}
	bool SimpleGarbageCollector::IsDirty(const void* address) const noexcept {
		const std::uintptr_t addressInt = reinterpret_cast<std::uintptr_t>(address);
		const std::size_t byte = static_cast<std::size_t>(addressInt / 512);
		const int bit = static_cast<int>((addressInt - byte) / 8);

		const std::uint8_t card = m_CardTable[reinterpret_cast<std::uintptr_t>(address) / 512];
		return card >> bit & 0b1;
	}
}