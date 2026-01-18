# What is a "Low-Latency FIX Engine" (in practice)

At minimum, your engine should:

- Parse & serialize FIX messages (FIX 4.2 or 4.4)
- Maintain sessions
  - Logon / Logout
  - Heartbeats
  - Sequence numbers
- Handle TCP sockets efficiently
- Send & receive messages with microsecond-level latency
- Be deterministic and predictable under load

Later, you can extend it to:

- Order routing
- Drop copy
- Risk checks
- Exchange-specific quirks

# Core Low-Latency Concepts You'll Learn (this is the real value)

By doing this project, you'll deeply learn:

## ⚡ CPU & Memory

- Cache lines (64 bytes)
- False sharing
- Struct packing & alignment
- Branch prediction
- Instruction-level efficiency

## ⚡ C++ Techniques

- Avoiding heap allocations
- Custom allocators
- std::string_view, constexpr
- Move semantics
- Lock-free / wait-free data structures

## ⚡ Networking

- TCP vs UDP tradeoffs
- Nagle's algorithm (TCP_NODELAY)
- epoll / io_uring / poll
- Zero-copy techniques
- Kernel bypass concepts (theory)

## ⚡ Multithreading

- Core pinning
- Single-writer principle
- SPSC / MPMC queues
- Avoiding mutexes on hot paths

## ⚡ Measurement (most important)

- Latency vs throughput
- P99 / P999 latency
- rdtsc timing
- Flamegraphs

# Recommended Architecture (Simple but Serious)

Start single-threaded first.
Low latency ≠ many threads.

```
[ NIC ]
   |
[ TCP Socket ]
   |
[ IO Loop (epoll) ]
   |
[ FIX Decoder ]
   |
[ Session Logic ]
   |
[ FIX Encoder ]
   |
[ TCP Send ]
```

Once stable:

```
IO Thread  --->  SPSC Queue  --->  Strategy / Logic Thread
```
