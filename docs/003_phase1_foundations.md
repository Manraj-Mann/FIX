# Phase 1 — Low-Latency Networking Foundations (TODO List)

## Objective

Build a minimal TCP echo server/client with measured latency, zero unnecessary overhead, and full understanding of where time is spent.

## 1. OS & Hardware Basics (Concepts)

**TODO – Understand (not just read):**

- User space vs kernel space
- Syscalls cost (read, write, send, recv)
- Context switches
- Page faults (minor vs major)
- CPU cache hierarchy (L1/L2/L3)
- Cache line size (usually 64 bytes)

**Outcome:**
You know why fewer syscalls = lower latency.

## 2. Linux TCP Socket Fundamentals

**TODO – Learn & Apply:**

- socket(), bind(), listen(), accept()
- Blocking vs non-blocking sockets
- recv() vs read()
- send() vs write()

**Key flags to know:**

- O_NONBLOCK
- MSG_DONTWAIT
- MSG_NOSIGNAL

## 3. Disable Latency Killers (Very Important)

**TODO – Explicitly set:**

**Disable Nagle's Algorithm:**
```
setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, ...)
```

**Increase socket buffers:**
- SO_SNDBUF
- SO_RCVBUF

**Concepts:**
- Why batching hurts latency
- When Nagle helps vs hurts

## 4. epoll (Core Skill)

**TODO – Implement:**

- epoll_create1
- epoll_ctl
- epoll_wait

**Concepts to master:**

- Edge-triggered vs level-triggered
- Why edge-triggered is preferred for latency
- What happens if you don't drain the socket fully

**Deliverable:**

Single-threaded epoll server handling multiple clients

## 5. Buffer Management (No std::string)

**TODO – Rules:**

- Use fixed-size buffers (stack or pre-allocated heap)
- Reuse buffers
- No dynamic allocation per message

**Example:**
```cpp
alignas(64) char recv_buffer[4096];
```

**Concepts:**

- Cache line alignment
- False sharing (even single-threaded, prepare mindset)

## 6. Echo Server (Minimal but Clean)

**TODO – Server behavior:**

- Accept connection
- Read bytes
- Immediately write them back
- No parsing, no logging

**Strict rules:**

- No std::cout
- No heap allocation per request
- No sleep

## 7. Echo Client (Latency Measurement)

**TODO – Client should:**

- Send a fixed-size payload (e.g., 64 bytes)
- Record send timestamp
- Wait for echo
- Record receive timestamp

**Measure:**

- Round-trip latency
- Average
- P50 / P99 (simple array sort is enough)

## 8. Time Measurement (CRITICAL)

**TODO – Learn & implement:**

- clock_gettime(CLOCK_MONOTONIC_RAW)
- rdtsc (later)

**Concepts:**

- Why std::chrono can lie
- TSC stability
- CPU frequency scaling effects

## 9. CPU Affinity (Intro Level)

**TODO – Pin threads:**

- Server thread → CPU 1
- Client thread → CPU 2

**Learn:**

- sched_setaffinity()

**Concepts:**

- Cache warmth
- Reduced context switches

## 10. Build & Compile Correctly

**TODO – Compile with:**
```
g++ -O3 -march=native -DNDEBUG
```

**Later explore:**

- -fno-exceptions
- -fno-rtti

## 11. Profiling & Observation

**TODO – Use tools:**

- perf stat
- strace (to count syscalls)
- tcpdump (verify packets)

**Questions you must answer:**

- How many syscalls per message?
- Where is the time going?
- What happens if payload size changes?

## Phase 1 Deliverables (Must Complete)

By end of Phase 1, you should have:

- ✔ Single-threaded epoll echo server
- ✔ Echo client with latency stats
- ✔ Nagle disabled
- ✔ Fixed-size buffers
- ✔ CPU pinned threads
- ✔ Measured latency numbers

## Mental Model You Should Gain

After Phase 1, you should intuitively feel:

> "Every syscall, cache miss, and branch has a cost."

That mindset is everything in low latency.
