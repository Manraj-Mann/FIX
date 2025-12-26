# FIX - High-performance C++ FIX Engine (prototype)

This repository contains a small, high-performance prototype FIX engine and a benchmark runner demonstrating a low-latency, epoll-based design with a zero-copy FIX parser.

Quick build and run:

```bash
mkdir -p build && cd build
cmake ..
cmake --build . -- -j
./fix_benchmark
```

What is included:
- `src/engine.cpp/.hpp` — epoll-based non-blocking TCP server skeleton
- `src/fix_parser.*` — minimal zero-copy parser that detects complete FIX messages
- `src/main.cpp` — benchmark runner that starts the server and client workers

Notes and next steps:
- This is a focused prototype for performance experimentation. To beat production engines you should iteratively add:
	- zero-copy network stacks (kernel-bypass / AF_XDP / io_uring)
	- lock-free ring buffers and batching
	- custom allocators and object pools
	- careful CPU-affinity, jumbo frames, and syscall reduction (sendmmsg/recvmmsg)
	- rigorous correctness (checksum, sequence numbers, resend, state machine)

If you want, I can now:
- add a lock-free ring buffer and custom allocator
- implement send/receive batching with `sendmmsg`/`recvmmsg`
- add a simple session state machine and persistence

**Next Steps (detailed roadmap)**
- **Recvmmsg/sendmmsg batching:** Replace the per-socket `recv` loop with `recvmmsg` to read multiple datagrams/messages with one syscall and use `sendmmsg` for batched writes. This typically yields large CPU and throughput gains for small messages.
- **Lock-free ring buffers:** Add a single-producer/single-consumer or multi-producer/multi-consumer ring buffer for handoff between the I/O thread and worker threads to avoid locks and memmoves.
- **Eliminate memmove copies:** Use circular buffer offsets or start/stop indices per connection to avoid shifting bytes after parsing; only advance indices and wrap.
- **Per-core threading model:** Run one epoll instance per thread (or shard fds) and pin threads to CPU cores. Use SO_REUSEPORT and accept load-balancing to scale linearly across cores.
- **Custom allocator & pools:** Replace `std::string`/heap allocations (if any) with object pools and fixed-size message buffers to reduce allocator overhead and fragmentation.
- **I/O optimizations & kernel bypass:** Evaluate `io_uring` for async batching or `AF_XDP` for kernel-bypass on supported kernels and NICs for multi‑million msg/s targets.
- **Protocol correctness:** Implement FIX session management (sequence numbers, resend requests, logout/reconnect, checksums) and unit tests to ensure correctness under load.
- **Observability & testing harness:** Add microbenchmarks, latency histograms, and network emulation (tc/netem) tests so changes improve real-world metrics, not just synthetic local runs.

If you want, I can implement the first item (`recvmmsg`/`sendmmsg` batching) now and re-run the benchmark to measure improvement.
