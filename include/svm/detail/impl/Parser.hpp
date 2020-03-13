#pragma once
#include <svm/Parser.hpp>

#include <svm/Memory.hpp>
#include <svm/Object.hpp>

#include <type_traits>
#include <utility>

namespace svm {
	template<typename T>
	T Parser::ReadFile() noexcept {
		T result = reinterpret_cast<T&>(m_File[m_Pos]);
		m_Pos += sizeof(T);

		if (sizeof(T) > 1 && GetEndian() != Endian::Little) return ReverseEndian(result);
		else return result;
	}
	inline auto Parser::ReadFile(std::size_t size) noexcept {
		const auto begin = m_File.begin() + m_Pos;
		const auto end = m_File.begin() + m_Pos + size;
		m_Pos += size;
		return std::make_pair(begin, end);
	}

	template<typename T>
	void Parser::ParseConstants(std::vector<T>& pool) noexcept {
		static_assert(std::is_base_of_v<Object, T>);
		for (T& obj : pool) {
			obj.Value = ReadFile<decltype(obj.Value)>();
		}
	}
}