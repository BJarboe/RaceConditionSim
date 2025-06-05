# Food Delivery Simulation

A multithreaded C++ simulation that models a food delivery system using POSIX threads, semaphores, and mutexes. It includes producer threads (sandwiches and pizzas) and consumer threads (two delivery services) sharing a synchronized queue.

## Build

Make sure your `Makefile` is present, then run:

```bash
make
```

## Run

```bash
./fooddelivery [options]
```

### Options

| Flag | Description                                  | Default |
|------|----------------------------------------------|---------|
| `-n` | Total number of delivery requests            | 100     |
| `-a` | Time (ms) for delivery service A             | 0       |
| `-b` | Time (ms) for delivery service B             | 0       |
| `-p` | Time (ms) to produce sandwich requests       | 0       |
| `-s` | Time (ms) to produce pizza requests          | 0       |

Example:

```bash
./fooddelivery -n 50 -a 500 -b 300 -p 100 -s 200
```

## File Structure

- `fooddelivery.cpp` — Main simulation
- `log.h`, `log.c` — Logging utilities
- `Makefile` — Build script

## Clean

```bash
make clean
```

