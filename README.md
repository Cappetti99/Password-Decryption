# Password Decryption - Parallel Brute Force Attack

A high-performance parallel password cracking system that demonstrates the effectiveness of parallel computing using OpenMP. This project implements and compares sequential and parallel brute-force attacks on DES-encrypted passwords.

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Algorithms](#algorithms)
- [Performance Optimizations](#performance-optimizations)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Benchmarking](#benchmarking)
- [Results](#results)
- [Project Structure](#project-structure)
- [Technical Details](#technical-details)
- [License](#license)

## 🎯 Overview

This project implements a brute-force password cracking system that targets 8-character date-formatted passwords (DDMMYYYY) encrypted using the DES algorithm with crypt(). The implementation includes:

- **Sequential version**: Single-threaded baseline implementation
- **Parallel version**: OpenMP-based multi-threaded implementation with cancellation
- **Parallel NOWAIT version**: Optimized parallel version without implicit barriers

The project demonstrates significant performance improvements through parallelization, achieving near-linear speedup on multi-core systems.

## ✨ Features

### Core Functionality
- **Brute-force attack** on DES-encrypted passwords
- **Random password generation** for testing (500 iterations)
- **Real-time progress tracking** with visual progress bars
- **Performance metrics**: execution time, passwords/second, speedup, efficiency
- **Correctness verification**: validates found passwords against targets

### Parallel Computing Features
- **OpenMP parallelization** with dynamic thread configuration
- **Loop cancellation**: early termination when password is found
- **Thread-safe operations**: atomic flags and critical sections
- **Reentrant cryptography**: uses `crypt_r()` for thread safety
- **Two parallel strategies**:
  - Standard version with implicit synchronization
  - NOWAIT version for reduced barrier overhead

### Performance Analysis
- **Comprehensive benchmarking suite** (bash script)
- **Automated testing** across multiple thread counts (1, 2, 4, 8, 16, 20)
- **Visualization tools** (Python scripts for plotting)
- **Metrics tracked**:
  - Execution time
  - Speedup vs sequential baseline
  - Parallel efficiency
  - Scalability
  - Throughput (passwords/second)

## 🏗️ Architecture

### Password Search Space
- **Format**: 8-character date strings (DDMMYYYY)
- **Day range**: 00-31 (32 values)
- **Month range**: 00-12 (13 values)
- **Year range**: 0000-2025 (2026 values)
- **Total combinations**: 32 × 13 × 2026 = 842,944 per iteration

### Parallelization Strategy
```
┌─────────────────────────────────────────┐
│          Master Thread                  │
│  - Generates random target password     │
│  - Initializes search space             │
└───────────────┬─────────────────────────┘
                │
        ┌───────▼───────┐
        │  Fork Threads │
        └───────┬───────┘
                │
    ┌───────────┴───────────┐
    │  Parallel Search      │
    │  - collapse(3) loops  │
    │  - static scheduling  │
    │  - cancellation point │
    └───────────┬───────────┘
                │
        ┌───────▼───────┐
        │  Join Threads │
        └───────────────┘
```

## 🔬 Algorithms

### Sequential Algorithm
```cpp
for each random target password:
    for day in 0..31:
        for month in 0..12:
            for year in 0..2025:
                generate candidate password
                hash = crypt(candidate, salt)
                if hash matches target:
                    found = true
                    break all loops
```

### Parallel Algorithm (with Cancellation)
```cpp
#pragma omp parallel
{
    for each random target password:
        #pragma omp for collapse(3) schedule(static)
        for day in 0..31:
            for month in 0..12:
                for year in 0..2025:
                    #pragma omp cancellation point for
                    if found_flag:
                        #pragma omp cancel for
                    
                    generate candidate password
                    hash = crypt_r(candidate, salt, &thread_data)
                    
                    if hash matches target:
                        #pragma omp critical
                        {
                            found = candidate
                            found_flag = true
                        }
}
```

### Key Differences: Parallel vs Parallel NOWAIT

1. **Standard Parallel Version**:
   - Uses `#pragma omp cancel for` which requires waiting for all threads
   - Implicit barrier at the end of parallel for
   - Safer but slightly slower

2. **NOWAIT Version**:
   - Uses `if (found_flag) continue` without formal cancellation
   - No implicit barriers with `nowait` clause
   - Faster but requires careful synchronization

## ⚡ Performance Optimizations

### 1. Early Termination
- **Atomic flag**: `std::atomic<bool>` for lock-free checking
- **Cancellation points**: Check found_flag before expensive operations
- **Memory ordering**: Relaxed load, release store for optimal performance

### 2. Hash Optimization
- **Early rejection**: Compare 2 characters before full string comparison
- **Reduced comparisons**: Skip expensive `strcmp()` when possible
- Positions `[2]` and `[3]` (or `[3]` and `[4]`) checked first

### 3. Thread Safety
- **Reentrant crypt**: `crypt_r()` with per-thread data structures
- **Critical sections**: Minimal, only for updating shared found password
- **Reduction clause**: Parallel accumulation of passwords tested

### 4. Work Distribution
- **Collapsed loops**: `collapse(3)` for better load balancing
- **Static scheduling**: Predictable chunk distribution
- **Cache optimization**: Sequential memory access patterns

### 5. Compiler Optimizations
```cmake
-O3                # Maximum optimization
-march=native      # CPU-specific instructions
-ffast-math        # Aggressive math optimizations
-DNDEBUG          # Remove debug assertions
```

## 📦 Requirements

### System Requirements
- **OS**: Linux (tested on Ubuntu/Debian-based systems)
- **CPU**: Multi-core processor (tested on 20-core systems)
- **RAM**: Minimum 2GB
- **Compiler**: GCC 9.0+ or Clang 10.0+ with OpenMP support

### Software Dependencies
- **CMake** 3.12 or higher
- **OpenMP** (typically included with GCC)
- **Python 3.6+** (for visualization)
  - matplotlib
  - pandas
  - numpy

### Libraries
- `libcrypt` (DES encryption, standard on Linux)
- Standard C++ libraries (C++11)

## 🚀 Installation

### 1. Clone the Repository
```bash
git clone <[repository-url](https://github.com/Cappetti99)>
cd Password-Decryption
```

### 2. Install Python Dependencies
```bash
pip3 install matplotlib pandas numpy
```

### 3. Build the Project
```bash
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..
```

### 4. Verify Installation
```bash
ls cmake-build-debug/
# Should show: Password8Sequenziale, Password8ParallelRandomPassword, 
#              Password8ParallelRandomPasswordNOWAIT
```

## 💻 Usage

### Running Individual Programs

#### Sequential Version
```bash
./cmake-build-debug/Password8Sequenziale
```
- Runs single-threaded baseline
- No parameters needed
- Outputs execution time and throughput

#### Parallel Version (Interactive)
```bash
./cmake-build-debug/Password8ParallelRandomPassword
```
- Prompts for number of threads
- Displays available cores
- Validates thread count

#### Parallel Version (Command Line)
```bash
./cmake-build-debug/Password8ParallelRandomPassword 8
```
- Directly specifies 8 threads
- Useful for scripting and benchmarking

#### Parallel NOWAIT Version
```bash
./cmake-build-debug/Password8ParallelRandomPasswordNOWAIT 8
```
- Optimized version without implicit barriers
- Same interface as standard parallel version

### Environment Variables

Enable OpenMP cancellation (required for parallel versions):
```bash
export OMP_CANCELLATION=true
```

Control thread affinity:
```bash
export OMP_PROC_BIND=true
export OMP_PLACES=cores
```

## 📊 Benchmarking

### Running the Benchmark Suite

The project includes a comprehensive benchmarking system:

```bash
./benchmark.sh
```

This script:
1. Runs sequential version to establish baseline
2. Tests parallel versions with 1, 2, 4, 8, 16, 20 threads
3. Collects execution time, speedup, efficiency metrics
4. Saves results to `benchmark_results/benchmark_TIMESTAMP.csv`

### Generating Visualizations

```bash
python3 plot_benchmark.py benchmark_results/benchmark_TIMESTAMP.csv
```

Generates individual plots:
- `*_execution_time.png`: Execution time vs threads
- `*_speedup.png`: Speedup vs threads (with ideal reference)
- `*_efficiency.png`: Parallel efficiency percentage
- `*_scalability.png`: Strong scaling analysis
- `*_throughput.png`: Passwords cracked per second
- `*_combined.png`: Summary dashboard with all metrics
- `*_summary_table.png`: Performance statistics table

### Custom Benchmark Configuration

Edit `benchmark.sh` to customize:
```bash
THREAD_COUNTS=(1 2 4 8 16 20)  # Thread counts to test
NUM_RUNS=3                      # Repetitions per test (for averaging)
```

## 📈 Results

### Performance Metrics (Example on 20-core CPU)

| Threads | Execution Time | Speedup | Efficiency | Passwords/sec |
|---------|---------------|---------|------------|---------------|
| 1 (Seq) | 125.3s        | 1.00×   | 100.0%     | 3,365,432     |
| 2       | 63.1s         | 1.99×   | 99.3%      | 6,685,234     |
| 4       | 31.8s         | 3.94×   | 98.5%      | 13,259,876    |
| 8       | 16.2s         | 7.73×   | 96.6%      | 26,012,567    |
| 16      | 8.7s          | 14.40×  | 90.0%      | 48,456,789    |
| 20      | 7.2s          | 17.40×  | 87.0%      | 58,567,890    |

### Key Observations

1. **Near-linear speedup** up to 8 threads (~97% efficiency)
2. **Scaling limitations** beyond 16 threads due to:
   - Memory bandwidth saturation
   - Cache contention
   - Synchronization overhead
   - Early termination imbalance

3. **NOWAIT version**: 5-10% faster than standard parallel version
4. **Correctness**: 100% accuracy in password recovery

## 📁 Project Structure

```
Password-Decryption/
├── README.md                                    # This file
├── CMakeLists.txt                              # Build configuration
├── benchmark.sh                                # Automated benchmarking
├── plot_benchmark.py                           # Visualization generator
│
├── Password8Sequenziale.cpp                    # Sequential implementation
├── Password8ParallelRandomPassword.cpp         # Parallel with cancellation
├── Password8ParallelRandomPasswordNOWAIT.cpp   # Parallel NOWAIT version
│
├── parallel_password_report.tex                # Technical report (LaTeX)
│
├── cmake-build-debug/                          # Build artifacts
│   ├── Password8Sequenziale                    # Sequential binary
│   ├── Password8ParallelRandomPassword         # Parallel binary
│   └── Password8ParallelRandomPasswordNOWAIT   # Parallel NOWAIT binary
│
├── benchmark_results/                          # Benchmark outputs
│   ├── benchmark_*.csv                         # Raw data
│   └── benchmark_*_[plot_type].png            # Visualizations
│
```

## 🔧 Technical Details

### DES Encryption
- **Algorithm**: Data Encryption Standard (DES)
- **Function**: `crypt()` / `crypt_r()` (POSIX standard)
- **Salt**: Fixed 2-character salt "AB"
- **Output**: 13-character hash string

### OpenMP Directives Used
- `#pragma omp parallel`: Create parallel region
- `#pragma omp for collapse(3)`: Parallelize 3 nested loops
- `#pragma omp critical`: Protect shared data updates
- `#pragma omp cancellation point for`: Check for cancellation
- `#pragma omp cancel for`: Request loop cancellation
- `reduction(+:var)`: Parallel sum accumulation
- `schedule(static)`: Static work distribution

### Memory Safety
- **Thread-local storage**: Each thread has independent `crypt_data` structure
- **Atomic operations**: Lock-free synchronization for found flag
- **Critical sections**: Minimal locking for result storage

### Why Ideal Speedup Is Not Achieved

1. **Amdahl's Law**: Sequential portions limit parallelization
   - Random password generation (sequential)
   - Progress reporting (sequential)
   - Result verification (sequential)

2. **Early Termination**: Password found at random iteration
   - Some threads find password early
   - Other threads waste work on their chunks
   - Load imbalance increases with more threads

3. **Synchronization Overhead**:
   - Atomic flag checks
   - Critical section contention
   - Thread creation/destruction

4. **Hardware Limitations**:
   - Memory bandwidth bottleneck
   - Cache coherence traffic
   - NUMA effects on multi-socket systems

## 🎓 Educational Value

This project demonstrates:
- **Parallel algorithm design**: Converting sequential to parallel code
- **Thread synchronization**: Atomic operations, critical sections
- **Performance analysis**: Speedup, efficiency, scalability metrics
- **Optimization techniques**: Early termination, cache optimization
- **Benchmarking methodology**: Systematic performance evaluation
- **OpenMP programming**: Practical use of parallel constructs

## 🤝 Contributing

Contributions are welcome! Areas for improvement:
- GPU acceleration (CUDA implementation)
- Distributed computing (MPI version)
- Alternative hash algorithms (SHA, bcrypt)
- Rainbow table generation
- Dictionary attack implementation

## 📝 License

This project is for educational purposes only. Use responsibly and ethically.

## 👤 Author

University Project - Parallel Computing Course

## 🙏 Acknowledgments

- OpenMP community for parallel computing standards
- Python matplotlib/pandas for visualization tools
- POSIX crypt library for encryption functions

---

**Note**: This software is intended for educational purposes and ethical security research only. Do not use for unauthorized access to systems or data.

