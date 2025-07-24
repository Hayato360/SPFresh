# Design Document

## Overview

This document outlines the comprehensive design for modernizing the SPFresh vector search system architecture. The modernization transforms SPFresh into a production-ready, enterprise-grade vector search platform with an 8-layer architecture that combines high-performance C++ core components with modern Rust API layers, multi-cloud storage abstraction, and advanced embedding capabilities.

The design leverages the existing SPFresh/SPANN vector search engine (Microsoft SPTAG-based) while adding modern infrastructure layers for scalability, reliability, and maintainability. The system will support high-throughput operations (10K+ QPS), multi-billion record datasets, and enterprise features like encryption, multi-tenancy, and comprehensive monitoring.

## Architecture

### High-Level Architecture Overview

The modernized SPFresh system implements an 8-layer architecture designed for separation of concerns, scalability, and maintainability:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         1. CLIENT LAYER                                     │
│  Web Apps │ Mobile Apps │ Desktop Apps │ Analytics │ Batch Processing      │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                      2. LOAD BALANCER LAYER (GCP)                          │
│  Health Checks │ Traffic Routing │ SSL Termination │ Auto-scaling          │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                       3. API LAYER (Rust/Axum)                             │
│  REST Endpoints │ Request Validation │ Authentication │ Rate Limiting       │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                    4. EMBEDDING LAYER (FastEmbedding)                       │
│  Text Processing │ Vector Generation │ Batch Processing │ SIMD Acceleration │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        5. RUST FFI LAYER                                   │
│  Safe Bindings │ Memory Management │ Error Handling │ Type Safety          │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                      6. C++ INTERFACE LAYER                                │
│  C API Wrapper │ Exception Handling │ Resource Management                  │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                       7. SPFRESH CORE (C++)                                │
│  Update Layer │ SPANN Index │ SPTAG Base │ Vector Algorithms               │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                    8. STORAGE LAYER (Multi-Cloud)                          │
│  RocksDB │ Cloud Storage │ Memory Cache │ Persistent Disk                  │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### Layer 1: Client Layer

**Purpose**: Provide comprehensive client support for various application types and access patterns.

**Components**:
- **Web Application Clients**: Browser-based applications using REST API
- **Mobile Application Clients**: iOS/Android apps with optimized request patterns
- **Desktop Application Clients**: Native desktop applications
- **Analytics Clients**: Real-time analytics and monitoring systems
- **Batch Processing Clients**: High-throughput batch operations

**Interfaces**:
- HTTP/JSON REST API for standard operations
- WebSocket connections for real-time updates
- gRPC for high-performance batch operations
- GraphQL for flexible data querying

**Design Decisions**:
- Support multiple communication protocols to accommodate different client needs
- Implement client SDKs in popular languages (JavaScript, Python, Java, Go)
- Provide comprehensive API documentation with OpenAPI specifications

### Layer 2: Load Balancer Layer (GCP)

**Purpose**: Provide intelligent traffic distribution, health monitoring, and auto-scaling capabilities.

**Components**:

**Health Check System**:
```yaml
health_checks:
  - path: /health
    interval: 10s
    timeout: 5s
    healthy_threshold: 2
    unhealthy_threshold: 3
  - path: /ready
    interval: 30s
    timeout: 10s
```

**Traffic Routing Strategies**:
- **Round Robin**: Equal distribution across healthy instances
- **Weighted Routing**: Based on instance capacity and performance
- **Geo-based Routing**: Route to nearest regional deployment
- **Least Connections**: Route to instance with fewest active connections

**Advanced Features**:
- **SSL Termination**: Handle TLS encryption/decryption at load balancer
- **DDoS Protection**: Rate limiting and traffic analysis
- **Circuit Breaker**: Automatic failover when services become unhealthy
- **Session Affinity**: Sticky sessions for stateful operations

**Auto-scaling Configuration**:
```yaml
auto_scaling:
  min_instances: 3
  max_instances: 50
  target_cpu_utilization: 70%
  scale_up_cooldown: 300s
  scale_down_cooldown: 600s
```

### Layer 3: API Layer (Rust/Axum)

**Purpose**: Provide a modern, safe, and high-performance HTTP API with comprehensive validation and security.

**Core Components**:

**Axum Web Server Configuration**:
```rust
use axum::{
    routing::{get, post, patch, delete},
    Router, middleware,
};
use tower::ServiceBuilder;
use tower_http::{
    cors::CorsLayer,
    trace::TraceLayer,
    compression::CompressionLayer,
};

fn create_app() -> Router {
    Router::new()
        .route("/v1/namespaces/:namespace/upsert", post(upsert_handler))
        .route("/v1/namespaces/:namespace/search", post(search_handler))
        .route("/v1/namespaces/:namespace/update", patch(update_handler))
        .route("/v1/namespaces/:namespace/batch-delete", delete(delete_handler))
        .layer(
            ServiceBuilder::new()
                .layer(TraceLayer::new_for_http())
                .layer(CompressionLayer::new())
                .layer(CorsLayer::permissive())
                .layer(middleware::from_fn(auth_middleware))
                .layer(middleware::from_fn(rate_limit_middleware))
        )
}
```

**Request Validation System**:
```rust
use serde::{Deserialize, Serialize};
use validator::{Validate, ValidationError};

#[derive(Debug, Deserialize, Validate)]
pub struct UpsertRequest {
    #[validate(length(min = 1, max = 1000))]
    pub upsert_rows: Vec<UpsertRow>,
}

#[derive(Debug, Deserialize, Validate)]
pub struct UpsertRow {
    #[validate(length(min = 1, max = 255))]
    pub id: String,
    #[validate(custom = "validate_vector")]
    pub vector: Option<Vec<f32>>,
    pub metadata: Option<serde_json::Value>,
}
```

**Authentication and Authorization**:
- JWT-based authentication with configurable providers
- Role-based access control (RBAC) for namespace operations
- API key authentication for service-to-service communication
- Rate limiting per user/API key with configurable limits

**Async Request Handling**:
- Tokio-based async runtime for optimal performance
- Connection pooling for database and external service connections
- Request timeout handling with configurable limits
- Graceful shutdown with connection draining

### Layer 4: Embedding Layer (FastEmbedding)

**Purpose**: Provide sophisticated text-to-vector conversion with high performance and batch processing capabilities.

**Text Preprocessing Pipeline**:
```rust
pub struct TextPreprocessor {
    tokenizer: Tokenizer,
    max_length: usize,
    normalization_config: NormalizationConfig,
}

impl TextPreprocessor {
    pub fn process(&self, text: &str) -> ProcessedText {
        let normalized = self.normalize_text(text);
        let tokens = self.tokenizer.tokenize(&normalized);
        let truncated = self.truncate_tokens(tokens, self.max_length);
        ProcessedText::new(truncated)
    }
}
```

**Vector Generation Engine**:
```rust
pub struct VectorGenerator {
    model: EmbeddingModel,
    dimension: usize,
    device: Device,
}

impl VectorGenerator {
    pub async fn generate_batch(&self, texts: Vec<ProcessedText>) -> Result<Vec<Vec<f32>>> {
        let batch_size = 32; // Optimal batch size
        let mut results = Vec::new();
        
        for chunk in texts.chunks(batch_size) {
            let vectors = self.model.encode_batch(chunk).await?;
            results.extend(vectors);
        }
        
        Ok(results)
    }
}
```

**Batch Processing System**:
- **Dynamic Batching**: Adjust batch size based on system load and memory availability
- **SIMD Acceleration**: Utilize CPU SIMD instructions for vector operations
- **Multi-threading**: Parallel processing across multiple CPU cores
- **Memory Management**: Efficient memory allocation and deallocation for large batches

**Performance Optimizations**:
- Model caching to avoid repeated loading
- Tensor reuse to minimize memory allocations
- Asynchronous processing pipeline
- Hardware acceleration support (GPU/TPU when available)

### Layer 5: Rust FFI Layer

**Purpose**: Provide safe and efficient interoperability between Rust and C++ components.

**Safe Wrapper Design**:
```rust
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_float};

pub struct SPFreshIndex {
    inner: *mut ffi::SPFreshIndexHandle,
}

impl SPFreshIndex {
    pub fn new(config: &IndexConfig) -> Result<Self, SPFreshError> {
        let config_ptr = config.to_c_struct()?;
        let handle = unsafe { ffi::spfresh_create_index(config_ptr) };
        
        if handle.is_null() {
            return Err(SPFreshError::CreationFailed);
        }
        
        Ok(SPFreshIndex { inner: handle })
    }
    
    pub fn search(&self, query: &[f32], k: usize) -> Result<SearchResults, SPFreshError> {
        let mut results = Vec::with_capacity(k);
        let mut scores = Vec::with_capacity(k);
        
        let status = unsafe {
            ffi::spfresh_search(
                self.inner,
                query.as_ptr(),
                query.len(),
                k,
                results.as_mut_ptr(),
                scores.as_mut_ptr(),
            )
        };
        
        if status != 0 {
            return Err(SPFreshError::SearchFailed(status));
        }
        
        unsafe {
            results.set_len(k);
            scores.set_len(k);
        }
        
        Ok(SearchResults::new(results, scores))
    }
}

impl Drop for SPFreshIndex {
    fn drop(&mut self) {
        unsafe {
            ffi::spfresh_destroy_index(self.inner);
        }
    }
}
```

**Memory Management Strategy**:
- RAII patterns for automatic resource cleanup
- Smart pointers for shared ownership
- Buffer management for large data transfers
- Memory pool allocation for frequent operations

**Error Handling System**:
```rust
#[derive(Debug, thiserror::Error)]
pub enum SPFreshError {
    #[error("Index creation failed")]
    CreationFailed,
    #[error("Search operation failed with code: {0}")]
    SearchFailed(i32),
    #[error("Invalid vector dimension: expected {expected}, got {actual}")]
    InvalidDimension { expected: usize, actual: usize },
    #[error("Memory allocation failed")]
    OutOfMemory,
}
```

### Layer 6: C++ Interface Layer

**Purpose**: Provide a C-compatible interface to the SPFresh C++ core while maintaining exception safety.

**C API Implementation**:
```cpp
extern "C" {
    SPFreshIndexHandle* spfresh_create_index(const SPFreshConfig* config) {
        try {
            auto index = std::make_unique<SPTAG::SSDServing::SPFresh::Index<float>>();
            
            // Configure index based on config parameters
            index->SetParameter("DistCalcMethod", config->distance_metric);
            index->SetParameter("IndexAlgoType", "SPANN");
            index->SetParameter("VectorType", "Float");
            index->SetParameter("DimCount", std::to_string(config->dimension));
            
            if (!index->Initialize()) {
                return nullptr;
            }
            
            return reinterpret_cast<SPFreshIndexHandle*>(index.release());
        } catch (const std::exception& e) {
            // Log error
            return nullptr;
        }
    }
    
    int spfresh_search(SPFreshIndexHandle* handle, const float* query, 
                      size_t query_len, size_t k, int* results, float* scores) {
        try {
            auto* index = reinterpret_cast<SPTAG::SSDServing::SPFresh::Index<float>*>(handle);
            
            SPTAG::QueryResult query_result(query, k, true);
            index->SearchIndex(query_result);
            
            for (size_t i = 0; i < k && i < query_result.GetResultNum(); ++i) {
                results[i] = query_result.GetResult(i)->VID;
                scores[i] = query_result.GetResult(i)->Dist;
            }
            
            return 0; // Success
        } catch (const std::exception& e) {
            return -1; // Error
        }
    }
}
```

**Exception Handling Strategy**:
- All C++ exceptions caught at the interface boundary
- Error codes returned to Rust layer for proper error handling
- Logging of C++ exceptions for debugging
- Resource cleanup in case of exceptions

### Layer 7: SPFresh Core (C++)

**Purpose**: Provide high-performance vector search capabilities using the enhanced SPFresh/SPANN architecture.

**SPFresh Update Layer**:
```cpp
namespace SPTAG::SSDServing::SPFresh {
    class UpdateLayer {
    private:
        std::unique_ptr<Memtable> memtable_;
        std::unique_ptr<LIRERebalancer> rebalancer_;
        std::atomic<bool> rebalancing_active_;
        
    public:
        ErrorCode AddVectors(const std::vector<VectorData>& vectors) {
            // Add to memtable
            auto result = memtable_->Insert(vectors);
            if (result != ErrorCode::Success) {
                return result;
            }
            
            // Check if rebalancing is needed
            if (memtable_->Size() > rebalance_threshold_ && !rebalancing_active_) {
                TriggerRebalancing();
            }
            
            return ErrorCode::Success;
        }
        
    private:
        void TriggerRebalancing() {
            rebalancing_active_ = true;
            std::thread([this]() {
                rebalancer_->Rebalance(memtable_.get());
                rebalancing_active_ = false;
            }).detach();
        }
    };
}
```

**SPANN Index Layer Enhancement**:
```cpp
class EnhancedSPANNIndex {
private:
    std::unique_ptr<CentroidIndex> centroid_index_;  // In-memory
    std::unique_ptr<PostingListManager> posting_lists_;  // On-disk
    std::shared_ptr<RocksDBIO> storage_backend_;
    
public:
    ErrorCode Search(const float* query, size_t k, SearchResults& results) {
        // Step 1: Search centroid index
        std::vector<int> candidate_clusters;
        centroid_index_->FindNearestClusters(query, candidate_clusters);
        
        // Step 2: Load and search posting lists
        std::vector<VectorCandidate> candidates;
        for (int cluster_id : candidate_clusters) {
            auto posting_list = posting_lists_->LoadPostingList(cluster_id);
            posting_list->SearchCandidates(query, candidates);
        }
        
        // Step 3: Rank and return top-k
        std::partial_sort(candidates.begin(), candidates.begin() + k, candidates.end());
        results.SetResults(candidates, k);
        
        return ErrorCode::Success;
    }
};
```

### Layer 8: Storage Layer (Multi-Cloud)

**Purpose**: Provide scalable, durable, and high-performance storage with multi-cloud support.

**Enhanced RocksDB Integration**:
```cpp
class OptimizedRocksDBController : public RocksDBIO {
private:
    rocksdb::DB* primary_db_;
    std::vector<std::unique_ptr<rocksdb::DB>> shard_dbs_;
    std::unique_ptr<ConsistentHashRing> hash_ring_;
    
public:
    OptimizedRocksDBController(const RocksDBConfig& config) {
        // Primary database configuration
        rocksdb::Options primary_options;
        ConfigurePrimaryDB(primary_options, config);
        
        // Shard configuration for horizontal scaling
        ConfigureShards(config.shard_count);
        
        // Initialize consistent hashing
        hash_ring_ = std::make_unique<ConsistentHashRing>(config.shard_count);
    }
    
private:
    void ConfigurePrimaryDB(rocksdb::Options& options, const RocksDBConfig& config) {
        // Memory optimizations
        options.write_buffer_size = 64UL * 1024 * 1024;  // 64MB
        options.max_write_buffer_number = 4;
        
        // Cache optimizations
        rocksdb::BlockBasedTableOptions table_options;
        table_options.block_cache = rocksdb::NewLRUCache(8UL << 30);  // 8GB
        table_options.row_cache = rocksdb::NewLRUCache(2UL << 30);    // 2GB
        
        // Compression settings
        options.compression = rocksdb::CompressionType::kLZ4Compression;
        options.bottommost_compression = rocksdb::CompressionType::kZSTD;
        
        // Blob storage for large values
        options.enable_blob_files = true;
        options.min_blob_size = 256;
        options.blob_compression_type = rocksdb::CompressionType::kLZ4Compression;
        
        options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
    }
};
```

**Multi-Cloud Storage Abstraction**:
```rust
pub trait CloudStorage: Send + Sync {
    async fn put_object(&self, key: &str, data: &[u8]) -> Result<(), StorageError>;
    async fn get_object(&self, key: &str) -> Result<Vec<u8>, StorageError>;
    async fn delete_object(&self, key: &str) -> Result<(), StorageError>;
    async fn list_objects(&self, prefix: &str) -> Result<Vec<String>, StorageError>;
}

pub struct MultiCloudStorage {
    primary: Box<dyn CloudStorage>,
    replicas: Vec<Box<dyn CloudStorage>>,
    consistency_level: ConsistencyLevel,
}

impl MultiCloudStorage {
    pub async fn put_with_replication(&self, key: &str, data: &[u8]) -> Result<(), StorageError> {
        // Write to primary
        self.primary.put_object(key, data).await?;
        
        // Async replication to replicas
        let replication_futures: Vec<_> = self.replicas
            .iter()
            .map(|replica| replica.put_object(key, data))
            .collect();
            
        match self.consistency_level {
            ConsistencyLevel::Eventual => {
                tokio::spawn(async move {
                    futures::future::join_all(replication_futures).await;
                });
            }
            ConsistencyLevel::Strong => {
                futures::future::try_join_all(replication_futures).await?;
            }
        }
        
        Ok(())
    }
}
```

## Data Models

### Core Data Structures

**Vector Document Model**:
```rust
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct VectorDocument {
    pub id: String,
    pub vector: Vec<f32>,
    pub metadata: HashMap<String, serde_json::Value>,
    pub namespace: String,
    pub created_at: DateTime<Utc>,
    pub updated_at: DateTime<Utc>,
    pub version: u64,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SearchResult {
    pub id: String,
    pub score: f32,
    pub metadata: HashMap<String, serde_json::Value>,
    pub vector: Option<Vec<f32>>,  // Optional for bandwidth optimization
}
```

**Index Configuration Model**:
```rust
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct IndexConfiguration {
    pub namespace: String,
    pub dimension: usize,
    pub distance_metric: DistanceMetric,
    pub index_type: IndexType,
    pub storage_config: StorageConfiguration,
    pub performance_config: PerformanceConfiguration,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum DistanceMetric {
    Cosine,
    Euclidean,
    DotProduct,
    Manhattan,
}
```

### Schema Management

**Dynamic Schema System**:
```rust
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct NamespaceSchema {
    pub namespace: String,
    pub fields: HashMap<String, FieldDefinition>,
    pub vector_field: String,
    pub created_at: DateTime<Utc>,
    pub version: u32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FieldDefinition {
    pub field_type: FieldType,
    pub indexed: bool,
    pub filterable: bool,
    pub required: bool,
    pub default_value: Option<serde_json::Value>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum FieldType {
    Text,
    Number,
    Boolean,
    Vector { dimension: usize },
    Array { element_type: Box<FieldType> },
    Object,
}
```

## Error Handling

### Comprehensive Error System

```rust
#[derive(Debug, thiserror::Error)]
pub enum SPFreshError {
    // API Layer Errors
    #[error("Invalid request: {message}")]
    InvalidRequest { message: String },
    
    #[error("Authentication failed: {reason}")]
    AuthenticationFailed { reason: String },
    
    #[error("Authorization failed: insufficient permissions")]
    AuthorizationFailed,
    
    // Embedding Layer Errors
    #[error("Embedding generation failed: {reason}")]
    EmbeddingFailed { reason: String },
    
    #[error("Text preprocessing failed: {reason}")]
    PreprocessingFailed { reason: String },
    
    // Core Engine Errors
    #[error("Index operation failed: {operation}")]
    IndexOperationFailed { operation: String },
    
    #[error("Search failed: {reason}")]
    SearchFailed { reason: String },
    
    // Storage Layer Errors
    #[error("Storage operation failed: {operation}")]
    StorageFailed { operation: String },
    
    #[error("Data corruption detected in {component}")]
    DataCorruption { component: String },
    
    // System Errors
    #[error("Resource exhausted: {resource}")]
    ResourceExhausted { resource: String },
    
    #[error("Timeout occurred during {operation}")]
    Timeout { operation: String },
}
```

### Error Recovery Strategies

**Circuit Breaker Pattern**:
```rust
pub struct CircuitBreaker {
    state: Arc<Mutex<CircuitState>>,
    failure_threshold: usize,
    recovery_timeout: Duration,
    failure_count: Arc<AtomicUsize>,
}

impl CircuitBreaker {
    pub async fn call<F, T, E>(&self, operation: F) -> Result<T, CircuitBreakerError<E>>
    where
        F: Future<Output = Result<T, E>>,
    {
        match self.get_state() {
            CircuitState::Closed => {
                match operation.await {
                    Ok(result) => {
                        self.reset_failure_count();
                        Ok(result)
                    }
                    Err(e) => {
                        self.record_failure();
                        Err(CircuitBreakerError::OperationFailed(e))
                    }
                }
            }
            CircuitState::Open => {
                Err(CircuitBreakerError::CircuitOpen)
            }
            CircuitState::HalfOpen => {
                // Allow one request to test if service is recovered
                match operation.await {
                    Ok(result) => {
                        self.close_circuit();
                        Ok(result)
                    }
                    Err(e) => {
                        self.open_circuit();
                        Err(CircuitBreakerError::OperationFailed(e))
                    }
                }
            }
        }
    }
}
```

## Testing Strategy

### Multi-Layer Testing Approach

**Unit Testing**:
- Individual component testing for each layer
- Mock implementations for external dependencies
- Property-based testing for vector operations
- Performance benchmarks for critical paths

**Integration Testing**:
- End-to-end API testing with real data
- Cross-layer integration validation
- Database integration testing
- Cloud storage integration testing

**Performance Testing**:
- Load testing with 10K+ QPS
- Stress testing with large datasets (1B+ vectors)
- Latency testing under various conditions
- Memory usage and leak detection

**Chaos Engineering**:
- Network partition simulation
- Service failure injection
- Resource exhaustion testing
- Data corruption recovery testing

### Test Infrastructure

```rust
#[cfg(test)]
mod tests {
    use super::*;
    use tokio_test;
    
    #[tokio::test]
    async fn test_end_to_end_vector_operations() {
        let test_env = TestEnvironment::new().await;
        
        // Test vector insertion
        let vectors = generate_test_vectors(1000);
        let upsert_result = test_env.api_client
            .upsert_vectors("test_namespace", vectors)
            .await
            .expect("Upsert should succeed");
            
        // Test vector search
        let query_vector = generate_random_vector(384);
        let search_results = test_env.api_client
            .search_vectors("test_namespace", query_vector, 10)
            .await
            .expect("Search should succeed");
            
        assert_eq!(search_results.len(), 10);
        
        // Cleanup
        test_env.cleanup().await;
    }
}
```

This comprehensive design provides a solid foundation for implementing the modernized SPFresh architecture with all the required layers, components, and features while maintaining high performance, scalability, and reliability.