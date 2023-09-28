#pragma once

#include <svm/core/ConstantPool.hpp>

#include <ostream>

namespace svm {
	class ConstantPool final : public core::ConstantPool {
	private:
		using core::ConstantPool::NPos;

	public:
		using core::ConstantPool::ConstantPool;

	private:
		using core::ConstantPool::AddIntConstant;
		using core::ConstantPool::AddLongConstant;
		using core::ConstantPool::AddSingleConstant;
		using core::ConstantPool::AddDoubleConstant;
		using core::ConstantPool::FindIntConstant;
		using core::ConstantPool::FindLongConstant;
		using core::ConstantPool::FindSingleConstant;
		using core::ConstantPool::FindDoubleConstant;
	};

	inline std::ostream& operator<<(std::ostream& stream, const ConstantPool& constantPool);
}

#include "detail/impl/ConstantPool.hpp"