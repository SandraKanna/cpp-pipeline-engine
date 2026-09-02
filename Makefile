# Convenience wrapper over the project's CMake presets and quality tools.
# Not part of the build system: the build itself works without this file
# (see CMakePresets.json).

.PHONY: build run test ci release-gcc release-clang clean re format format-check tidy

build:
# configure (cached, so this is a no-op if nothing changed), then compile
	cmake --preset dev
	cmake --build --preset dev

# depends on build so the binary exists and is up to date before running it
run: build
	./build/dev/cpe_skeleton

test:
	ctest --preset dev

release-gcc:
	cmake --preset release-gcc && cmake --build --preset release-gcc

release-clang:
	cmake --preset release-clang && cmake --build --preset release-clang

# run the complete ci workflow: check compilation with both compilers + perform GoogleTests
ci: release-gcc release-clang tidy test

clean:
	rm -rf build

re: clean build

FORMAT_SRC = src/*.cpp include/cpe/*.hpp tests/*.cpp

# apply clang-format to all source files, editing them in place
format:
	@printf "This will reformat files in place. Continue? [y/N] "
	@read ans; [ "$$ans" = "y" ] || [ "$$ans" = "Y" ] || { echo "Aborted."; exit 1; }
	clang-format -i $(FORMAT_SRC)

# check formatting without modifying anything; fails if a file is not formatted
format-check:
	clang-format --dry-run --Werror $(FORMAT_SRC)

CLANG_TIDY = /opt/homebrew/opt/llvm/bin/clang-tidy
TIDY_SRC = src/bytes.cpp src/data_model.cpp src/acquisition/buffer_acquisition.cpp src/acquisition/file_acquisition.cpp \
		   src/deserialization/line_delimitation.cpp src/deserialization/raw_parsing.cpp src/processing/processing.cpp \
		   src/serialization/raw_serialization.cpp src/engine.cpp \
		   tests/data_model_test.cpp tests/buffer_acquisition_test.cpp tests/file_acquisition_test.cpp \
		   tests/line_delimitation_test.cpp tests/raw_parsing_test.cpp tests/raw_serialization_test.cpp tests/engine_test.cpp

# static analysis: likely bugs, modernization opportunities, performance issues, and
# naming/readability conventions. It needs the compilation database (from a configured build)
# and, on macOS, the SDK path so non-Apple clang-tidy finds the system headers.
tidy: build
	$(CLANG_TIDY) -p build/dev \
		--extra-arg=-isysroot --extra-arg=$(shell xcrun --show-sdk-path) \
		$(TIDY_SRC)
