#include <svm/Interpreter.hpp>
#include <svm/IO.hpp>
#include <svm/Parser.hpp>

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

	const auto defaultPrecision = std::cout.precision();

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

	svm::Interpreter i(std::move(byteFile));
	i.AllocateStack();
	const bool success = i.Interpret();

	const auto endInterpreting = std::chrono::system_clock::now();
	const std::chrono::duration<double> interpreting = endInterpreting - startInterpreting;

	if (!success) {
		const auto& exception = i.GetException();
		auto callStacks = i.GetCallStacks();
		callStacks[0].Caller = static_cast<std::size_t>(exception.InstructionIndex);
		const auto& funcs = i.GetByteFile().GetFunctions();

		std::cout << "Occured exception!\n"
				  << "Message: \"" << svm::GetInterpreterExceptionMessage(exception.Code) << "\"\n"
				  << "Function: ";
		if (exception.Function == nullptr) {
			std::cout << "entrypoint";
		} else {
			const auto iter = std::find_if(funcs.begin(), funcs.end(), [&](const auto& func) {
				return &func == exception.Function;
			});
			const auto dis = std::distance(funcs.begin(), iter);
			std::cout << '[' << dis << ']';
		}
		std::cout << "\nCallStacks:\n";
		for (const auto& frame : callStacks) {
			if (frame.Function == nullptr) {
				std::cout << "\tentrypoint";
			} else {
				const auto iter = std::find_if(funcs.begin(), funcs.end(), [&](const auto& func) {
					return &func == frame.Function;
					});
				const auto dis = std::distance(funcs.begin(), iter);
				std::cout << "\t[" << dis << ']';
			}
			std::cout << '(' << frame.Caller
					  << '(' << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << frame.Instructions->GetInstruction(frame.Caller).Offset
					  << std::dec << std::nouppercase << "))";
			if (frame.Function != nullptr) {
				std::cout << " at\n";
			}
		}

		return EXIT_FAILURE;
	}

	std::cout << "Interpreted in " << std::fixed << std::setprecision(6) << interpreting.count() << "s!\n"
			  << "Result: ";

	const auto result = i.GetResult();
	if (std::holds_alternative<std::monostate>(result)) {
		std::cout << "none";
	} else if (std::holds_alternative<std::uint32_t>(result)) {
		std::cout << std::get<std::uint32_t>(result);
	} else if (std::holds_alternative<std::uint64_t>(result)) {
		std::cout << std::get<std::uint64_t>(result);
	} else if (std::holds_alternative<double>(result)) {
		std::cout << std::get<double>(result);
	} else if (std::holds_alternative<const svm::StructureObject*>(result)) {
		std::cout << std::get<const svm::StructureObject*>(result)->GetType()->Name;
	}

	std::cout << "\n----------------------------------------\n"
			  << "Total used: " << std::fixed << std::setprecision(6) << parsing.count() + interpreting.count() << "s\n";

	return EXIT_SUCCESS;
}