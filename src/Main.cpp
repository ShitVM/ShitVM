#include <svm/Interpreter.hpp>
#include <svm/IO.hpp>
#include <svm/Parser.hpp>
#include <svm/jit/Native.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>

using namespace svm;
using namespace svm::jit::native;

int main(int argc, char* argv[]) {
	Builder b;

	std::uint8_t buffer[10000];
	std::size_t size = b.GetResult(buffer);

	return EXIT_SUCCESS;
}