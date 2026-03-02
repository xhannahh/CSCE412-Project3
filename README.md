# CSCE412-Project3

# Load Balancer & Bonus Load Balancer Simulation

## Project Overview

This project implements a scalable **Load Balancer simulation** in C++ that models how distributed systems dynamically adjust server capacity in response to changing workloads.

The system simulates incoming network requests over a configurable number of clock cycles and automatically scales server instances up or down based on queue load thresholds. A bonus implementation extends the design to support **separate processing and streaming services**, each with independent scaling logic.

The simulation demonstrates core concepts in:

- Auto-scaling infrastructure
- Queue management
- Request routing
- Cooldown-based scaling control
- Distributed system behavior under variable load

---

## Features

### Core Load Balancer

- Configurable minimum and maximum server counts
- Request queue with capacity limits
- Random request generation per clock cycle
- Dynamic scaling:
  - **Scale Up** when load exceeds threshold
  - **Scale Down** when load drops below threshold
- Cooldown mechanism to prevent rapid scaling oscillation
- Detailed simulation logging

### Bonus Load Balancer

- Two independent load balancers:
  - **Processing Servers**
  - **Streaming Servers**
- Intelligent routing based on request type
- Independent scaling behavior per service
- Blocked request tracking
- Per-cycle statistics reporting

---

## How It Works

1. The system initializes with a configurable number of servers.
2. For each clock cycle:
   - A random number of requests is generated.
   - Requests are routed based on type (Processing or Streaming).
   - Servers process queued requests.
   - The system evaluates scaling conditions.
3. Scaling decisions are based on:
   - Queue size per server
   - Configurable scale-up and scale-down thresholds
   - Cooldown interval between scaling events

The simulation runs for a specified number of clock cycles and logs system activity, including:

- Requests routed
- Requests blocked
- Queue sizes
- Scale up/down events
- Final server utilization

---
