#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <cstdint>

namespace time_utils
{
    /* -------- Time -------- */
    inline uint64_t now_ns() {
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return uint64_t(ts.tv_sec) * 1'000'000'000ull + ts.tv_nsec;
    }

} // namespace time_utils
