@echo off
REM Compile the benchmark
echo Compiling RocksDB benchmark...
g++ -std=c++17 rocksdb-benchmark.cpp -o rocksdb-benchmark.exe -lrocksdb -pthread

REM Run the benchmark with SPFresh-like workload
echo Running benchmark for SPFresh workload (10K QPS, 1B records simulation)...
rocksdb-benchmark.exe ^
  --db_path spfresh_benchmark_db ^
  --threads 16 ^
  --duration 300 ^
  --qps 10000 ^
  --read_ratio 0.7 ^
  --value_size 1024 ^
  --num_records 10000000 ^
  --block_cache_gb 3 ^
  --direct_io true ^
  --enable_blob true

echo Benchmark complete. Results saved to rocksdb_stats.txt