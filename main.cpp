#include <cstdlib>
#include <cstdint>
#include <cstdio>

#ifdef PLATFORM_WINDOWS

#include <io.h>

#define STDERR_FILENO 2

#define write(...) _write(__VA_ARGS__)

#else

#include <unistd.h>

#endif

#include <type_traits>

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

template <size_t message_size>
void write_error_and_exit(const char (&message)[message_size], int exitCode) noexcept {
	write(STDERR_FILENO, message, message_size - 1);
	std::exit(exitCode);
}

#define REPORT_ERROR_AND_EXIT(message, exitCode) write_error_and_exit("ERROR: " message "\n", exitCode)

uint64_t parseUInt64(const char* string) noexcept {
	if (string[0] == '\0') { REPORT_ERROR_AND_EXIT("<num-repeats> arg cannot be empty", EXIT_SUCCESS); }
	uint64_t result = string[0] - '0';
	if (result > 9) { REPORT_ERROR_AND_EXIT("<num-repeats> arg is invalid", EXIT_SUCCESS); }
	for (size_t i = 1; string[i] != '\0'; i++) {
		unsigned char digit = string[i] - '0';
		if (digit > 9) { REPORT_ERROR_AND_EXIT("<num-repeats> arg is invalid", EXIT_SUCCESS); }
		// NOTE: Optimal compiler optimization would be if the for loop gets unrolled so that the first digits (till digits<(uint64_t)-1>{} - 1) don't do any of the below.
		// NOTE: Additionally, the subtract and divide could be replaced by a switch case by the compiler.
		// TODO: Are there any other fancy ways to do this parsing stuff? Research.
		if (i >= digits<(uint64_t)-1>{} - 1 && ((uint64_t)-1 - digit) / 10 < result) { REPORT_ERROR_AND_EXIT("<num-repeats> arg too large", EXIT_SUCCESS); }
		result = result * 10 + digit;
	}
	return result;
}

int main(int argc, const char* const* argv) {
	if (argc < 3) { REPORT_ERROR_AND_EXIT("not enough args", EXIT_SUCCESS); }
	if (argc > 3) { REPORT_ERROR_AND_EXIT("too many args", EXIT_SUCCESS); }

	for (uint64_t i = 1; i <= parseUInt64(argv[2]); i++) {
		if (fputs(argv[1], stdout) == EOF) { REPORT_ERROR_AND_EXIT("failed to write to stdout", EXIT_FAILURE); }
	}
}
