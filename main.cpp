#include <cstdio>			// literally just so that we can use BUFSIZ

// NOTE: You may want to consider changing these to something larger, because it seems that bigger is better.
#define STDIN_BUFFER_SIZE BUFSIZ	// This upper one is actually the buffer step size. As in the step size with which the buffer gets larger.
#define STDOUT_MIN_BUFFER_SIZE BUFSIZ	// This lower one is the minimum size that the buffer needs to have. It can be much much larger though, it depends on how much text needs to be stored.
					// If the buffer isn't as big as the minimum size, it's concatinated with itself until the resulting buffer is at least as large as the minimum size.

#include <cstdlib>
#include <cstdint>

#include <cstring>			// for std::strcmp() and std::memcpy()
#include <string>

#ifdef PLATFORM_WINDOWS

#include <io.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

using sioret_t = int;

#define crossplatform_read(...) _read(__VA_ARGS__)
#define crossplatform_write(...) _write(__VA_ARGS__)

#else

#include <unistd.h>

using sioret_t = ssize_t;

#define crossplatform_read(...) read(__VA_ARGS__)
#define crossplatform_write(...) write(__VA_ARGS__)

#endif

#include <type_traits>

constexpr char help_text[] = R"(usage: repeat <num-repeats>
       repeat --help

description: repeats everything it read on stdin a specified amount of times after stdin closes

note: Exits with EXIT_SUCCESS if everything went according to plan and with EXIT_FAILURE if an error occurred.
      An error message is returned on stderr if an error occurs.
)";

[[noreturn]] void halt_program(int exit_code) noexcept {
	std::exit(exit_code);
	while (true) { }
}

template <size_t message_size>
[[noreturn]] void write_error_and_exit(const char (&message)[message_size], int exit_code) noexcept {
	crossplatform_write(STDERR_FILENO, message, message_size - sizeof(char));
	halt_program(exit_code);
}

#define REPORT_ERROR_AND_EXIT(message, exitCode) write_error_and_exit("ERROR: " message "\n", exitCode)

[[noreturn]] void show_help_and_exit() noexcept {
	if (crossplatform_write(STDOUT_FILENO, help_text, sizeof(help_text) - sizeof(char)) == false) {
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

template <auto num, typename std::enable_if<std::is_integral<decltype(num)> { }, bool>::type = true>
struct digits {
	static constexpr uint8_t value = calc_digits<integer<decltype(num), num>>::value;

	constexpr operator uint8_t() const noexcept { return value; }
};

uint64_t parseUInt64(const char* string_input) noexcept {
	if (string_input[0] == '\0') { REPORT_ERROR_AND_EXIT("<num-repeats> arg cannot be empty", EXIT_FAILURE); }

	const unsigned char* string = (const unsigned char*)string_input;
	// NOTE: SIGNED INTEGER OVERFLOW IS UNDEFINED, AVOID IT AT ALL COSTS!

	uint64_t result = string[0] - '0';
	if (result > 9) { REPORT_ERROR_AND_EXIT("<num-repeats> arg is invalid", EXIT_FAILURE); }

	for (size_t i = 1; string[i] != '\0'; i++) {
		if (i == digits<(uint64_t)-1>()) { REPORT_ERROR_AND_EXIT("<num-repeats> arg too large", EXIT_FAILURE); }

		const unsigned char digit = string[i] - '0';
		if (digit > 9) { REPORT_ERROR_AND_EXIT("<num-repeats> arg is invalid", EXIT_FAILURE); }

		// NOTE: Optimal compiler optimization would be if the for loop gets unrolled so that the first digits don't have to do
		// the overflow check, which obviously only comes into play once the number gets higher.
		// Also, it would be cool if the i check dissappears when the loop is unrolled, since that is also possible.

		const uint64_t shifted_result = result * 10;
		if (shifted_result / 10 != result) { REPORT_ERROR_AND_EXIT("<num-repeats> arg too large", EXIT_FAILURE); }

		result = shifted_result + digit;
		if (result < shifted_result) { REPORT_ERROR_AND_EXIT("<num-repeats> arg too large", EXIT_FAILURE); }
	}

	return result;
}

bool write_everything(int fd, const char *buf, size_t count) noexcept {
	while (count != 0) {
		sioret_t result = crossplatform_write(fd, buf, count);
		if (result == -1) { return false; }
		buf += result;
		count -= result;
	}
	return true;
}

int main(int argc, const char* const* argv) {
	if (argc < 2) { REPORT_ERROR_AND_EXIT("not enough args", EXIT_FAILURE); }
	if (argc > 2) { REPORT_ERROR_AND_EXIT("too many args", EXIT_FAILURE); }

	if (argv[1][0] == '-') {
		if (std::strcmp(argv[1] + 1, "-help") == 0) { show_help_and_exit(); }
		REPORT_ERROR_AND_EXIT("one or more invalid flags", EXIT_FAILURE);
	}

	const uint64_t amount_of_repeats = parseUInt64(argv[1]);

	// TODO: Using stdlib string is a problem because it doesn't tell us if it runs out of memory, it throws an exception, which will end the program.
	// The best way to make this better is to create your own simple string class that'll handle it correctly.
	// If we run out of memory, we should work with the buffer that we DO have and still output everything correctly, just in a slower fashion.
	std::string input;

	size_t current_input_size = 0;
	while (true) {
		input.resize(current_input_size + STDIN_BUFFER_SIZE);
		sioret_t bytes_read = crossplatform_read(STDIN_FILENO, input.data() + current_input_size, STDIN_BUFFER_SIZE);
		if (bytes_read == -1) { REPORT_ERROR_AND_EXIT("failed to read from stdin", EXIT_FAILURE); }
		if (bytes_read == 0) { break; }
		current_input_size += bytes_read;
	}
	input.resize(current_input_size);

	const size_t input_original_length = current_input_size;
	size_t amount_of_default_instances = 1;
	size_t input_new_length;
	while ((input_new_length = input.length()) < STDOUT_MIN_BUFFER_SIZE) {
		input.resize(input_new_length + input_original_length);
		std::memcpy(input.data() + input_new_length, input.c_str(), input_original_length);
		amount_of_default_instances++;
	}

	const uint64_t amount_of_full_chunks = amount_of_repeats / amount_of_default_instances;
	const size_t amount_of_rest_units = amount_of_repeats % amount_of_default_instances;

	for (uint64_t i = 0; i < amount_of_full_chunks; i++) {
		if (write_everything(STDOUT_FILENO, input.c_str(), input_new_length) == false) { REPORT_ERROR_AND_EXIT("failed to write to stdout", EXIT_FAILURE); }
	}
	if (write_everything(STDOUT_FILENO, input.c_str(), input_original_length * amount_of_rest_units) == false) { REPORT_ERROR_AND_EXIT("failed to write to stdout", EXIT_FAILURE); }
}
