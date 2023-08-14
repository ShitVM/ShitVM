#include <svm/Interpreter.hpp>
#include <svm/IO.hpp>
#include <svm/Parser.hpp>
#include <svm/ProgramOption.hpp>
#include <svm/Version.hpp>
#include <svm/core/Version.hpp>
#include <svm/gc/SimpleGarbageCollector.hpp>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <variant>

int Run(const svm::ProgramOption& option);

int main(int argc, char* argv[]) {
	svm::ProgramOption option;
	option.AddOption("version")
		  .AddOption("dump-bytefile")
		  .AddVariable("stack", 1 * 1024 * 1024)
		  .AddVariable("young", 8 * 1024 * 1024)
		  .AddVariable("old", 32 * 1024 * 1024)
		  .AddFlag("gc", true);

	if (!option.Parse(argc, argv) || !option.Verity()) {
		return EXIT_FAILURE;
	} else if (option.GetOption("version")) {
		std::cout << "ShitVM " << svm::Version << '\n'
				  << "(C) 2020. kmc7468 All rights reserved.\n"
				  << "This version is based on ShitCore " << svm::core::Version << "\n\n"
				  << "You can get the source of latest ShitVM here:\n"
				  << "https://github.com/ShitVM\n";
		return EXIT_SUCCESS;
	}

	std::cout << "----------------------------------------\n";
	return Run(option);
}

int Run(const svm::ProgramOption& option) {
	std::cout << "Started loading...\n";
	const auto startLoading = std::chrono::system_clock::now();

	svm::Loader loader;
	svm::StdModule stdModule;
	svm::Module program;
	try {
		stdModule = svm::InitStdModule(loader);
		program = loader.Load(option.Path);
	} catch (const std::exception& e) {
		std::cout << "Occured exception!\n"
				  << "Message: \"" << e.what() << "\"\n";
		return EXIT_FAILURE;
	}

	const auto endLoading = std::chrono::system_clock::now();
	const std::chrono::duration<double> loading = endLoading - startLoading;
	std::cout << "Succeed loading in " << std::fixed << std::setprecision(6) << loading.count() << "s!\n";
	
	if (option.GetOption("dump-bytefile")) {
		std::cout << svm::Indent << std::get<svm::core::ByteFile>(program->Module) << svm::UnIndent << '\n';
	}

	std::cout << "Started interpreting...\n";
	const auto startInterpreting = std::chrono::system_clock::now();

	svm::Interpreter interpreter(std::move(loader), program);
	interpreter.AllocateStack(static_cast<std::size_t>(option.GetVariable("stack")));
	if (option.GetFlag("gc")) {
		interpreter.SetGarbageCollector(std::make_unique<svm::SimpleGarbageCollector>(
			static_cast<std::size_t>(option.GetVariable("young")), static_cast<std::size_t>(option.GetVariable("old"))));
	}

	if (!interpreter.Interpret()) {
		const auto& exception = interpreter.GetException();
		const auto callStacks = interpreter.GetCallStacks();
		std::cout << "Occured exception!\n"
				  << "Message: \"" << svm::GetInterpreterExceptionMessage(exception.Code) << "\"\n"
				  << "Call stacks:\n";

		for (const auto& frame : callStacks) {
			if (std::holds_alternative<std::monostate>(frame.Function)) {
				std::cout << "\tentrypoint";
			} else {
				for (std::uint32_t i = 0; i < interpreter.GetFunctionCount(); ++i) {
					if (frame.Function == interpreter.GetFunction(i)) {
						std::cout << "\t[" << i << ']';
						break;
					}
				}
			}

			if (!std::holds_alternative<svm::VirtualFunction>(frame.Function)) {
				using namespace svm;
				std::cout << '(' << frame.Caller << '(' << QWord(frame.Instructions->GetInstruction(frame.Caller).Offset) << "))";
			}
			if (!std::holds_alternative<std::monostate>(frame.Function)) {
				std::cout << " at";
			}
			std::cout << '\n';
		}

		return EXIT_FAILURE;
	}

	const auto endInterpreting = std::chrono::system_clock::now();
	const std::chrono::duration<double> interpreting = endInterpreting - startInterpreting;
	std::cout << "Succeed interpreting in " << std::fixed << std::setprecision(6) << interpreting.count() << "s!\n";

	if (interpreter.HasResult()) {
		std::cout << "\tResult: " << std::defaultfloat;

		const svm::Object* result = interpreter.GetResult();
		interpreter.PrintObject(std::cout, result, true);
	}

	std::cout << "\n----------------------------------------\n"
			  << "Total used: " << std::fixed << std::setprecision(6) << loading.count() + interpreting.count() << "s\n";

	return EXIT_SUCCESS;

}