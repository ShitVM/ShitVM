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
	const bool success = i.Interpret();

	const auto endInterpreting = std::chrono::system_clock::now();
	const std::chrono::duration<double> interpreting = endInterpreting - startInterpreting;

	if (!success) {
		std::cout << "Occured exception!\n"
				  << "Code: " << i.GetException().Code << '\n';
		return EXIT_FAILURE;
	}

	std::cout << "Interpreted in " << std::fixed << std::setprecision(6) << interpreting.count() << "s!\n"
			  << "Result: ";

	const auto result = i.GetResult();
	if (std::holds_alternative<std::uint32_t>(result)) {
		std::cout << std::get<std::uint32_t>(result);
	} else if (std::holds_alternative<std::uint64_t>(result)) {
		std::cout << std::get<std::uint64_t>(result);
	} else if (std::holds_alternative<double>(result)) {
		std::cout << std::get<double>(result);
	}

	std::cout << "\n----------------------------------------\n"
			  << "Total used: " << std::fixed << std::setprecision(6) << parsing.count() + interpreting.count() << "s\n";

	return EXIT_SUCCESS;
}