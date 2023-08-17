#pragma once

namespace svm::detail {
	template<typename... F>
	struct Overload : F... {
		using F::operator()...;
	};
	template<typename... F>
	Overload(F...)->Overload<F...>;
}