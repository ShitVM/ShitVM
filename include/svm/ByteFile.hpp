#pragma once

#include <svm/ConstantPool.hpp>
#include <svm/core/ByteFile.hpp>

#include <ostream>

namespace svm {
	namespace detail {
		class ByteFileAdapter : public core::ByteFile {
		public:
			using core::ByteFile::ByteFile;

		protected:
			inline const ConstantPool& GetWrappedConstantPool() const noexcept;
			inline ConstantPool& GetWrappedConstantPool() noexcept;

		private:
			using core::ByteFile::GetConstantPool;
		};
	}

	class ByteFile final : public detail::ByteFileAdapter {
	public:
		using detail::ByteFileAdapter::ByteFileAdapter;

	public:
		inline const ConstantPool& GetConstantPool() const noexcept;
		inline ConstantPool& GetConstantPool() noexcept;
	};

	inline std::ostream& operator<<(std::ostream& stream, const ByteFile& byteFile);
}

#include "detail/impl/ByteFile.hpp"