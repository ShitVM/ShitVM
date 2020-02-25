#include <svm/Interpreter.hpp>
#include <svm/IO.hpp>
#include <svm/Parser.hpp>

#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		std::cout << "Usage: ./ShitVM <File>\n";
		return EXIT_FAILURE;
	}

	const auto defaultPrecision = std::cout.precision();

	const auto startParsing = std::chrono::system_clock::now();

	svm::Parser parser;
	parser.Load(argv[1]);
	parser.Parse();

	const auto endParsing = std::chrono::system_clock::now();
	const std::chrono::duration<double> parsing = endParsing - startParsing;

	svm::ByteFile byteFile = parser.GetResult();
	std::cout << "Parsed in " << std::fixed << std::setprecision(6) << parsing.count() << "s!\n"
			  << "Result:\n" << std::defaultfloat << svm::Indent << byteFile << "\n----------------------------------------\n";

	const auto startInterpreting = std::chrono::system_clock::now();

	svm::Interpreter i(std::move(byteFile));
	i.AllocateStack();
	i.Interpret();

	const auto endInterpreting = std::chrono::system_clock::now();
	const std::chrono::duration<double> interpreting = endInterpreting - startInterpreting;
	std::cout << "Interpreted in " << std::fixed << std::setprecision(6) << interpreting.count() << "s!\n"
			  << "Result: ";

	const auto resultType = i.GetResult<const svm::Type*>();
	if (resultType == svm::IntType) {
		std::cout << i.GetResult<svm::IntObject>().Value;
	} else if (resultType == svm::LongType) {
		std::cout << i.GetResult<svm::LongObject>().Value;
	} else if (resultType == svm::DoubleType) {
		std::cout << std::setprecision(defaultPrecision) << i.GetResult<svm::DoubleObject>().Value;
	}

	std::cout << "\n----------------------------------------\n"
			  << "Total used: " << std::fixed << std::setprecision(6) << parsing.count() + interpreting.count() << "s\n";

	return EXIT_SUCCESS;
}