# Cache Simulator

A C++ implementation of a configurable cache memory simulator that models different cache organizations and replacement policies.

## Authors
- Akshat Meena
- Siddharth Singh
- Muskan Jeph

## Overview

This project simulates the behavior of cache memory systems with various configurations. It processes memory access traces to calculate and report cache performance metrics such as hit rates, miss rates, and the number of dirty blocks evicted.

## Features

- **Configurable Cache Parameters**:
  - Cache Size (in bytes)
  - Block Size (in bytes)
  - Cache Organization:
    - Fully Associative (0)
    - Direct-Mapped (1)
    - Set-Associative (2/4/8/16/32)
  - Replacement Policies:
    - Random Replacement (0)
    - LRU (Least Recently Used) (1)
    - Pseudo-LRU (Tree-based implementation) (2)

- **Performance Metrics Tracked**:
  - Total cache accesses
  - Read accesses and misses
  - Write accesses and misses
  - Types of misses:
    - Compulsory misses
    - Capacity misses
    - Conflict misses
  - Dirty blocks evicted

## Implementation Details

The simulator uses a linked list data structure to implement cache sets, with each node representing a cache block containing:
- Tag value
- Valid bit
- Dirty bit

For the Pseudo-LRU replacement policy, a binary tree structure is implemented to track access patterns and determine which block to replace when eviction is necessary.

## Usage

1. Compile the source code:
   ```
   g++ Cache_Simulator.cpp -o cache_simulator
   ```

2. Run the program:
   ```
   ./cache_simulator
   ```

3. Input parameters when prompted:
   - Cache Size (in bytes)
   - Block Size (in bytes)
   - Cache Type (0, 1, or 2/4/8/16/32)
   - Replacement Policy (0, 1, or 2)
   - Name of file containing memory traces

## Input Format

The program expects memory traces in hexadecimal format with read/write indicators:
- `0x[address]r` for read operations
- `0x[address]w` for write operations

## Output

The simulator outputs the following statistics:
- Cache configuration details
- Total cache accesses
- Read and write accesses
- Cache misses (total and categorized)
- Number of dirty blocks evicted

## Example

```
Enter the Cache Size (in Bytes) : 1024
Enter the Block Size (in Bytes) : 32
Enter the Cache Type (0 for Fully Associative, 1 for Direct Mapped, 2/4/8/16/32 for Set-Associative) : 4
Enter the Replacement Policy (0 for Random, 1 for LRU, 2 for Pseudo-LRU) : 1
Enter the name of file containing memory traces : traces.txt

1024     #Cache Size
32       #Block Size
4-Way Set Associative Cache
LRU Replacement
10000    #Cache Access
7500     #Read Access
2500     #Write Access
3450     #Cache Misses
2800     #Compulsory Misses
0        #Capacity Misses
650      #Conflict Misses
2760     #Read Misses
690      #Write Misses
320      #Dirty Blocks Evicted
```
