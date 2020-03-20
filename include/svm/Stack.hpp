#pragma once

#include <svm/Type.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace svm {
	class Stack final {
	private:
		std::vector<std::uint8_t> m_Data;
		std::size_t m_Used = 0;

	public:
		Stack() noexcept = default;
		explicit Stack(std::size_t size);
		Stack(Stack&& stack) noexcept;
		~Stack() = default;

	public:
		Stack& operator=(Stack&& stack) noexcept;
		bool operator==(const Stack&) = delete;
		bool operator!=(const Stack&) = delete;

	public:
		void Allocate(std::size_t size);
		void Reallocate(std::size_t newSize);
		void Deallocate() noexcept;

		template<typename T>
		bool Push(const T& value) noexcept;
		template<typename T>
		std::optional<T> Pop() noexcept;
		template<typename T>
		const T* Get(std::size_t offset) const noexcept;
		template<typename T>
		T* Get(std::size_t offset) noexcept;
		template<typename T>
		const T* GetTop() const noexcept;
		template<typename T>
		T* GetTop() noexcept;
		const Type* GetTopType() const noexcept;
		Type* GetTopType() noexcept;

		std::size_t GetSize() const noexcept;
		std::size_t GetUsedSize() const noexcept;
		void SetUsedSize(std::size_t newUsedSize) noexcept;
		std::size_t GetFreeSize() const noexcept;

		bool Expand(std::size_t delta) noexcept;
		void Reduce(std::size_t delta) noexcept;

		const std::uint8_t* Begin() const noexcept;
		std::uint8_t* Begin() noexcept;
		const std::uint8_t* Last() const noexcept;
		std::uint8_t* Last() noexcept;
	};
}

#include "detail/impl/Stack.hpp"