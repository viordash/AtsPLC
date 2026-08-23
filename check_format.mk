CF_SOURCE_DIRS := $(shell find -L $(CF_SRC) $(foreach dir,$(CF_EXCLUDE_DIRS),-path '$(dir)' -prune -o) -type d -print)

# define the C header files
CF_HEADERS		:= $(wildcard $(patsubst %,%/*.h, $(CF_SOURCE_DIRS)))
# define the C source files
CF_SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(CF_SOURCE_DIRS)))
# define the CPP source files
CF_SOURCES_CXX		:= $(wildcard $(patsubst %,%/*.cpp, $(CF_SOURCE_DIRS)))

apply_format:
	@clang-format --style=file:$(ROOT_DIR)/.clang-format $(CF_SOURCES) $(CF_SOURCES_CXX) $(CF_HEADERS) -i

check_format:
	@echo "----------------------- check_format"
	@clang-format --style=file:$(ROOT_DIR)/.clang-format $(CF_SOURCES) $(CF_SOURCES_CXX) $(CF_HEADERS) --Werror --dry-run

code_check:
	@echo "----------------------- Static analysis"
	@CodeChecker check --build "make -j$$(nproc) project_prepare && make -j$$(nproc) clean && make -j$$(nproc) all" --analyzers cppcheck \
		--no-missing-checker-error \
		--disable android-cloexec-accept \
		--skip CodeChecker.skipfile \
		--output build/CodeChecker/check_static/reports --clean	|| true
	CodeChecker parse --export html --output build/CodeChecker/reports/check_static build/CodeChecker/check_static/reports > /dev/null 2>&1	