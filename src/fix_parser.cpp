#include "fix_parser.hpp"
#include <cstring>

namespace fix {

static constexpr char SOH = '\x01';

size_t parse_messages(const char* data, size_t len, const MessageCallback& cb) {
    size_t i = 0;
    size_t last_consumed = 0;
    while (i < len) {
        // find "10=" sequence
        const char* found = (const char*)memmem(data + i, len - i, "10=", 3);
        if (!found) break;
        size_t pos = found - data;
        // find the next SOH after checksum value
        size_t j = pos + 3; // start of checksum digits
        while (j < len && data[j] != SOH) ++j;
        if (j >= len) break; // partial checksum, wait for more data

        size_t msg_end = j + 1; // include final SOH
        cb(data + last_consumed, msg_end - last_consumed);
        i = msg_end;
        last_consumed = msg_end;
    }

    return last_consumed;
}

} // namespace fix
