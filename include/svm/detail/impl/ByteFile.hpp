#pragma once
#include <svm/ByteFile.hpp>

namespace svm {
	namespace detail {
		inline const ConstantPool& ByteFileAdapter::GetWrappedConstantPool() const noexcept {
			return static_cast<const ConstantPool&>(GetConstantPool());
		}
		inline ConstantPool& ByteFileAdapter::GetWrappedConstantPool() noexcept {
			return static_cast<ConstantPool&>(GetConstantPool());
		}
	}

	inline const ConstantPool& ByteFile::GetConstantPool() const noexcept {
		return GetWrappedConstantPool();
	}
	inline ConstantPool& ByteFile::GetConstantPool() noexcept {
		return GetWrappedConstantPool();
	}

	inline std::ostream& operator<<(std::ostream& stream, const ByteFile& byteFile) {
		return core::operator<<(stream, byteFile);
	}
}