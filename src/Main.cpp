#include <svm/Interpreter.hpp>
#include <svm/IO.hpp>
#include <svm/Parser.hpp>
#include <svm/gc/SimpleGarbageCollector.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		std::cout << "Usage: ./ShitVM <File>\n";
		return EXIT_FAILURE;
	}

	const auto startParsing = std::chrono::system_clock::now();

	svm::Parser parser;
	parser.Load(argv[1]);
	try {
		parser.Parse();
	} catch (const std::exception & e) {
		std::cout << "Occured exception!\n"
				  << "Message: \"" << e.what() << "\"\n";
		return EXIT_FAILURE;
	}

	const auto endParsing = std::chrono::system_clock::now();
	const std::chrono::duration<double> parsing = endParsing - startParsing;

	svm::ByteFile byteFile = parser.GetResult();
	std::cout << "Parsed in " << std::fixed << std::setprecision(6) << parsing.count() << "s!\n"
			  << "Result:\n" << std::defaultfloat << svm::Indent << byteFile << "\n----------------------------------------\n";

	const auto startInterpreting = std::chrono::system_clock::now();

	svm::Interpreter interpreter(std::move(byteFile));
	interpreter.AllocateStack();
	interpreter.SetGarbageCollector(std::make_unique<svm::SimpleGarbageCollector>(8 * 1024 * 1024, 32 * 1024 * 1024));
	const bool success = interpreter.Interpret();

	const auto endInterpreting = std::chrono::system_clock::now();
	const std::chrono::duration<double> interpreting = endInterpreting - startInterpreting;

	if (!success) {
		const auto& exception = interpreter.GetException();
		const auto callStacks = interpreter.GetCallStacks();
		const auto& funcs = interpreter.GetByteFile().GetFunctions();

		std::cout << "Occured exception!\n"
				  << "Message: \"" << svm::GetInterpreterExceptionMessage(exception.Code) << "\"\n"
				  << "CallStacks:\n";
		for (const auto& frame : callStacks) {
			using namespace svm;

			if (frame.Function == nullptr) {
				std::cout << "\tentrypoint";
			} else {
				const auto iter = std::find_if(funcs.begin(), funcs.end(), [&frame](const auto& func) {
					return &func == frame.Function;
				});
				const auto dis = std::distance(funcs.begin(), iter);
				std::cout << "\t[" << dis << ']';
			}
			std::cout << '(' << frame.Caller
					  << '(' << QWord(frame.Instructions->GetInstruction(frame.Caller).Offset) << "))";
			if (frame.Function != nullptr) {
				std::cout << " at\n";
			}
		}

		return EXIT_FAILURE;
	}

	std::cout << "Interpreted in " << std::fixed << std::setprecision(6) << interpreting.count() << "s!\n"
			  << "Result: " << std::defaultfloat;

	const svm::Object* result = interpreter.GetResult();
	interpreter.PrintObject(std::cout, result);

	std::cout << "\n----------------------------------------\n"
			  << "Total used: " << std::fixed << std::setprecision(6) << parsing.count() + interpreting.count() << "s\n";

	return EXIT_SUCCESS;
}