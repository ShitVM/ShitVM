#pragma once

#include <svm/ByteFile.hpp>

#include <cstddef>
#include <memory>
#include <utility>

namespace svm {
	class Stack final {
	private:
		std::unique_ptr<std::uint8_t[]> m_Data;
		std::size_t m_Size = 0;
		std::size_t m_Used = 0;

	public:
		Stack() noexcept;
		explicit Stack(std::size_t size);
		Stack(Stack&& stack) noexcept;
		~Stack() = default;

	public:
		Stack& operator=(Stack&& stack) noexcept;

	public:
		void Allocate(std::size_t size);
		void Resize(std::size_t newSize);
		void Deallocate() noexcept;

		template<typename T>
		void Push(const T& value) {
			if (m_Used + sizeof(T) >= m_Size) throw std::runtime_error("Failed to push the value to the stack. Stack overflow.");
			*reinterpret_cast<T*>(m_Data.get() + m_Used) = value;
			m_Used += sizeof(T);
		}
		template<typename T>
		T Pop() {
			if (m_Used < sizeof(T)) throw std::runtime_error("Failed to pop the value from the stack. Not enough space.");
			T result = *reinterpret_cast<T*>(m_Data.get() + m_Used - sizeof(T));
			m_Used -= sizeof(T);
			return std::move(result);
		}
		template<typename T>
		T& Get(std::size_t offset) {
			if (offset + sizeof(T) < m_Size) throw std::runtime_error("Failed to get the value from the stack. Not enough space.");
			return *reinterpret_cast<T*>(m_Data.get() + offset);
		}
		std::size_t GetSize() const noexcept;
		std::size_t GetUsedSize() const noexcept;
	};

	class Interpreter final {
	private:
		ByteFile m_ByteFile;

		Stack m_Stack;

	public:
		Interpreter() noexcept = default;
		explicit Interpreter(ByteFile&& byteFile) noexcept;
		Interpreter(Interpreter&& interpreter) noexcept;
		~Interpreter() = default;

	public:
		Interpreter& operator=(Interpreter&& interpreter) noexcept;
		bool operator==(const ByteFile&) = delete;
		bool operator!=(const ByteFile&) = delete;

	public:
		void Clear() noexcept;
		void Load(ByteFile&& byteFile) noexcept;
		void AllocateStack(std::size_t size = 1 * 1024 * 1024);
		void ReallocateStack(std::size_t newSize);
	};
}