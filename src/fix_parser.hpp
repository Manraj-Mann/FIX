// Minimal, high-performance, zero-copy FIX parser utilities (header-only)
#pragma once

#include <cstddef>
#include <cstdint>

namespace fix {

// Templated header-only parser to avoid std::function and virtual calls.
// Callback can be any callable: function pointer, lambda, functor — it will be inlined.
// Calls `cb(msg_ptr, msg_len)` for each complete FIX message found.
// Messages are considered complete when the checksum tag "10=" is followed by SOH (0x01).
// Returns the number of bytes consumed from the start of the buffer (so the caller can erase them).
template <typename Callback>
inline size_t parse_messages(const char* data, size_t len, Callback&& cb) {
	constexpr unsigned char SOH = 0x01;
	size_t i = 0;
	size_t last_consumed = 0;

	while (i + 3 <= len) {
		// find substring "10="
		bool found = false;
		size_t pos = i;
		for (; pos + 2 < len; ++pos) {
			if (data[pos] == '1' && data[pos+1] == '0' && data[pos+2] == '=') { found = true; break; }
		}
		if (!found) break;

		// find next SOH after checksum value
		size_t j = pos + 3;
		while (j < len && static_cast<unsigned char>(data[j]) != SOH) ++j;
		if (j >= len) break; // incomplete checksum field

		size_t msg_end = j + 1; // include SOH

		// callback receives the complete message span
		cb(data + last_consumed, msg_end - last_consumed);

		i = msg_end;
		last_consumed = msg_end;
	}

	return last_consumed;
}

} // namespace fix
