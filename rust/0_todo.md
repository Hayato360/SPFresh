# SPFresh Integration with Rust

## Overview

This document outlines the architecture for integrating SPFresh (a C++ vector search library) with a Rust application. The integration uses FFI (Foreign Function Interface) through the C ABI to bridge between Rust and C++.

## Architecture Diagrams

### API Request Flow

```
+---------------------------------------------------------------------------------------------------+
|                                       API WRITE REQUEST BODY                                      |
|                                       (JSON Payload from Client)                                  |
+---------------------------------------------------------------------------------------------------+
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  upsert_rows: Array of Objects (Optional)                                                   |  |
|  |  - Each object: `id` (required), `vector` (optional), `attributes` (other keys)             |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  upsert_columns: Object (Optional)                                                          |  |
|  |  - Object keys: `id` (array), `vector` (array, optional), `attributes` (arrays)             |  |
|  |  - All arrays must be same length. Use `null` for missing values.                           |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  patch_rows: Array of Objects (Optional)                                                    |  |
|  |  - Identical to `upsert_rows` but only specified keys are written.                          |  |
|  |  - `vector` key cannot be patched. Ignored if ID doesn't exist.                             |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  patch_columns: Object (Optional)                                                           |  |
|  |  - Identical to `upsert_columns` but only specified keys are written.                       |  |
|  |  - `vector` key cannot be patched. Ignored if ID doesn't exist.                             |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  deletes: Array of Document IDs (Optional)                                                  |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  upsert_condition: Object (Optional)                                                        |  |
|  |  - Condition for `upsert_rows`/`columns`. Evaluated before write.                           |  |
|  |  - Supports `$ref_new` for new values.                                                      |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  patch_condition: Object (Optional)                                                         |  |
|  |  - Like `upsert_condition`, but for `patch_rows`/`columns`.                                 |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  delete_condition: Object (Optional)                                                        |  |
|  |  - Like `upsert_condition`, but for `deletes` (by ID). `$ref_new` is `null`.                |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  delete_by_filter: Object (Optional)                                                        |  |
|  |  - Deletes documents matching a filter. Applied BEFORE other operations.                    |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  distance_metric: String (Optional, required if vector index and no copy_from_namespace)    |  |
|  |  Values: "cosine_distance" | "euclidean_squared"                                            |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  copy_from_namespace: String (Optional)                                                     |  |
|  |  - Copies all documents from specified namespace.                                           |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  schema: Object (Optional)                                                                  |  |
|  |  - Manually specify schema for attributes (e.g., UUID, full-text search, filterable).       |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
|  +---------------------------------------------------------------------------------------------+  |
|  |  encryption: Object (Optional)                                                              |  |
|  |  - Configure Customer Managed Encryption Key (CMEK).                                        |  |
|  +---------------------------------------------------------------------------------------------+  |
|                                                                                                   |
+---------------------------------------------------------------------------------------------------+
         |
         | (This JSON Payload is sent to your Axum API)
         |
         v
```#
## Rust Application Flow

```
+-----------------------------------------------------------------------------------------------------------------+
|                                                   RUST APPLICATION                                              |
|                                                                                                                 |
|   +---------------------------------------+                                                                     |
|   |          Rust Code (High-Level)       |                                                                     |
|   | (e.g., Axum Handlers, Business Logic) |                                                                     |
|   |                                       |                                                                     |
|   |   - Receives API Request Payload      |                                                                     |
|   |   - Parses & Validates Data           |                                                                     |
|   +---------------------------------------+                                                                     |
|                   |                                                                                             |
|                   | IF (Payload has text AND no vector)                                                         |
|                   v                                                                                             |
|   +---------------------------------------+                                                                     |
|   |       FAST EMBEDDING LIBRARY          |                                                                     |
|   | (High-performance vector generation)  |                                                                     |
|   |   (Converts text to high-dim vectors) |                                                                     |
|   +---------------------------------------+                                                                     |
|                   |                                                                                             |
|                   | Prepared Data (Now with vectors, ready for index)                                           |
|                   v                                                                                             |
|   +---------------------------------------+                                                                     |
|   |      Rust FFI Bindings                |                                                                     |
|   |   (Defined with extern "C" { ... })   |                                                                     |
|   |   (Declares C-compatible functions)   |                                                                     |
|   |   (Marshals Rust data types to C-compatible types)                                                          |
|   +---------------------------------------+                                                                     |
|                   |                                                                                             |
|                   | Uses C ABI (Stable, Standardized)                                                           |
|                   v                                                                                             |
+-----------------------------------------------------------------------------------------------------------------+
```

### C++ Integration Flow

```
+-----------------------------------------------------------------------------------------------------------------+
|                                                   C++ LIBRARY                                                   |
|                                                                                                                 |
|   +---------------------------------------+                                                                     |
|   |      C++ FFI Interface Layer          |                                                                     |
|   |   (extern "C" { ... } declarations)   |                                                                     |
|   |   (Converts C types to C++ types)     |                                                                     |
|   |   (Handles memory management)         |                                                                     |
|   +---------------------------------------+                                                                     |
|                   |                                                                                             |
|                   | Native C++ types and objects                                                                |
|                   v                                                                                             |
|   +---------------------------------------+                                                                     |
|   |      SPFresh Core Components          |                                                                     |
|   |                                       |                                                                     |
|   |   +-------------------------------+   |                                                                     |
|   |   |        SPANN Layer            |   |                                                                     |
|   |   | (SSD-optimized index struct)  |   |                                                                     |
|   |   +-------------------------------+   |                                                                     |
|   |                 |                     |                                                                     |
|   |                 v                     |                                                                     |
|   |   +-------------------------------+   |                                                                     |
|   |   |        SPTAG Layer            |   |                                                                     |
|   |   | (Base vector search engine)   |   |                                                                     |
|   |   +-------------------------------+   |                                                                     |
|   |                                       |                                                                     |
|   +---------------------------------------+                                                                     |
|                   |                                                                                             |
|                   | Vector search results                                                                       |
|                   v                                                                                             |
|   +---------------------------------------+                                                                     |
|   |      C++ to C Conversion Layer        |                                                                     |
|   |   (Converts C++ results to C types)   |                                                                     |
|   |   (Handles memory for return values)  |                                                                     |
|   +---------------------------------------+                                                                     |
|                   |                                                                                             |
|                   | C-compatible return values                                                                  |
|                   v                                                                                             |
+-----------------------------------------------------------------------------------------------------------------+
```### S
PFresh Internal Architecture

```
+----------------------------------+
|      RAW HIGH-DIMENSION DATA     |  <--- data from C ABI
| (Vectors for Initial Index Build)|
+----------------------------------+
               |
               v
+-------------------------------------------------------+
|                                                       |
|        SPANN INDEX BUILDING (Offline Phase)           |
|  (Creating Initial Clustering & Centroid Index)       |
|                                                       |
+-------------------------------------------------------+
               |
               v
+----------------------------------------------------------------------------------------------+
|                                                                                              |
|                          SPANN INDEX (CORE / PRIMARY INDEX)                                  |
|       (Hybrid Disk-Memory for Billion-Scale Approximate Nearest Neighbor Search)             |
|                                                                                              |
|   +--------------------------------------------+                                             |
|   |     CENTROID INDEX (In-Memory using SPTAG) |                                             |
|   |  (Fast coarse search, tree-graph structure)|                                             |
|   +----------------------+---------------------+                                             |
|                          |                                                                   |
|                          v                                                                   |
|                 +------------------------------------+    (On-Disk Storage)                  |
|                 |     * POSTING LISTS (On-Disk) *   |<-----------------------------------+  |
|                 | (Grouped High-Dim Vectors by cluster) |                                   |
|                 +------------------------------------+                                   |  |
|                          ^                                                              |  |
|                          |                                                              |  |
|                +------------------------+                                               |  |
|                |   QUERY PROCESSING     |                                               |  |
|                |------------------------|                                               |  |
|                | 1. Search Centroid Index (RAM)     |                                     |
|                | 2. Select relevant clusters         |                                     |
|                | 3. Load Posting Lists from disk     |                                     |
|                | 4. Refine and rank results          |                                     |
|                +------------------------+                                               |  |
|                          ^                                                              |  |
|                          |                                                              |  |
+--------------------------+--------------------------------------------------------------+  |
                           | Query Vector                                                    |
                           v                                                                 |
                +----------------------------+                                              |
                |       QUERY RESULTS        |                                              |
                | (Approximate Neighbors)   |                                              |
                +----------------------------+                                              |
                           |                                                                 |
                           |                                                                 |
                           | (Update flow continues below)                                   |
                           v                                                                 |
+----------------------------------------------------------------------------------------------+
|                                                                                              |
|                           SPFRESH (UPDATE LAYER)                                             |
|       (Handles Incremental Inserts & Deletes to keep SPANN Index fresh)                      |
|                                                                                              |
|   +------------------------------------------+       +------------------------------------+ |
|   |     IN-MEMORY MEMTABLE (Buffer)          |<------+ Incoming New / Deleted Vectors     | |
|   | (Temp insert buffer & tombstone storage) |       |     (Dynamic Data Stream)          | |
|   +------------------------------------------+       +------------------------------------+ |
|                           |                                                                 |
|                           | (Periodic Compaction & Rebalancing)                             |
|                           v                                                                 |
|   +---------------------------------------------------------------+                         |
|   |    LIRE (Lightweight Incremental Rebalancer)                  |                         |
|   |  - Processes Memtable                                         |                         |
|   |  - Detects index imbalance                                    |                         |
|   |  - Merges changes into SPANN index                            |                         |
|   +---------------------------------------------------------------+                         |
|                           |                                                                 |
|                           v                                                                 |
+----------------------------------------------------------------------------------------------+
|                                                                                              |
|      SPANN INDEX (CORE / PRIMARY INDEX) — NOW UPDATED & OPTIMIZED                           |
|                                                                                              |
+----------------------------------------------------------------------------------------------+
```## I
mplementation Plan

### 1. Rust API Layer

The Rust API layer will be built using Axum, a modern web framework for Rust. This layer will:

- Handle HTTP requests and responses
- Parse and validate JSON payloads
- Manage authentication and authorization
- Route requests to appropriate handlers

```rust
// Example Axum handler for write operations
async fn handle_write(
    Path(namespace): Path<String>,
    State(app_state): State<AppState>,
    Json(request): Json<WriteRequest>,
) -> Result<Json<WriteResponse>, ApiError> {
    // Validate request
    validate_request(&request)?;
    
    // Process write operation
    let result = app_state.vector_store.write(namespace, request).await?;
    
    // Return response
    Ok(Json(result))
}
```

### 2. Embedding Layer

For documents that contain text but no vector, we'll use FastEmbedding, a high-performance Rust embedding library:

```rust
// FastEmbedding service
pub struct FastEmbedding {
    model: EmbeddingModel,
    batch_size: usize,
    device: Device,
}

impl FastEmbedding {
    pub fn new(model_name: &str, device: Device) -> Result<Self, EmbeddingError> {
        // Initialize the FastEmbedding model
        let model = EmbeddingModel::from_pretrained(model_name, device)?;
        
        Ok(Self {
            model,
            batch_size: 32,  // Default batch size
            device,
        })
    }
    
    pub async fn embed(&self, text: &str) -> Result<Vec<f32>, EmbeddingError> {
        // Generate embedding vector from text using FastEmbedding
        let embeddings = self.model.encode(&[text], self.batch_size)?;
        Ok(embeddings[0].clone())
    }
    
    pub async fn embed_batch(&self, texts: &[String]) -> Result<Vec<Vec<f32>>, EmbeddingError> {
        // Process texts in batches for optimal performance
        let mut results = Vec::with_capacity(texts.len());
        
        for chunk in texts.chunks(self.batch_size) {
            let batch_embeddings = self.model.encode(chunk, self.batch_size)?;
            results.extend(batch_embeddings);
        }
        
        Ok(results)
    }
}
```

### 3. FFI Layer

The FFI layer will bridge between Rust and C++:

```rust
// C FFI declarations
#[repr(C)]
pub struct SPFreshIndex {
    _private: [u8; 0],  // Opaque type
}

extern "C" {
    fn SPFresh_Create(config: *const SPFreshConfig) -> *mut SPFreshIndex;
    fn SPFresh_AddBatch(index: *mut SPFreshIndex, ids: *const i32, vectors: *const f32, count: i32) -> i32;
    // Other FFI functions...
}

// Safe Rust wrapper
pub struct SPFresh {
    index: *mut SPFreshIndex,
    dimension: usize,
}

impl SPFresh {
    pub fn add_batch(&mut self, ids: &[i32], vectors: &[f32]) -> Result<(), SPFreshError> {
        // Call C function safely
        let result = unsafe {
            SPFresh_AddBatch(
                self.index,
                ids.as_ptr(),
                vectors.as_ptr(),
                ids.len() as i32
            )
        };
        
        if result != 0 {
            return Err(SPFreshError::AddFailed(result));
        }
        
        Ok(())
    }
}
```###
 4. C++ Interface Layer

The C++ interface layer will expose SPFresh functionality through C-compatible functions:

```cpp
// C++ interface (in spfresh_c_api.h)
extern "C" {
    SPFreshIndex* SPFresh_Create(const SPFreshConfig* config);
    int SPFresh_AddBatch(SPFreshIndex* index, const int* ids, const float* vectors, int count);
    int SPFresh_Search(SPFreshIndex* index, const float* query, int k, SPFreshSearchResult* results);
    void SPFresh_Destroy(SPFreshIndex* index);
}

// C++ implementation (in spfresh_c_api.cpp)
SPFreshIndex* SPFresh_Create(const SPFreshConfig* config) {
    try {
        // Create SPFresh index with the given configuration
        auto index = new SPTAG::SSDServing::SPFresh::Index<float>(
            config->dimension,
            static_cast<SPTAG::DistCalcMethod>(config->distance_type),
            config->index_directory
        );
        
        return reinterpret_cast<SPFreshIndex*>(index);
    } catch (...) {
        return nullptr;
    }
}

int SPFresh_AddBatch(SPFreshIndex* index, const int* ids, const float* vectors, int count) {
    try {
        auto spfresh_index = reinterpret_cast<SPTAG::SSDServing::SPFresh::Index<float>*>(index);
        
        // Add vectors to the index
        spfresh_index->AddIndexSPFresh(vectors, count, spfresh_index->GetFeatureDim(), const_cast<int*>(ids));
        
        return 0;  // Success
    } catch (...) {
        return -1;  // Error
    }
}
```

## Next Steps

1. **Setup Project Structure**
   - Create Rust project with Cargo
   - Set up CMake for C++ compilation
   - Configure build system for cross-language integration

2. **Implement Core Components**
   - Rust API layer with Axum
   - Embedding service
   - FFI bindings
   - C++ interface to SPFresh

3. **Develop Write Pipeline**
   - JSON parsing and validation
   - Vector generation (if needed)
   - Batch processing
   - Index updates

4. **Testing**
   - Unit tests for each component
   - Integration tests for the full pipeline
   - Performance benchmarks

5. **Documentation**
   - API documentation
   - Architecture diagrams
   - Usage examples

## References

- [SPFresh GitHub Repository](https://github.com/SPFresh/SPFresh)
- [Rust FFI Documentation](https://doc.rust-lang.org/nomicon/ffi.html)
- [Axum Web Framework](https://github.com/tokio-rs/axum)
- [FastEmbedding Library](https://github.com/example/fastembedding)
- [Rust Embedding Libraries](https://github.com/huggingface/tokenizers)