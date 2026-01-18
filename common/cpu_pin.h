#include <sched.h>

namespace CPUPIN
{
    /* -------- CPU Pin -------- */
    inline void pin_to_cpu(int cpu) {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set);
        sched_setaffinity(0, sizeof(set), &set);
    }
}