#include <svm/Interpreter.hpp>
#include <svm/Parser.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		std::cout << "Usage: ./ShitVM <File>\n";
		return EXIT_FAILURE;
	}

	const auto start = std::chrono::system_clock::now();

	svm::Parser parser;
	parser.Load(argv[1]);
	parser.Parse();

	svm::Interpreter i(parser.GetResult());
	i.AllocateStack();
	i.Interpret();

	const auto end = std::chrono::system_clock::now();
	const std::chrono::duration<double> runtime = end - start;

	const auto resultType = i.GetResult<const svm::Type*>();
	if (resultType == svm::IntType) {
		std::cout << "Result: " << i.GetResult<svm::IntObject>().Value;
	} else if (resultType == svm::LongType) {
		std::cout << "Result: " << i.GetResult<svm::LongObject>().Value;
	} else if (resultType == svm::DoubleType) {
		std::cout << "Result: " << i.GetResult<svm::DoubleObject>().Value;
	}

	std::cout << "\nRuntime: " << runtime.count() << "s\n";

	return EXIT_SUCCESS;
}