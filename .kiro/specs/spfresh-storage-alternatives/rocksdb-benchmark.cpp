#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <random>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <condition_variable>
#include <algorithm>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/table.h"
#include "rocksdb/filter_policy.h"
#include "rocksdb/slice.h"
#include "rocksdb/rate_limiter.h"

// Configuration parameters
struct BenchmarkConfig {
    std::string db_path = "benchmark_db";
    int num_threads = 16;
    int duration_seconds = 60;
    int target_qps = 10000;
    double read_ratio = 0.7;  // 70% reads, 30% writes
    int value_size = 1024;    // 1KB values
    int num_records = 1000000; // 1M records for testing (scale as needed)
    bool use_direct_io = true;
    int block_cache_gb = 3;
    bool enable_statistics = true;
    bool enable_blob = true;
};

// Latency statistics
struct LatencyStats {
    std::vector<double> read_latencies;
    std::vector<double> write_latencies;
    std::mutex stats_mutex;

    void add_read_latency(double ms) {
        std::lock_guard<std::mutex> lock(stats_mutex);
        read_latencies.push_back(ms);
    }

    void add_write_latency(double ms) {
        std::lock_guard<std::mutex> lock(stats_mutex);
        write_latencies.push_back(ms);
    }

    void print_summary() {
        std::lock_guard<std::mutex> lock(stats_mutex);
        
        if (!read_latencies.empty()) {
            std::sort(read_latencies.begin(), read_latencies.end());
            double p50 = read_latencies[read_latencies.size() * 0.5];
            double p95 = read_latencies[read_latencies.size() * 0.95];
            double p99 = read_latencies[read_latencies.size() * 0.99];
            
            std::cout << "Read Latency (ms):" << std::endl;
            std::cout << "  p50: " << std::fixed << std::setprecision(2) << p50 << std::endl;
            std::cout << "  p95: " << std::fixed << std::setprecision(2) << p95 << std::endl;
            std::cout << "  p99: " << std::fixed << std::setprecision(2) << p99 << std::endl;
        }
        
        if (!write_latencies.empty()) {
            std::sort(write_latencies.begin(), write_latencies.end());
            double p50 = write_latencies[write_latencies.size() * 0.5];
            double p95 = write_latencies[write_latencies.size() * 0.95];
            double p99 = write_latencies[write_latencies.size() * 0.99];
            
            std::cout << "Write Latency (ms):" << std::endl;
            std::cout << "  p50: " << std::fixed << std::setprecision(2) << p50 << std::endl;
            std::cout << "  p95: " << std::fixed << std::setprecision(2) << p95 << std::endl;
            std::cout << "  p99: " << std::fixed << std::setprecision(2) << p99 << std::endl;
        }
    }
};

// Throughput tracking
struct ThroughputTracker {
    std::atomic<uint64_t> reads{0};
    std::atomic<uint64_t> writes{0};
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    
    void start() {
        start_time = std::chrono::steady_clock::now();
    }
    
    void print_stats(bool final = false) {
        auto now = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        
        double read_qps = reads / elapsed_seconds;
        double write_qps = writes / elapsed_seconds;
        double total_qps = read_qps + write_qps;
        
        std::cout << (final ? "Final " : "") << "Throughput:" << std::endl;
        std::cout << "  Reads:  " << std::fixed << std::setprecision(1) << read_qps << " ops/sec" << std::endl;
        std::cout << "  Writes: " << std::fixed << std::setprecision(1) << write_qps << " ops/sec" << std::endl;
        std::cout << "  Total:  " << std::fixed << std::setprecision(1) << total_qps << " ops/sec" << std::endl;
        
        if (final) {
            std::cout << "Total operations:" << std::endl;
            std::cout << "  Reads:  " << reads << std::endl;
            std::cout << "  Writes: " << writes << std::endl;
            std::cout << "  Total:  " << (reads + writes) << std::endl;
        }
    }
};

// Generate random data for benchmark
std::string generate_random_value(int size) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    std::string result;
    result.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    
    for (int i = 0; i < size; ++i) {
        result += alphanum[dis(gen)];
    }
    
    return result;
}

// Configure RocksDB options similar to SPFresh
rocksdb::Options configure_rocksdb(const BenchmarkConfig& config) {
    rocksdb::Options options;
    
    // Basic options
    options.create_if_missing = true;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    
    // SST file size options
    options.target_file_size_base = 128UL * 1024 * 1024;
    options.target_file_size_multiplier = 2;
    options.max_bytes_for_level_base = 16 * 1024UL * 1024 * 1024;
    options.max_bytes_for_level_multiplier = 4;
    options.max_subcompactions = 16;
    options.num_levels = 4;
    options.level0_file_num_compaction_trigger = 1;
    options.level_compaction_dynamic_level_bytes = false;
    options.write_buffer_size = 16UL * 1024 * 1024;
    
    // Direct I/O settings
    if (config.use_direct_io) {
        options.use_direct_io_for_flush_and_compaction = true;
        options.use_direct_reads = true;
    }
    
    // Statistics
    if (config.enable_statistics) {
        options.statistics = rocksdb::CreateDBStatistics();
    }
    
    // Blob options
    if (config.enable_blob) {
        options.enable_blob_files = true;
        options.min_blob_size = 64;
        options.blob_file_size = 8UL << 30;
        options.blob_compression_type = rocksdb::CompressionType::kNoCompression;
        options.enable_blob_garbage_collection = true;
        options.compaction_pri = rocksdb::CompactionPri::kRoundRobin;
        options.blob_garbage_collection_age_cutoff = 0.4;
    }
    
    // Block cache options
    rocksdb::BlockBasedTableOptions table_options;
    table_options.block_cache = rocksdb::NewLRUCache(config.block_cache_gb * 1UL << 30);
    
    // Filter options
    table_options.filter_policy.reset(rocksdb::NewBloomFilterPolicy(10, true));
    table_options.optimize_filters_for_memory = true;
    
    options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
    
    return options;
}

// Worker thread function
void worker_thread(int thread_id, rocksdb::DB* db, BenchmarkConfig& config, 
                  std::atomic<bool>& should_stop, ThroughputTracker& tracker,
                  LatencyStats& latency_stats) {
    
    std::random_device rd;
    std::mt19937 gen(rd() + thread_id);  // Different seed for each thread
    std::uniform_int_distribution<> key_dist(0, config.num_records - 1);
    std::uniform_real_distribution<> op_dist(0.0, 1.0);
    
    // Pre-generate some values to avoid generating them during the benchmark
    std::vector<std::string> sample_values;
    for (int i = 0; i < 10; i++) {
        sample_values.push_back(generate_random_value(config.value_size));
    }
    
    // Calculate sleep time to achieve target QPS across all threads
    double sleep_time_us = 1000000.0 * config.num_threads / config.target_qps;
    
    while (!should_stop) {
        // Generate random key
        int key_num = key_dist(gen);
        std::string key = "key_" + std::to_string(key_num);
        
        // Determine operation type (read or write)
        bool is_read = op_dist(gen) < config.read_ratio;
        
        if (is_read) {
            // Read operation
            std::string value;
            auto start = std::chrono::high_resolution_clock::now();
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(), key, &value);
            auto end = std::chrono::high_resolution_clock::now();
            
            double latency_ms = std::chrono::duration<double, std::milli>(end - start).count();
            latency_stats.add_read_latency(latency_ms);
            
            if (status.ok() || status.IsNotFound()) {
                tracker.reads++;
            }
        } else {
            // Write operation
            std::string value = sample_values[key_num % sample_values.size()];
            auto start = std::chrono::high_resolution_clock::now();
            rocksdb::Status status = db->Put(rocksdb::WriteOptions(), key, value);
            auto end = std::chrono::high_resolution_clock::now();
            
            double latency_ms = std::chrono::duration<double, std::milli>(end - start).count();
            latency_stats.add_write_latency(latency_ms);
            
            if (status.ok()) {
                tracker.writes++;
            }
        }
        
        // Sleep to control QPS
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(sleep_time_us)));
    }
}

// Populate database with initial data
void populate_database(rocksdb::DB* db, const BenchmarkConfig& config) {
    std::cout << "Populating database with " << config.num_records << " records..." << std::endl;
    
    rocksdb::WriteOptions write_options;
    write_options.disableWAL = true;  // Disable WAL for initial load
    
    // Use batching for better performance
    int batch_size = 1000;
    int num_batches = (config.num_records + batch_size - 1) / batch_size;
    
    for (int batch = 0; batch < num_batches; batch++) {
        rocksdb::WriteBatch write_batch;
        
        int start_idx = batch * batch_size;
        int end_idx = std::min((batch + 1) * batch_size, config.num_records);
        
        for (int i = start_idx; i < end_idx; i++) {
            std::string key = "key_" + std::to_string(i);
            std::string value = generate_random_value(config.value_size);
            write_batch.Put(key, value);
        }
        
        rocksdb::Status status = db->Write(write_options, &write_batch);
        if (!status.ok()) {
            std::cerr << "Error populating database: " << status.ToString() << std::endl;
            return;
        }
        
        if (batch % 100 == 0) {
            std::cout << "  Progress: " << (batch * batch_size) << "/" << config.num_records 
                      << " (" << (batch * 100 / num_batches) << "%)" << std::endl;
        }
    }
    
    std::cout << "Database population complete." << std::endl;
}

// Monitor system resources
void monitor_resources(rocksdb::DB* db, std::atomic<bool>& should_stop) {
    while (!should_stop) {
        std::string stats;
        db->GetProperty("rocksdb.stats", &stats);
        
        std::ofstream stats_file("rocksdb_stats.txt", std::ios::app);
        stats_file << "=== " << std::time(nullptr) << " ===" << std::endl;
        stats_file << stats << std::endl;
        stats_file.close();
        
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

int main(int argc, char* argv[]) {
    BenchmarkConfig config;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];
        if (i + 1 >= argc) {
            std::cerr << "Missing value for argument: " << arg << std::endl;
            return 1;
        }
        
        if (arg == "--db_path") {
            config.db_path = argv[i + 1];
        } else if (arg == "--threads") {
            config.num_threads = std::stoi(argv[i + 1]);
        } else if (arg == "--duration") {
            config.duration_seconds = std::stoi(argv[i + 1]);
        } else if (arg == "--qps") {
            config.target_qps = std::stoi(argv[i + 1]);
        } else if (arg == "--read_ratio") {
            config.read_ratio = std::stod(argv[i + 1]);
        } else if (arg == "--value_size") {
            config.value_size = std::stoi(argv[i + 1]);
        } else if (arg == "--num_records") {
            config.num_records = std::stoi(argv[i + 1]);
        } else if (arg == "--block_cache_gb") {
            config.block_cache_gb = std::stoi(argv[i + 1]);
        } else if (arg == "--direct_io") {
            config.use_direct_io = (std::string(argv[i + 1]) == "true");
        } else if (arg == "--enable_blob") {
            config.enable_blob = (std::string(argv[i + 1]) == "true");
        }
    }
    
    // Print configuration
    std::cout << "RocksDB Benchmark Configuration:" << std::endl;
    std::cout << "  Database path: " << config.db_path << std::endl;
    std::cout << "  Threads: " << config.num_threads << std::endl;
    std::cout << "  Duration: " << config.duration_seconds << " seconds" << std::endl;
    std::cout << "  Target QPS: " << config.target_qps << std::endl;
    std::cout << "  Read ratio: " << (config.read_ratio * 100) << "%" << std::endl;
    std::cout << "  Value size: " << config.value_size << " bytes" << std::endl;
    std::cout << "  Number of records: " << config.num_records << std::endl;
    std::cout << "  Block cache size: " << config.block_cache_gb << " GB" << std::endl;
    std::cout << "  Direct I/O: " << (config.use_direct_io ? "enabled" : "disabled") << std::endl;
    std::cout << "  Blob files: " << (config.enable_blob ? "enabled" : "disabled") << std::endl;
    
    // Configure RocksDB
    rocksdb::Options options = configure_rocksdb(config);
    
    // Open database
    rocksdb::DB* db;
    rocksdb::Status status = rocksdb::DB::Open(options, config.db_path, &db);
    
    if (!status.ok()) {
        std::cerr << "Error opening database: " << status.ToString() << std::endl;
        return 1;
    }
    
    // Populate database if needed
    std::string value;
    if (db->Get(rocksdb::ReadOptions(), "key_0", &value).IsNotFound()) {
        populate_database(db, config);
    } else {
        std::cout << "Database already contains data, skipping population." << std::endl;
    }
    
    // Setup tracking
    ThroughputTracker tracker;
    LatencyStats latency_stats;
    std::atomic<bool> should_stop(false);
    
    // Start monitoring thread
    std::thread monitor_thread(monitor_resources, db, std::ref(should_stop));
    
    // Start worker threads
    std::vector<std::thread> threads;
    tracker.start();
    
    for (int i = 0; i < config.num_threads; i++) {
        threads.emplace_back(worker_thread, i, db, std::ref(config), 
                            std::ref(should_stop), std::ref(tracker),
                            std::ref(latency_stats));
    }
    
    // Print progress during the test
    for (int i = 0; i < config.duration_seconds; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        if (i % 10 == 0) {
            tracker.print_stats();
        }
    }
    
    // Stop all threads
    should_stop = true;
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    monitor_thread.join();
    
    // Print final results
    std::cout << "\n=== Benchmark Results ===" << std::endl;
    tracker.print_stats(true);
    latency_stats.print_summary();
    
    // Print RocksDB statistics
    if (options.statistics) {
        std::cout << "\nRocksDB Statistics:" << std::endl;
        std::cout << options.statistics->ToString() << std::endl;
    }
    
    // Close database
    delete db;
    
    return 0;
}