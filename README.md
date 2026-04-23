# Sleeping Barber Problem Simulator

## Project Description

This implementation solves the **Sleeping Barber Problem** - a classic synchronization problem that demonstrates concurrency control, semaphore usage, and process coordination. The simulation adds modern enhancements including:

- **Priority-based customer service** (higher priority customers are served first)
- **Real-time shop status visualization**
- **Comprehensive test scenarios**
- **Multi-process architecture with FIFO communication**
- **Detailed logging system**

### Problem Overview

The classic Sleeping Barber problem involves:
- A barber shop with a certain number of waiting chairs
- A single barber who cuts hair
- Customers who arrive at random intervals
- If no customers exist, the barber sleeps
- If all chairs are occupied, new customers leave

### Enhanced Features

This implementation adds:
- **Priority Queuing**: Customers with higher priority (lower priority number) are served first
- **Dynamic Customer Generation**: Test various arrival patterns
- **Real-time Visualization**: Live shop status display
- **IPC Logging**: Separate consumer process handles logging
- **Multiple Test Scenarios**: Pre-defined test cases for thorough validation

## Features

### Core Functionality
- **Semaphore-based synchronization** (customers, barber, mutex)
- **Priority scheduling** for waiting customers
- **Thread-safe operations** with mutex protection
- **Graceful shop closing** mechanism
- **Real-time status updates**

### Testing Capabilities
- **6 pre-defined test cases**:
  1. No customers arrive
  2. Single customer arrival
  3. Multiple customers with available chairs
  4. All chairs occupied scenario
  5. Continuous customer stream
  6. User-defined number of customers

### Visualization & Logging
- **Live console dashboard** showing:
  - Waiting customers count
  - Customers served total
  - Customers turned away
  - Current shop status
- **Persistent logging** with timestamps
- **Separate consumer process** for non-blocking logging

### Inter-Process Communication
- **Named pipe (FIFO)** for process communication
- **Producer-Consumer architecture**
- **Process isolation** for better reliability

## Installation

### Prerequisites
- Linux/Unix environment (POSIX compliant)
- GCC compiler
- pthread library (usually included)
- Make (optional)

### Compilation

```bash
# Compile with pthread and real-time libraries
gcc -o barber_sim sleeping_barber.c -lpthread -lrt

# Or use the provided Makefile (see below)
make
```

### Running the simulation
```bash
./barber_sim
```

## Project Structure
```
sleeping-barber/
├── sleeping_barber.c      # Main source code
├── Makefile               # Build automation
├── README.md              # This file
├── log.txt               # Simulation log (generated)
└── /tmp/prodcon_fifo     # IPC pipe (created at runtime)
```

## Makefile
Create this makefile for easy compilation:
```bash
CC = gcc
CFLAGS = -Wall -pthread -lrt
TARGET = barber_sim
SOURCES = sleeping_barber.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) log.txt
	rm -f /tmp/prodcon_fifo

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb ./$(TARGET)

.PHONY: all clean run debug
```
