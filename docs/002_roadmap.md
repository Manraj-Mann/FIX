# Step-by-Step Roadmap (Very Important)

## Phase 1 – Foundations (No FIX yet)

**Goal:** Learn raw low-latency IO

Write a TCP echo server

**Use:**
- epoll (Linux)
- Non-blocking sockets
- Measure round-trip latency

**Concepts learned:**
- Syscalls
- Context switches
- Buffer reuse

## Phase 2 – FIX Message Parser (Critical)

**Goal:** Zero-allocation FIX parsing

Example FIX message:
```
8=FIX.4.2|9=65|35=A|49=CLIENT|56=EXCHANGE|34=1|52=...|10=128|
```

**Rules:**
- Use SOH (0x01), not |
- No std::string allocations
- Parse using pointers

**You'll learn:**
- Pointer arithmetic
- Avoiding copies
- Cache-friendly parsing

## Phase 3 – Session Layer

**Implement:**
- Logon (35=A)
- Heartbeat (35=0)
- Test Request (35=1)
- Logout (35=5)
- Sequence numbers

**This teaches:**
- State machines
- Timeouts without sleeping
- Deterministic behavior

## Phase 4 – Encoder & Send Path

- Pre-build FIX templates
- Patch fields in place (seq num, checksum)
- Reuse buffers

This is where nanoseconds matter.

## Phase 5 – Multithreading (Optional but Realistic)

- IO thread
- Business logic thread
- Lock-free queues
- Pin threads to CPU cores.

# Coding Rules for Low Latency (Follow These Religiously)

## ❌ Don't:

- new / delete on hot path
- std::cout
- std::string concatenation
- mutex in IO path
- exceptions

## ✅ Do:

- Pre-allocate everything
- Use fixed-size buffers
- -O3 -march=native
- Disable RTTI & exceptions (later)
- Profile constantly

# Tools You Must Use

- perf
- valgrind (cachegrind)
- htop
- tcpdump
- gdb
- clang-format (clean code still matters)

# Libraries – Use Carefully

**For learning:**
- Do NOT use QuickFIX (too heavy, hides latency problems)

**Optional later:**
- boost::asio (only to compare, not for final engine)

# End Goal (What You'll Be Able to Say)

After this project, you can confidently say:

> "I understand where every microsecond goes between NIC → CPU → code → NIC."

That's exactly what trading firms look for.
