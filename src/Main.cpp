#include <svm/Interpreter.hpp>
#include <svm/IO.hpp>
#include <svm/Parser.hpp>
#include <svm/ProgramOption.hpp>
#include <svm/Version.hpp>
#include <svm/gc/SimpleGarbageCollector.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>

int main(int argc, char* argv[]) {
	svm::ProgramOption option;
	option.AddVariable("stack", 1 * 1024 * 1024)
		  .AddVariable("young", 8 * 1024 * 1024)
		  .AddVariable("old", 32 * 1024 * 1024)
		  .AddFlag("gc", true);

	if (!option.Parse(argc, argv) || !option.Verity()) {
		return EXIT_FAILURE;
	} else if (option.ShowVersion) {
		std::cout << "ShitVM " << SVM_VER_STRING << '\n'
				  << "(C) 2020. kmc7468 All rights reserved.\n\n"
				  << "You can get the source of latest ShitVM here:\n"
				  << "https://github.com/ShitVM\n";
		return EXIT_SUCCESS;
	}

	std::cout << "----------------------------------------\n";

	const auto startParsing = std::chrono::system_clock::now();

	svm::Parser parser;
	parser.Load(option.Path);
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
	interpreter.AllocateStack(static_cast<std::size_t>(option.GetVariable("stack")));
	if (option.GetFlag("gc")) {
		interpreter.SetGarbageCollector(std::make_unique<svm::SimpleGarbageCollector>(
			static_cast<std::size_t>(option.GetVariable("young")), static_cast<std::size_t>(option.GetVariable("old"))));
	}
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
			if (frame.Function == nullptr) {
				std::cout << '\n';
			} else {
				std::cout << " at\n";
			}
		}

		return EXIT_FAILURE;
	}

	std::cout << "Interpreted in " << std::fixed << std::setprecision(6) << interpreting.count() << "s!";
	if (interpreter.HasResult()) {
		std::cout << "\nResult: " << std::defaultfloat;

		const svm::Object* result = interpreter.GetResult();
		interpreter.PrintObject(std::cout, result, true);
	}

	std::cout << "\n----------------------------------------\n"
			  << "Total used: " << std::fixed << std::setprecision(6) << parsing.count() + interpreting.count() << "s\n";

	return EXIT_SUCCESS;
}