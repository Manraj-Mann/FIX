# recvmmsg/sendmmsg batching plan

This branch adds a short implementation plan for adding `recvmmsg`/`sendmmsg` batching to the FIX engine.

Goal
- Reduce syscall overhead by reading multiple packets/messages per syscall and writing batches out with `sendmmsg`.
- Target: increase small-message throughput (expected 2x–5x) and reduce CPU load per message.

Implementation notes
- Use `recvmmsg` in the I/O loop for EPOLLIN sockets. Prepare an array of `mmsghdr`/`iovec` sized to a modest batch (e.g. 8–32).
- For TCP stream sockets the messages are not packetized; use recvmmsg only for datagram sockets or use `readv`/`recvmmsg` with MSG_WAITFORONE carefully. For our benchmark (stream), implement a `recvmmsg`-style batching by using `io_uring` or `recvmmsg` on UDP tests; otherwise use `recvmmsg` on connected UDP or rely on `recvmmsg` with MSG_DONTWAIT on raw sockets.
- Simpler approach for TCP: use `recvmmsg`-like batching by calling `recv` into multiple preallocated buffers (loop with non-blocking) and aggregate before parsing — reduces syscall frequency by coalescing reads where possible.
- On write path use `sendmmsg` to batch multiple outbound messages into a single syscall.
- Carefully handle partial reads (stream framing) and buffer boundaries; keep per-connection circular buffer offsets and avoid memmove.

Testing
- Add unit tests for correctness (partial messages, checksum boundaries).
- Re-run the benchmark and measure throughput & CPU.

TODOs
- Implement `recvmmsg` batching prototype in `src/engine.{hpp,cpp}` (I/O thread).
- Add fallbacks for environments where `recvmmsg` is not available.

