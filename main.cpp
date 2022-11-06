#include <cstdio>

#define STDIN_BUFFER_SIZE BUFSIZ

#include <cstdlib>
#include <cstdint>

#include <cstring>				// for std::strcmp()
#include <string>

#ifdef PLATFORM_WINDOWS

#include <io.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

using sioret_t = int;

#define read(...) _read(__VA_ARGS__)
#define write(...) _write(__VA_ARGS__)

#else

#include <unistd.h>

using sioret_t = ssize_t;

#endif

#include <type_traits>

constexpr char help_text[] = R"(usage: repeat <num-repeats>
       repeat <--help>

description: repeats everything it read on stdin a specified amount of times after stdin closes
)";

[[noreturn]] void halt_program(int exit_code) noexcept {
	std::exit(exit_code);
}

template <size_t message_size>
[[noreturn]] void write_error_and_exit(const char (&message)[message_size], int exitCode) noexcept {
	write(STDERR_FILENO, message, message_size - 1);
	halt_program(exitCode);
}

#define REPORT_ERROR_AND_EXIT(message, exitCode) write_error_and_exit("ERROR: " message "\n", exitCode)

[[noreturn]] void show_help_and_exit() noexcept {
	if (write(STDOUT_FILENO, help_text, sizeof(help_text) - 1) == -1) {
		REPORT_ERROR_AND_EXIT("failed to output help text to stdout", EXIT_FAILURE);
	}
	halt_program(EXIT_SUCCESS);
}

template <typename integral_t, integral_t num>
struct integer {
	using type = integral_t;
	static constexpr integral_t value = num;
};

template <typename integral_t>
struct calc_digits {
	static constexpr uint8_t value = calc_digits<integer<typename integral_t::type, integral_t::value / 10>>::value + 1;
};

template <typename actual_integral_t>
struct calc_digits<integer<actual_integral_t, 0>> {
	static constexpr uint8_t value = 0;
};

template <auto num, typename std::enable_if<std::is_integral<decltype(num)>{}, bool>::type = false>
struct digits {
	static constexpr uint8_t value = calc_digits<integer<decltype(num), num>>::value;

	constexpr operator uint8_t() const noexcept { return value; }
};

uint64_t parseUInt64(const char* string_input) noexcept {
	if (string_input[0] == '\0') { REPORT_ERROR_AND_EXIT("<num-repeats> arg cannot be empty", EXIT_SUCCESS); }

	const unsigned char* string = (const unsigned char*)string_input;
	// NOTE: SIGNED INTEGER OVERFLOW IS UNDEFINED, AVOID IT AT ALL COSTS!

	uint64_t result = string[0] - (unsigned char)'0';
	if (result > 9) { REPORT_ERROR_AND_EXIT("<num-repeats> arg is invalid", EXIT_SUCCESS); }

	for (size_t i = 1; string[i] != '\0'; i++) {
		if (i >= digits<(uint64_t)-1>{}) { REPORT_ERROR_AND_EXIT("<num-repeats> arg too large", EXIT_SUCCESS); }

		unsigned char digit = string[i] - (unsigned char)'0';
		if (digit > 9) { REPORT_ERROR_AND_EXIT("<num-repeats> arg is invalid", EXIT_SUCCESS); }

		// NOTE: Optimal compiler optimization would be if the for loop gets unrolled so that the first digits don't have to do
		// the overflow check, which obviously only comes into play once the number gets higher.
		// Also, it would be cool if the i check dissappears when the loop is unrolled, since that is also possible.

		uint64_t shifted_result = result * 10;
		if (shifted_result / 10 != result) { REPORT_ERROR_AND_EXIT("<num-repeats> arg too large", EXIT_SUCCESS); }

		result = shifted_result + digit;
		if (result < shifted_result) { REPORT_ERROR_AND_EXIT("<num-repeats> arg too large", EXIT_SUCCESS); }
	}

	return result;
}

int main(int argc, const char* const* argv) {
	if (argc < 2) { REPORT_ERROR_AND_EXIT("not enough args", EXIT_SUCCESS); }
	if (argc > 2) { REPORT_ERROR_AND_EXIT("too many args", EXIT_SUCCESS); }

	if (argv[1][0] == '-') { 
		if (std::strcmp(argv[1] + 1, "-help") == 0) { show_help_and_exit(); }
		REPORT_ERROR_AND_EXIT("one or more invalid flags", EXIT_FAILURE);
	}

	uint64_t amount_of_repeats = parseUInt64(argv[1]);

	std::string input;
	while (true) {
		size_t current_input_size = input.size();
		input.resize(current_input_size + STDIN_BUFFER_SIZE);
		sioret_t bytes_read = read(STDIN_FILENO, (void*)(input.data() + current_input_size), STDIN_BUFFER_SIZE);
		if (bytes_read == -1) { REPORT_ERROR_AND_EXIT("failed to read from stdin", EXIT_FAILURE); }
		input.resize(current_input_size + bytes_read);
		if (bytes_read == 0) { break; }
	}

	for (uint64_t i = 1; i <= amount_of_repeats; i++) {
		if (fputs(input.c_str(), stdout) == EOF) { REPORT_ERROR_AND_EXIT("failed to write to stdout", EXIT_FAILURE); }
	}
}
