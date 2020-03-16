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
	void ManagedHeapGeneration::SetCurrentBlock(Block newCurrentBlock) noexcept {
		m_CurrentBlock = newCurrentBlock;
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

	ManagedHeapGeneration::Block ManagedHeapGeneration::Begin() noexcept {
		return m_Blocks.begin();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::End() noexcept {
		return m_Blocks.end();
	}
	ManagedHeapGeneration::Block ManagedHeapGeneration::FindBlock(const void* address) noexcept {
		const Any* ptr = static_cast<const Any*>(address);

		for (Block iter = m_Blocks.begin(); iter != m_Blocks.end(); ++iter) {
			const Any* begin = iter->Begin();
			const Any* last = iter->Last();
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
		m_OldGeneration.Initialize(oldGenerationSize);
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

		std::memset(address + 1, 0, size - sizeof(ManagedHeapInfo));
		address->Size = size;
		address->Age = 0;
		return address;
	}
	void SimpleGarbageCollector::MakeDirty(const void* address) noexcept {
		const std::uintptr_t addressInt = reinterpret_cast<std::uintptr_t>(address);
		const std::uintptr_t byte = addressInt / 512;
		const int bit = static_cast<int>((addressInt - byte) / 8);

		std::uint8_t& card = m_CardTable[byte];
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
			return address;
		}

		if (size > m_OldGeneration.GetCurrentBlockFreeSize()) {
			MajorGC(interpreter);
		}

		void* address = m_OldGeneration.Allocate(size);
		if (!address) {
			address = m_OldGeneration.CreateNewBlock(size);
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
		const auto endBlock = m_YoungGeneration.End();

		std::unordered_map<void*, std::vector<void**>> pointerTable;

		for (std::uint32_t i = 0; i < varCount; ++i) {
			Type* const varTypePtr = interpreter.GetLocalVariable(i);
			if (*varTypePtr != GCPointerType) continue;

			GCPointerObject* const var = reinterpret_cast<GCPointerObject*>(varTypePtr);
			const auto block = m_YoungGeneration.FindBlock(var->Value);
			if (block == endBlock) continue;

			ManagedHeapInfo* const info = reinterpret_cast<ManagedHeapInfo*>(var->Value);
			info->Age |= 1 << 7;
			pointerTable[info].push_back(&var->Value);
		}

		for (auto block = m_OldGeneration.Begin(); block != m_OldGeneration.End(); ++block) {
			std::size_t offset = block->GetUsedSize();
			while (offset) {
				ManagedHeapInfo* const info = block->Get<ManagedHeapInfo>(offset);
				if (!IsDirty(info)) {
					offset -= info->Size;
					continue;
				}

				Type* const typePtr = reinterpret_cast<Type*>(info + 1);
				if (!typePtr->IsStructure()) continue;

				const Structure structure = interpreter.GetByteFile().GetStructures()[static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10];
				const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

				for (std::uint32_t i = 0; i < fieldCount; ++i) {
					Type* const fieldTypePtr = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
					if (*fieldTypePtr != GCPointerType) continue;

					GCPointerObject* const field = reinterpret_cast<GCPointerObject*>(fieldTypePtr);
					ManagedHeapInfo* const targetInfo = static_cast<ManagedHeapInfo*>(field->Value);
					if (!targetInfo || m_YoungGeneration.FindBlock(targetInfo) == endBlock) continue;

					targetInfo->Age |= 1 << 7;
					pointerTable[targetInfo].push_back(&field->Value);
				}

				offset -= info->Size;
			}
		}

		for (auto block = m_YoungGeneration.Begin(); block != endBlock; ++block) {
			std::size_t offset = block->GetUsedSize();
			while (offset) {
				ManagedHeapInfo* const info = block->Get<ManagedHeapInfo>(offset);
				offset -= MarkYoungGCObject(interpreter, pointerTable, info) + sizeof(ManagedHeapInfo);
			}
		}

		// Sweep
		std::vector<void*> promoted;

		auto emptyBlock = m_YoungGeneration.GetEmptyBlock();
		for (std::size_t i = 0; i < m_YoungGeneration.GetBlockCount(); ++i) {
			auto currentBlock = std::next(emptyBlock);
			if (currentBlock == endBlock) {
				currentBlock = m_YoungGeneration.Begin();
			}

			while (currentBlock->GetUsedSize()) {
				ManagedHeapInfo* const info = currentBlock->GetTop<ManagedHeapInfo>();
				if (info->Age >> 7 == 0) {
					currentBlock->Reduce(info->Size);
					continue;
				}

				info->Age &= 0b00111111;
				info->Age += 1;

				void* newAddress = nullptr;
				if ((info->Age & 0b00111111) == 32) {
					newAddress = AllocateOnOldGeneration(interpreter, info->Size);
					if (reinterpret_cast<Type*>(info + 1)->IsStructure()) {
						promoted.push_back(newAddress);
					}
				} else {
					if (!emptyBlock->Expand(info->Size)) {
						emptyBlock = m_YoungGeneration.GetEmptyBlock();
						emptyBlock->Expand(info->Size);
					}
					newAddress = emptyBlock->GetTop<Any>();
				}

				std::vector<void**>& pointers = pointerTable[info];
				for (void** pointer : pointers) {
					*pointer = newAddress;
				}

				pointers[0] = static_cast<void**>(newAddress);
				currentBlock->Reduce(info->Size);
			}
		}

		for (const auto& [from, to] : pointerTable) {
			std::memcpy(to[0], from, static_cast<ManagedHeapInfo*>(from)->Size);
		}

		m_YoungGeneration.SetCurrentBlock(emptyBlock);

		for (auto address : promoted) {
			const Type* type = reinterpret_cast<const Type*>(static_cast<const ManagedHeapInfo*>(address) + 1);
			const Structure structure = interpreter.GetByteFile().GetStructures()[static_cast<std::uint32_t>(type->GetReference().Code) - 10];
			const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

			for (std::uint32_t i = 0; i < fieldCount; ++i) {
				const Type* fieldType = reinterpret_cast<const Type*>(reinterpret_cast<const std::uint8_t*>(type) + structure->FieldOffsets[i]);
				if (*fieldType != GCPointerType) continue;

				const void* target = reinterpret_cast<const GCPointerObject*>(fieldType)->Value;
				const auto block = m_YoungGeneration.FindBlock(target);
				if (block != endBlock) {
					MakeDirty(address);
					break;
				}
			}
		}
	}

	std::size_t SimpleGarbageCollector::MarkYoungGCObject(Interpreter& interpreter, std::unordered_map<void*, std::vector<void**>>& pointerTable, ManagedHeapInfo* info) {
		Type* const typePtr = reinterpret_cast<Type*>(info + 1);
		if (info->Age >> 7 == 0 || (info->Age >> 6 & 0b1) == 1 || !typePtr->IsStructure()) return typePtr->GetReference().Size;

		info->Age |= 1 << 7;

		const Structure structure = interpreter.GetByteFile().GetStructures()[static_cast<std::uint32_t>(typePtr->GetReference().Code) - 10];
		const std::uint32_t fieldCount = static_cast<std::uint32_t>(structure->FieldTypes.size());

		for (std::uint32_t i = 0; i < fieldCount; ++i) {
			Type* const fieldTypePtr = reinterpret_cast<Type*>(reinterpret_cast<std::uint8_t*>(typePtr) + structure->FieldOffsets[i]);
			if (*fieldTypePtr != GCPointerType) continue;

			GCPointerObject* const field = reinterpret_cast<GCPointerObject*>(fieldTypePtr);
			ManagedHeapInfo* const targetInfo = static_cast<ManagedHeapInfo*>(field->Value);
			if (!targetInfo || m_YoungGeneration.FindBlock(targetInfo) == m_YoungGeneration.End()) continue;

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
		const std::uintptr_t byte = addressInt / 512;
		const int bit = static_cast<int>((addressInt - byte) / 8);
		const auto iter = m_CardTable.find(byte);

		if (iter == m_CardTable.end()) return false;
		else return iter->second >> bit & 0b1;
	}
}