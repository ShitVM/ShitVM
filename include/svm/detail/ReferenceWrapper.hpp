#pragma once

#include <cstddef>

namespace svm::detail {
	template<typename T>
	class ReferenceWrapper {
	private:
		const T* m_Data = nullptr;

	public:
		ReferenceWrapper() noexcept = default;
		ReferenceWrapper(std::nullptr_t) noexcept {}
		ReferenceWrapper(const T& data) noexcept
			: m_Data(&data) {}
		ReferenceWrapper(const ReferenceWrapper& wrapper) noexcept
			: m_Data(wrapper.m_Data) {}
		~ReferenceWrapper() = default;

	public:
		ReferenceWrapper& operator=(const ReferenceWrapper& wrapper) noexcept {
			m_Data = wrapper.m_Data;
			return *this;
		}
		bool operator==(const ReferenceWrapper& wrapper) const noexcept {
			return m_Data == wrapper.m_Data;
		}
		bool operator!=(const ReferenceWrapper& wrapper) const noexcept {
			return m_Data != wrapper.m_Data;
		}
		const T& operator*() const noexcept {
			return *m_Data;
		}
		const T* operator->() const noexcept {
			return m_Data;
		}

	public:
		bool IsEmpty() const noexcept {
			return m_Data == nullptr;
		}
		const T& GetReference() const noexcept {
			return *m_Data;
		}
		const T* GetPointer() const noexcept {
			return m_Data;
		}
	};
}