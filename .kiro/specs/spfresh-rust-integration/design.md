# Design Document

## Overview

This document outlines the design for integrating SPFresh (a C++ vector search library) with a Rust application. The integration aims to create a high-performance vector search system that leverages the speed and efficiency of SPFresh's C++ implementation while providing a modern, safe API through Rust.

The system will follow a layered architecture with clear separation of concerns, allowing for maintainability, testability, and future extensibility. The design incorporates best practices from both Rust and C++ ecosystems, ensuring type safety, memory safety, and optimal performance.

## Architecture

### High-Level Architecture

The system is composed of the following layers:

1. **API Layer (Rust/Axum)** - Handles HTTP requests and responses
2. **Embedding Layer (Rust)** - Converts text to vectors using FastEmbedding
3. **FFI Layer (Rust)** - Provides safe Rust bindings to the C++ SPFresh library
4. **C++ Interface Layer** - Exposes SPFresh functionality through C-compatible functions
5. **SPFresh Core (C++)** - The core vector search engine components
6. **Storage Layer (GCP)** - Persistent storage for vectors and metadata

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              CLIENT APPLICATIONS                             │
├──────────────────────────────────┬──────────────────────────────────────────┤
│         REST API Clients         │          Streaming Clients                │
│                                  │                                           │
│ ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐      │
│ │ Web App  │  │  Mobile  │  │ Desktop  │  │ Real-time│  │ Batch    │      │
│ │ Frontend │  │   App    │  │   App    │  │ Analytics│  │ Processor│      │
│ │   🌐     │  │    📱    │  │    💻    │  │    📊    │  │    ⚙️    │      │
│ └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘      │
│      │              │              │              │              │           │
│      │HTTP/JSON     │HTTP/JSON     │HTTP/JSON     │WebSockets    │gRPC       │
│      │              │              │              │              │           │
└──────────────────────────────────┬──────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                           LOAD BALANCER LAYER                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    GCP Load Balancer 🔄                             │    │
│  ├─────────────────────────────────────────────────────────────────────┤    │
│  │                                                                     │    │
│  │  ┌─────────────────────┐    ┌─────────────────────┐                 │    │
│  │  │   Health Checks     │    │   Traffic Routing   │                 │    │
│  │  │ • Service health    │    │ • Round robin       │                 │    │
│  │  │ • Response time     │    │ • Weighted routing  │                 │    │
│  │  │ • Error rates       │    │ • Geo-based routing │                 │    │
│  │  └─────────────────────┘    └─────────────────────┘                 │    │
│  │                                                                     │    │
│  │  ┌─────────────────────────────────────────────────────────────────┐ │    │
│  │  │                   Load Balancing Features                       │ │    │
│  │  │ • SSL termination           • Rate limiting                     │ │    │
│  │  │ • DDoS protection          • Request routing                    │ │    │
│  │  │ • Auto-scaling triggers    • Circuit breaker                    │ │    │
│  │  │ • Session affinity         • Failover handling                  │ │    │
│  │  └─────────────────────────────────────────────────────────────────┘ │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                   │                                         │
│                    ┌───────────────┼───────────────┐                        │
│                    │               │               │                        │
│                    ▼               ▼               ▼                        │
│            ┌─────────────┐ ┌─────────────┐ ┌─────────────┐                  │
│            │  Write      │ │  Read       │ │  Batch      │                  │
│            │  Optimized  │ │  Optimized  │ │  Processing │                  │
│            └─────────────┘ └─────────────┘ └─────────────┘                  │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                            API LAYER (Rust/Axum)                            │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                       Axum Web Server 🚀                            │    │
│  ├─────────────────────────────────────────────────────────────────────┤    │
│  │ POST /v1/namespaces/{namespace}/upsert                              │    │
│  │ POST /v1/namespaces/{namespace}/upsert-columns                      │    │
│  │ PATCH /v1/namespaces/{namespace}/update                            │    │
│  │ PATCH /v1/namespaces/{namespace}/update-columns                    │    │
│  │ DELETE /v1/namespaces/{namespace}/batch-delete                     │    │
│  │ POST /v1/namespaces/{namespace}/conditional-upsert                 │    │
│  │ PATCH /v1/namespaces/{namespace}/conditional-update                │    │
│  │ DELETE /v1/namespaces/{namespace}/conditional-delete               │    │
│  │ DELETE /v1/namespaces/{namespace}/filter-delete                    │    │
│  │ POST /v1/namespaces/{namespace}/configure-metric                   │    │
│  │ POST/PUT /v1/namespaces/{namespace}/schema                         │    │
│  │ POST/PUT /v1/namespaces/{namespace}/encryption                     │    │
│  │ POST /v1/namespaces/{namespace}/copy                               │    │
│  │                                                                     │    │
│  │ Features:                                                           │    │
│  │ • Async request handling     • Rate limiting                        │    │
│  │ • Request validation         • Authentication/Authorization         │    │
│  │ • Error handling             • Metrics collection                   │    │
│  │ • Logging & tracing          • Health checks                        │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                   │                                         │
└───────────────────────────────────┼─────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         EMBEDDING LAYER (FastEmbedding)                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ╔═══════════════════════════════════════════════════════════════════════╗ │
│  ║                    FastEmbedding Service                               ║ │
│  ╠═══════════════════════════════════════════════════════════════════════╣ │
│  ║                                                                        ║ │
│  ║  ┌─────────────────────┐    ┌─────────────────────┐                   ║ │
│  ║  │ Text Preprocessing  │───▶│ Vector Generation   │                   ║ │
│  ║  │ • Tokenization      │    │ • Model Inference   │                   ║ │
│  ║  │ • Normalization     │    │ • Dimensionality    │                   ║ │
│  ║  │ • Truncation        │    │ • Normalization     │                   ║ │
│  ║  └─────────────────────┘    └─────────────────────┘                   ║ │
│  ║                                      │                                 ║ │
│  ║                                      ▼                                 ║ │
│  ║  ┌─────────────────────────────────────────────────────────────────┐  ║ │
│  ║  │                   Batch Processing Engine                        │  ║ │
│  ║  │ • Optimal batch size: 32                                        │  ║ │
│  ║  │ • Dynamic batching based on load                                │  ║ │
│  ║  │ • SIMD acceleration                                             │  ║ │
│  ║  │ • Multi-threading support                                       │  ║ │
│  ║  └─────────────────────────────────────────────────────────────────┘  ║ │
│  ╚═══════════════════════════════════════════════════════════════════════╝ │
│                                   │                                         │
└───────────────────────────────────┼─────────────────────────────────────────┘
                                   │
                                   ▼┌──
───────────────────────────────────────────────────────────────────────────┐
│                         RUST FFI LAYER (Bindings)                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────┐      ┌───────────────────────┐                  │
│  │   Safe Rust Wrapper   │      │   Memory Management   │                  │
│  ├───────────────────────┤      ├───────────────────────┤                  │
│  │ • Error handling      │      │ • RAII patterns       │                  │
│  │ • Type safety         │      │ • Buffer management   │                  │
│  │ • Null safety         │      │ • Ownership transfer  │                  │
│  │ • Thread safety       │      │ • Resource cleanup    │                  │
│  └───────────┬───────────┘      └───────────┬───────────┘                  │
│              │                               │                              │
│              └───────────────────┬───────────┘                              │
│                                  │                                          │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │                       FFI Function Declarations                        │ │
│  │                                                                        │ │
│  │  extern "C" {                                                          │ │
│  │      fn SPFresh_Create(...) -> *mut SPFreshIndex;                      │ │
│  │      fn SPFresh_AddBatch(...) -> i32;                                  │ │
│  │      fn SPFresh_Search(...) -> i32;                                    │ │
│  │      fn SPFresh_Delete(...) -> i32;                                    │ │
│  │      fn SPFresh_Destroy(...);                                          │ │
│  │  }                                                                     │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
│                                  │                                          │
└──────────────────────────────────┼──────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         C++ INTERFACE LAYER                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │                       C API Implementation                             │ │
│  │                                                                        │ │
│  │  // C++ implementation (in spfresh_c_api.cpp)                         │ │
│  │  SPFreshIndex* SPFresh_Create(const SPFreshConfig* config) {          │ │
│  │      try {                                                            │ │
│  │          auto index = new SPTAG::SSDServing::SPFresh::Index<float>(...│ │
│  │          return reinterpret_cast<SPFreshIndex*>(index);               │ │
│  │      } catch (...) {                                                  │ │
│  │          return nullptr;                                              │ │
│  │      }                                                                │ │
│  │  }                                                                    │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
│                                  │                                          │
└──────────────────────────────────┼──────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SPFRESH CORE (C++)                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │                       SPFresh Update Layer                             │ │
│  │                                                                        │ │
│  │  ┌─────────────────────┐      ┌─────────────────────┐                 │ │
│  │  │   In-Memory Buffer  │      │   LIRE Rebalancer   │                 │ │
│  │  │ (Memtable)          │─────▶│                     │                 │ │
│  │  └─────────────────────┘      └─────────────────────┘                 │ │
│  │                                          │                             │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
│                                             │                               │
│                                             ▼                               │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │                       SPANN Index Layer                                │ │
│  │                                                                        │ │
│  │  ┌─────────────────────┐      ┌─────────────────────┐                 │ │
│  │  │   Centroid Index    │      │   Posting Lists     │                 │ │
│  │  │   (In-Memory)       │─────▶│   (On-Disk)         │                 │ │
│  │  └─────────────────────┘      └─────────────────────┘                 │ │
│  │                                                                        │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
│                                             │                               │
│                                             ▼                               │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │                       SPTAG Base Layer                                 │ │
│  │                                                                        │ │
│  │  ┌─────────────────────┐      ┌─────────────────────┐                 │ │
│  │  │   Vector Index      │      │   Distance Calc     │                 │ │
│  │  │   Algorithms        │─────▶│   Methods           │                 │ │
│  │  └─────────────────────┘      └─────────────────────┘                 │ │
│  │                                                                        │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                         STORAGE LAYER (GCP)                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ╔═════════════════════════════════════════════════════════════════════╗   │
│  ║                    GCP Vector Storage Architecture                   ║   │
│  ╠═════════════════════════════════════════════════════════════════════╣   │
│  ║                                                                      ║   │
│  ║  ┌────────────────────┬─────────────────────────────────────────┐   ║   │
│  ║  │   Storage Type     │            Characteristics              │   ║   │
│  ║  ├────────────────────┼─────────────────────────────────────────┤   ║   │
│  ║  │ Memory (RAM)       │ • Centroid index                        │   ║   │
│  ║  │                    │ • Hot vectors                            │   ║   │
│  ║  │                    │ • Update buffer                          │   ║   │
│  ║  ├────────────────────┼─────────────────────────────────────────┤   ║   │
│  ║  │ Cloud Storage      │ • Vector data files                      │   ║   │
│  ║  │                    │ • WAL (Write-Ahead Log)                  │   ║   │
│  ║  │                    │ • Durable storage                        │   ║   │
│  ║  ├────────────────────┼─────────────────────────────────────────┤   ║   │
│  ║  │ Persistent Disk    │ • Posting lists                          │   ║   │
│  ║  │                    │ • SSD performance                        │   ║   │
│  ║  │                    │ • Optimized for random access            │   ║   │
│  ║  ├────────────────────┼─────────────────────────────────────────┤   ║   │
│  ║  │ BigQuery           │ • Analytics data                         │   ║   │
│  ║  │                    │ • Usage statistics                       │   ║   │
│  ║  │                    │ • Performance metrics                    │   ║   │
│  ║  └────────────────────┴─────────────────────────────────────────┘   ║   │
│  ║                                                                      ║   │
│  ║  GCP Storage Strategy:                                               ║   │
│  ║  • Cloud Storage for durability and scalability                      ║   │
│  ║  • Persistent Disk for performance-critical components               ║   │
│  ║  • Memory cache for frequently accessed vectors                      ║   │
│  ║  • Regional redundancy for high availability                         ║   │
│  ║  • Object lifecycle management for cost optimization                 ║   │
│  ╚═════════════════════════════════════════════════════════════════════╝   │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
#
# Complete Data Flow Journey

### 1. Vector Search Flow (Query Operation)

```
Client Request → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage → Response
```

#### Detailed Steps:

1. **Client Request Initiation**
   - Client sends a search request to `/v1/namespaces/{namespace}/search`
   - Request includes query vector, top_k parameter, and optional filters
   - Example: `{"vector": [0.1, 0.2, 0.3], "top_k": 10, "filters": {"category": "electronics"}}`

2. **API Layer Processing**
   - Axum handler validates the request format and parameters
   - Extracts query vector, top_k, and filters
   - Performs authentication and authorization checks
   - Logs the incoming request with trace ID

3. **Vector Normalization**
   - If distance metric is cosine_distance, normalize the query vector
   - Apply any preprocessing required for the vector

4. **FFI Layer Preparation**
   - Convert Rust data types to C-compatible types
   - Allocate memory for results buffer
   - Prepare search parameters structure

5. **C++ Interface Call**
   - Call `SPFresh_Search` function with prepared parameters
   - Pass query vector, top_k, and filter conditions

6. **SPFresh Core Search**
   - Load centroid index from memory
   - Perform approximate nearest neighbor search on centroid index
   - Identify relevant clusters containing potential matches
   - Load posting lists from disk for identified clusters
   - Perform exact distance calculations on candidate vectors
   - Apply filters to candidate results
   - Sort results by distance

7. **Result Assembly**
   - Collect top_k matching vectors with their distances
   - Retrieve associated metadata for each match
   - Format results in the expected structure

8. **Response Return**
   - Convert C++ results back to Rust types
   - Free allocated memory
   - Format JSON response
   - Return results to client with appropriate status code

### 2. Vector Insertion Flow (Upsert Operation)

```
Client Request → API Layer → Embedding Layer (optional) → FFI Layer → C++ Interface → SPFresh Core → Storage → Response
```

#### Detailed Steps:

1. **Client Request Initiation**
   - Client sends an upsert request to `/v1/namespaces/{namespace}/upsert`
   - Request includes vectors and metadata in row or column format
   - Example: `{"upsert_rows": [{"id": 1, "vector": [0.1, 0.2, 0.3], "title": "Product A"}]}`

2. **API Layer Processing**
   - Axum handler validates the request format and parameters
   - Extracts vectors, IDs, and metadata
   - Performs authentication and authorization checks
   - Logs the incoming request with trace ID

3. **Text-to-Vector Conversion (if needed)**
   - If text is provided without vectors, route to FastEmbedding service
   - FastEmbedding tokenizes and processes text
   - Generate embedding vectors using the model
   - Normalize vectors according to distance metric

4. **Batch Processing**
   - Group vectors into optimal batch sizes
   - Prepare batch metadata

5. **FFI Layer Preparation**
   - Convert Rust data types to C-compatible types
   - Allocate memory for vector data
   - Prepare upsert parameters structure

6. **C++ Interface Call**
   - Call `SPFresh_AddBatch` function with prepared parameters
   - Pass vectors, IDs, and metadata

7. **SPFresh Core Processing**
   - Add vectors to in-memory buffer (Memtable)
   - Write to Write-Ahead Log (WAL) for durability
   - Update in-memory index structures
   - Schedule background rebalancing if needed

8. **Storage Layer Writing**
   - Write WAL entries to GCP Cloud Storage
   - Update metadata in database
   - Confirm successful write

9. **Response Return**
   - Process operation status from C++ layer
   - Format JSON response with success/failure information
   - Return response to client with appropriate status code

### 3. Vector Update Flow (Patch Operation)

```
Client Request → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage → Response
```

#### Detailed Steps:

1. **Client Request Initiation**
   - Client sends a patch request to `/v1/namespaces/{namespace}/update`
   - Request includes document IDs and fields to update
   - Example: `{"patch_rows": [{"id": 1, "title": "Updated Product A"}]}`

2. **API Layer Processing**
   - Axum handler validates the request format and parameters
   - Extracts IDs and fields to update
   - Performs authentication and authorization checks
   - Logs the incoming request with trace ID

3. **Condition Evaluation (if applicable)**
   - If patch_condition is provided, evaluate it against existing data
   - Retrieve current document state from index
   - Apply condition logic
   - Proceed only if condition is met

4. **FFI Layer Preparation**
   - Convert Rust data types to C-compatible types
   - Prepare patch parameters structure

5. **C++ Interface Call**
   - Call `SPFresh_Update` function with prepared parameters
   - Pass IDs and fields to update

6. **SPFresh Core Processing**
   - Locate documents by ID in the index
   - Update metadata fields (vector fields cannot be patched)
   - Write to Write-Ahead Log (WAL) for durability

7. **Storage Layer Writing**
   - Write WAL entries to GCP Cloud Storage
   - Update metadata in database
   - Confirm successful write

8. **Response Return**
   - Process operation status from C++ layer
   - Format JSON response with success/failure information
   - Return response to client with appropriate status code

### 4. Vector Deletion Flow (Delete Operation)

```
Client Request → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage → Response
```

#### Detailed Steps:

1. **Client Request Initiation**
   - Client sends a delete request to `/v1/namespaces/{namespace}/batch-delete`
   - Request includes document IDs to delete
   - Example: `{"deletes": [1, 2, 3]}`

2. **API Layer Processing**
   - Axum handler validates the request format and parameters
   - Extracts IDs to delete
   - Performs authentication and authorization checks
   - Logs the incoming request with trace ID

3. **Condition Evaluation (if applicable)**
   - If delete_condition is provided, evaluate it against existing data
   - Retrieve current document state from index
   - Apply condition logic
   - Proceed only if condition is met

4. **FFI Layer Preparation**
   - Convert Rust data types to C-compatible types
   - Prepare delete parameters structure

5. **C++ Interface Call**
   - Call `SPFresh_Delete` function with prepared parameters
   - Pass IDs to delete

6. **SPFresh Core Processing**
   - Mark documents as deleted in the in-memory index
   - Add delete markers to Write-Ahead Log (WAL)
   - Schedule background garbage collection

7. **Storage Layer Writing**
   - Write delete markers to GCP Cloud Storage
   - Update metadata in database
   - Confirm successful deletion

8. **Response Return**
   - Process operation status from C++ layer
   - Format JSON response with success/failure information
   - Return response to client with appropriate status code

### 5. Filter-Based Deletion Flow (Delete by Filter)

```
Client Request → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage → Response
```

#### Detailed Steps:

1. **Client Request Initiation**
   - Client sends a filter-delete request to `/v1/namespaces/{namespace}/filter-delete`
   - Request includes filter conditions
   - Example: `{"delete_by_filter": {"category": "discontinued"}}`

2. **API Layer Processing**
   - Axum handler validates the request format and parameters
   - Extracts filter conditions
   - Performs authentication and authorization checks
   - Logs the incoming request with trace ID

3. **FFI Layer Preparation**
   - Convert Rust filter conditions to C-compatible types
   - Prepare filter-delete parameters structure

4. **C++ Interface Call**
   - Call `SPFresh_QueryAndDelete` function with prepared parameters
   - Pass filter conditions

5. **SPFresh Core Processing**
   - Execute query to find matching documents
   - Collect IDs of all matching documents
   - Mark documents as deleted in the in-memory index
   - Add delete markers to Write-Ahead Log (WAL)
   - Schedule background garbage collection

6. **Storage Layer Writing**
   - Write delete markers to GCP Cloud Storage
   - Update metadata in database
   - Confirm successful deletion

7. **Response Return**
   - Process operation status from C++ layer
   - Format JSON response with success/failure information and count of deleted items
   - Return response to client with appropriate status code

### 6. Namespace Configuration Flow

```
Client Request → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage → Response
```

#### Detailed Steps:

1. **Client Request Initiation**
   - Client sends a configuration request to `/v1/namespaces/{namespace}/configure-metric`
   - Request includes configuration parameters like distance_metric
   - Example: `{"distance_metric": "cosine_distance"}`

2. **API Layer Processing**
   - Axum handler validates the request format and parameters
   - Extracts configuration parameters
   - Performs authentication and authorization checks
   - Logs the incoming request with trace ID

3. **FFI Layer Preparation**
   - Convert Rust configuration to C-compatible types
   - Prepare configuration parameters structure

4. **C++ Interface Call**
   - Call `SPFresh_Configure` function with prepared parameters
   - Pass configuration settings

5. **SPFresh Core Processing**
   - Apply configuration to the index
   - Update internal settings
   - Initialize necessary components based on configuration

6. **Storage Layer Writing**
   - Write configuration to GCP Cloud Storage
   - Update metadata in database
   - Confirm successful configuration

7. **Response Return**
   - Process operation status from C++ layer
   - Format JSON response with success/failure information
   - Return response to client with appropriate status code## Webhook 
Interfaces

To support integration with external systems, the SPFresh-Rust system will provide webhook interfaces. These webhooks allow for event-driven architecture, enabling real-time notifications when specific events occur within the system.

### Webhook Architecture

```
External Systems                           SPFresh-Rust System
─────────────────                          ────────────────────────────────────────────────
                                           ┌──────────────────────────────────────────────┐
                                           │              EVENT TRIGGERS                  │
                                           │                                              │
                                           │  ┌────────────────────────────────┐          │
                                           │  │      Vector Operations         │          │
                                           │  │                                │          │
                                           │  │  • Insert/Update               │          │
                                           │  │  • Delete                      │          │
                                           │  │  • Search                      │          │
                                           │  └─────────────────┬──────────────┘          │
                                           │                    │                         │
                                           │                    ▼                         │
                                           │  ┌────────────────────────────────┐          │
                                           │  │      Event Detection           │          │
                                           │  │                                │          │
                                           │  │  • Operation completion        │          │
                                           │  │  • Threshold crossing          │          │
                                           │  │  • Error conditions            │          │
                                           │  └─────────────────┬──────────────┘          │
                                           │                    │                         │
                                           └────────────────────┼─────────────────────────┘
                                                                │
                                                                ▼
                                           ┌──────────────────────────────────────────────┐
                                           │              WEBHOOK MANAGER                 │
                                           │                                              │
                                           │  ┌────────────────────────────────┐          │
                                           │  │      Event Processing          │          │
                                           │  │                                │          │
                                           │  │  • Format event payload        │          │
                                           │  │  • Apply filters               │          │
                                           │  └─────────────────┬──────────────┘          │
                                           │                    │                         │
                                           │                    ▼                         │
                                           │  ┌────────────────────────────────┐          │
                                           │  │      Delivery Service          │          │
                                           │  │                                │          │
                                           │  │  • Async HTTP requests         │          │
                                           │  │  • Retry logic                 │          │
                                           │  │  • Backoff strategy            │          │
                                           │  └─────────────────┬──────────────┘          │
                                           │                    │                         │
                                           └────────────────────┼─────────────────────────┘
                                                                │
                                                                ▼
  ┌─────────────┐                                               │
  │  External   │                                               │
  │  System     │◀────────────────────────────────────────────────
  │  Endpoint   │         HTTP POST with JSON payload
  └─────────────┘
```

### Webhook Event Types

1. **Vector Operations Events**
   - `vector.inserted`: Triggered when new vectors are added
   - `vector.updated`: Triggered when existing vectors are updated
   - `vector.deleted`: Triggered when vectors are deleted

2. **System Events**
   - `system.index.rebalanced`: Triggered when the index is rebalanced
   - `system.error`: Triggered when system errors occur
   - `system.threshold`: Triggered when configured thresholds are crossed

### Webhook Payload Format

```json
{
  "event_type": "vector.inserted",
  "timestamp": "2025-07-23T14:30:00Z",
  "namespace": "product-catalog",
  "data": {
    "operation_id": "op-123456",
    "affected_ids": [1, 2, 3],
    "metadata": {
      "vector_count": 3,
      "processing_time_ms": 45
    }
  }
}
```

### Webhook Configuration

Webhooks can be configured through the API:

```
POST /v1/webhooks
{
  "url": "https://example.com/webhook",
  "event_types": ["vector.inserted", "vector.deleted"],
  "namespaces": ["product-catalog"],
  "secret": "webhook_signing_secret"
}
```

### Webhook Security

1. **Signature Verification**
   - Each webhook request includes an HMAC signature in the `X-Signature` header
   - Signature is computed using the webhook secret and request body
   - Recipients should verify the signature to ensure authenticity

2. **Retry with Exponential Backoff**
   - Failed deliveries are retried with exponential backoff
   - Maximum of 5 retry attempts
   - Configurable retry interval

This webhook architecture enables real-time integration with external systems, allowing them to react to events in the SPFresh-Rust system without polling.

## Load Balancer Architecture

### GCP Load Balancer Configuration

The load balancer serves as the entry point for all client requests and provides critical infrastructure services:

#### **Load Balancing Strategy**
```yaml
# GCP Load Balancer Configuration
load_balancer:
  type: "HTTP(S) Load Balancer"
  backend_service:
    protocol: "HTTP"
    port: 8080
    health_check:
      path: "/health"
      interval: 10s
      timeout: 5s
      healthy_threshold: 2
      unhealthy_threshold: 3
  
  routing_rules:
    - path_matcher: "/v1/namespaces/*/search"
      backend_service: "search-optimized-instances"
      load_balancing_scheme: "ROUND_ROBIN"
    
    - path_matcher: "/v1/namespaces/*/upsert*"
      backend_service: "write-optimized-instances" 
      load_balancing_scheme: "LEAST_CONNECTIONS"
    
    - path_matcher: "/v1/namespaces/*/delete*"
      backend_service: "write-optimized-instances"
      load_balancing_scheme: "LEAST_CONNECTIONS"

  auto_scaling:
    min_replicas: 3
    max_replicas: 50
    target_cpu_utilization: 70
    target_rps_per_instance: 1000
```

#### **Traffic Distribution Strategies**

1. **Search Traffic (Read-Heavy)**
   - Round-robin distribution
   - Prefer instances with lower search latency
   - Cache-aware routing to maximize hit rates

2. **Write Traffic (Upsert/Delete)**
   - Least-connections routing
   - Consistent hashing for data locality
   - Write-optimized instance pools

3. **Batch Operations**
   - Dedicated high-memory instances
   - Queue-based processing
   - Separate scaling policies

#### **High Availability Features**

```yaml
availability:
  multi_region: true
  regions: ["us-central1", "us-east1", "europe-west1"]
  
  failover:
    primary_region: "us-central1"
    backup_regions: ["us-east1", "europe-west1"]
    failover_threshold: "50% instance failure"
    
  circuit_breaker:
    failure_threshold: 5
    recovery_timeout: 30s
    half_open_max_calls: 3
```

#### **Performance Optimizations**

- **SSL Termination**: Handle TLS at load balancer level
- **Connection Pooling**: Reuse connections to backend instances
- **Request Compression**: Gzip compression for large payloads
- **CDN Integration**: Cache static responses and embeddings
- **Rate Limiting**: Per-client and global rate limits

## Components and Interfaces

### API Layer Components

1. **Axum Web Server**
   - Handles HTTP requests and responses
   - Routes requests to appropriate handlers
   - Manages request/response lifecycle

2. **Request Handlers**
   - Validate request parameters
   - Parse JSON payloads
   - Call appropriate service methods
   - Format responses

3. **Error Handling**
   - Consistent error response format
   - Detailed error messages
   - Appropriate HTTP status codes

### Embedding Layer Components

1. **FastEmbedding Service**
   - Converts text to vector embeddings
   - Manages embedding model lifecycle
   - Optimizes batch processing

2. **Vector Processing**
   - Normalizes vectors based on distance metric
   - Formats vectors for SPFresh
   - Handles vector validation

### FFI Layer Components

1. **Safe Rust Wrapper**
   - Provides idiomatic Rust interface
   - Handles error propagation
   - Ensures memory safety

2. **Memory Management**
   - Allocates and deallocates buffers
   - Manages ownership transfer
   - Prevents memory leaks

3. **FFI Function Declarations**
   - Defines C-compatible function signatures
   - Specifies data structures for cross-language communication
   - Handles type conversions

### C++ Interface Components

1. **C API Implementation**
   - Implements C-compatible functions
   - Converts between C and C++ types
   - Handles exceptions

### SPFresh Core Components

1. **SPFresh Update Layer**
   - Manages in-memory buffer for updates
   - Handles incremental rebalancing
   - Maintains index freshness

2. **SPANN Index Layer**
   - Manages centroid index in memory
   - Handles posting lists on disk
   - Performs approximate nearest neighbor search

3. **SPTAG Base Layer**
   - Implements vector indexing algorithms
   - Performs distance calculations
   - Manages core data structures

### Storage Layer Components

1. **Cloud Storage Integration**
   - Writes WAL entries to GCP Cloud Storage
   - Manages vector data persistence
   - Handles data replication

2. **Persistent Disk Management**
   - Optimizes posting list access
   - Manages SSD performance
   - Handles data layout

## Error Handling

The system will implement a comprehensive error handling strategy:

1. **Error Types**
   - `ApiError`: Errors at the API layer
   - `EmbeddingError`: Errors in text-to-vector conversion
   - `FfiError`: Errors crossing the FFI boundary
   - `SPFreshError`: Errors from the SPFresh core
   - `StorageError`: Errors in data persistence

2. **Error Propagation**
   - Errors are propagated up the stack with context
   - Low-level errors are wrapped in higher-level error types
   - Error context is preserved for debugging

3. **Error Responses**
   - Consistent JSON format for error responses
   - Appropriate HTTP status codes
   - Detailed error messages for debugging

## Testing Strategy

The system will be tested at multiple levels:

1. **Unit Tests**
   - Test individual components in isolation
   - Mock dependencies
   - Focus on edge cases

2. **Integration Tests**
   - Test interaction between components
   - Verify correct data flow
   - Test error handling

3. **End-to-End Tests**
   - Test complete workflows
   - Verify system behavior from client perspective
   - Test performance under load##
 Webhook Payload Examples

### Vector Insert Webhook

```
┌──────────────────────────────────────┐
│      Vector Insert Webhook           │
├──────────────────────────────────────┤
│ POST /external-system-endpoint       │
│ {                                    │
│   "event_type": "vector.inserted",   │
│   "timestamp": "2025-07-23T14:30:00Z",│
│   "namespace": "product-catalog",    │
│                                      │
│   // Operation Details               │
│   "operation_id": "op-123456",       │
│   "affected_ids": [1, 2, 3],         │
│   "vector_count": 3,                 │
│                                      │
│   // Performance Metrics             │
│   "processing_time_ms": 45,          │
│   "index_size": 10240,               │
│   "total_vectors": 1503              │
│ }                                    │
└──────────────────────────────────────┘
```

### Vector Search Webhook

```
┌──────────────────────────────────────┐
│      Vector Search Webhook           │
├──────────────────────────────────────┤
│ POST /external-system-endpoint       │
│ {                                    │
│   "event_type": "vector.search",     │
│   "timestamp": "2025-07-23T14:35:00Z",│
│   "namespace": "product-catalog",    │
│                                      │
│   // Query Information               │
│   "query_id": "q-789012",            │
│   "top_k": 10,                       │
│   "filter_applied": true,            │
│                                      │
│   // Performance Metrics             │
│   "latency_ms": 12,                  │
│   "result_count": 10,                │
│   "distance_range": [0.12, 0.45]     │
│ }                                    │
└──────────────────────────────────────┘
```

### System Rebalance Webhook

```
┌──────────────────────────────────────┐
│      System Rebalance Webhook        │
├──────────────────────────────────────┤
│ POST /external-system-endpoint       │
│ {                                    │
│   "event_type": "system.rebalanced", │
│   "timestamp": "2025-07-23T15:00:00Z",│
│   "namespace": "product-catalog",    │
│                                      │
│   // Rebalance Details               │
│   "rebalance_id": "rb-345678",       │
│   "trigger": "size_threshold",       │
│   "affected_clusters": 5,            │
│                                      │
│   // Performance Impact              │
│   "duration_ms": 3500,               │
│   "vectors_moved": 1250,             │
│   "index_size_change_kb": -320       │
│ }                                    │
└──────────────────────────────────────┘
```
## Bil
lion-Scale Architecture

To support 1 billion records, the SPFresh-Rust integration requires a distributed architecture with specialized components for handling massive scale. This section outlines the key architectural considerations for billion-scale vector search.

### Distributed Sharding Strategy

The system employs a distributed sharding approach to handle 1 billion vectors:

1. **Vector Partitioning**
   - Vectors are partitioned into 5 major shards of 200M vectors each
   - Each shard is further distributed across 20 nodes (10M vectors per node)
   - Consistent hashing ensures even distribution and minimal resharding during scaling

2. **Hierarchical Load Balancing**
   - Global load balancer routes requests based on vector IDs or namespace
   - Shard-level load balancers distribute queries within each 200M vector partition
   - Node-level load balancing optimizes resource utilization

3. **Cross-Shard Query Coordination**
   - Distributed query executor aggregates results across shards
   - Parallel query execution with configurable timeout parameters
   - Result merging with score normalization

### Storage Optimization for Billion-Scale

1. **Tiered Storage Architecture**
   - Hot tier: In-memory centroid index (5% of vectors, ~50M vectors)
   - Warm tier: NVMe SSD for frequently accessed vectors (15%, ~150M vectors)
   - Cold tier: Standard SSD for less frequently accessed vectors (30%, ~300M vectors)
   - Archive tier: HDD/object storage for rarely accessed vectors (50%, ~500M vectors)

2. **Vector Compression**
   - Product Quantization (PQ) reduces vector storage by 8-16x
   - Scalar quantization for additional 2-4x compression
   - Hybrid compression strategies based on access patterns

3. **Distributed WAL and Recovery**
   - Replicated Write-Ahead Log across regions
   - Point-in-time recovery capabilities
   - Incremental backup strategy

### Query Optimization for Billion-Scale

1. **Multi-Level Indexing**
   - Global coarse quantizer for initial routing
   - Shard-level HNSW/IVF indexes for efficient search
   - Node-level fine-grained indexes

2. **Adaptive Search Parameters**
   - Dynamic beam width based on query complexity
   - Automatic parameter tuning based on latency requirements
   - Early termination strategies for time-bounded queries

3. **Caching Strategy**
   - Query result cache with TTL
   - Hot vector cache with LRU eviction
   - Centroid cache with pinning for critical clusters

### Operational Considerations

1. **Monitoring and Observability**
   - Per-shard metrics collection
   - Distributed tracing for cross-shard queries
   - Anomaly detection for performance degradation

2. **Scaling Operations**
   - Automated horizontal scaling based on load
   - Zero-downtime resharding capabilities
   - Background reindexing for optimization

3. **Disaster Recovery**
   - Cross-region replication with async or sync options
   - Automated failover with configurable consistency levels
   - Regular disaster recovery testing

This billion-scale architecture enables SPFresh-Rust to efficiently handle 1 billion vectors while maintaining acceptable query performance and operational reliability.#
# High-Throughput Write Architecture

To achieve **10,000+ writes/second per namespace**, the system implements a specialized high-throughput write pipeline optimized for vector data. This architecture focuses on minimizing latency while maximizing throughput through efficient batching and asynchronous processing.

### Write Pipeline Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                       HIGH-THROUGHPUT WRITE PIPELINE                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────┐    ┌─────────────────────┐    ┌─────────────────┐  │
│  │  Request Reception  │    │  Batching System    │    │   WAL Creation  │  │
│  │  • Load balancer    │───▶│  • 100ms windows    │───▶│  • Direct write │  │
│  │  • Consistent hash  │    │  • Aggregation      │    │  • GCS/S3       │  │
│  └─────────────────────┘    └─────────────────────┘    └────────┬────────┘  │
│                                                                 │           │
│                                                                 ▼           │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                        Async Indexing                                 │  │
│  │                                                                       │  │
│  │  ┌─────────────────┐   ┌─────────────────┐   ┌─────────────────┐      │  │
│  │  │ SPFresh Vector  │   │ BM25 Full-text  │   │ Attribute       │      │  │
│  │  │ Index           │   │ Index           │   │ Indexes         │      │  │
│  │  └────────┬────────┘   └────────┬────────┘   └────────┬────────┘      │  │
│  │           │                     │                     │                │  │
│  │           └─────────────────────┼─────────────────────┘                │  │
│  │                                 │                                      │  │
│  └─────────────────────────────────┼──────────────────────────────────────┘  │
│                                    │                                         │
│                                    ▼                                         │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                        Cache Invalidation                             │  │
│  │                                                                       │  │
│  │  ┌─────────────────┐   ┌─────────────────┐   ┌─────────────────┐      │  │
│  │  │ L1 Query Cache  │   │ L2 Result Cache │   │ Distributed     │      │  │
│  │  │ Invalidation    │   │ Update          │   │ Cache Sync      │      │  │
│  │  └─────────────────┘   └─────────────────┘   └─────────────────┘      │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Key Components

1. **Request Reception**
   - Load balancer with consistent hashing routes requests to appropriate shards
   - Connection pooling and keep-alive connections minimize connection overhead
   - Request validation and preprocessing happens at edge nodes

2. **Batching System**
   - Implements 100ms time windows to aggregate incoming writes
   - Dynamic batch sizing based on current load and vector dimensions
   - Priority queuing for different operation types (upsert vs update vs delete)
   - Backpressure mechanisms to prevent system overload

3. **Write-Ahead Log (WAL) Creation**
   - Direct write to object storage (GCS/S3) for durability
   - Parallel WAL writers for high throughput
   - Checksumming and integrity verification
   - Optimized binary format with compression

4. **Asynchronous Indexing**
   - Parallel index updates across multiple index types:
     - SPFresh vector index for ANN search
     - BM25 full-text index for text search
     - Attribute indexes for filtering
   - Background rebalancing of vector clusters
   - Incremental index updates to minimize latency impact

5. **Cache Invalidation**
   - Multi-tier cache update strategy:
     - L1 query cache invalidation
     - L2 result cache updates
     - Distributed cache synchronization
   - Bloom filters to efficiently track affected queries
   - Partial cache updates for efficiency

### Performance Optimizations

1. **Memory Management**
   - Zero-copy data handling where possible
   - Pre-allocated buffers for vector data
   - SIMD-accelerated vector operations
   - Custom memory pools for vector data

2. **I/O Optimization**
   - Asynchronous I/O for all storage operations
   - Direct I/O for WAL writes
   - Vectored I/O for batch operations
   - I/O scheduling to prioritize critical paths

3. **Concurrency Control**
   - Lock-free data structures for high concurrency
   - Fine-grained locking where necessary
   - Optimistic concurrency control for metadata updates
   - Wait-free algorithms for read paths

4. **Monitoring and Adaptation**
   - Real-time throughput monitoring
   - Adaptive batch sizing based on system load
   - Dynamic resource allocation
   - Automatic throttling during overload conditions

This high-throughput write architecture enables the system to handle 10,000+ writes per second per namespace while maintaining data consistency and search accuracy. The design prioritizes write throughput while ensuring that search performance remains responsive even under heavy write loads.
##
 High-Throughput Architecture

To support 10,000+ writes per second with 1 billion records, the SPFresh-Rust integration requires an optimized architecture focused on write throughput and efficient storage. This section outlines the key architectural considerations.

### Optimized Write Path

1. **Write Optimization Techniques**
   - Batched writes (1,000-5,000 vectors per batch)
   - Asynchronous write acknowledgment
   - Write buffering with configurable flush intervals
   - Background indexing to minimize write latency

2. **Load Distribution**
   - Dedicated write nodes with optimized configuration
   - Read/write separation for predictable performance
   - Write throttling to prevent overload conditions

3. **Write Amplification Reduction**
   - Efficient WAL (Write-Ahead Log) design
   - Minimized metadata updates
   - Optimized RocksDB configuration for write-heavy workloads

### Storage Efficiency

1. **Tiered Storage Architecture**
   - Hot tier: In-memory buffer for recent writes (5-10M vectors)
   - Warm tier: SSD for frequently accessed vectors
   - Cold tier: Object storage for historical data

2. **Vector Compression**
   - Scalar quantization for 2-4x storage reduction
   - Selective compression based on access patterns
   - Metadata compression for additional space savings

3. **Efficient WAL Design**
   - Optimized WAL format for vector data
   - Periodic WAL compaction
   - Configurable durability guarantees

### Performance Optimization

1. **Indexing Strategy**
   - Incremental indexing for minimal write impact
   - Background index optimization
   - Prioritized indexing for frequently accessed vectors

2. **Resource Management**
   - Dedicated CPU cores for write operations
   - Memory management optimized for write throughput
   - I/O scheduling for consistent performance

3. **Caching Strategy**
   - Metadata cache for fast lookups
   - Write buffer cache for repeated writes
   - Adaptive cache sizing based on workload

### Operational Considerations

1. **Monitoring and Alerting**
   - Write throughput metrics
   - Latency percentile tracking
   - Queue depth monitoring
   - Storage utilization alerts

2. **Scaling Strategy**
   - Vertical scaling for write nodes
   - Horizontal scaling for read capacity
   - Automated capacity planning

3. **Reliability Features**
   - Write retry mechanisms
   - Graceful degradation under load
   - Automated recovery procedures

This architecture enables SPFresh-Rust to efficiently handle 10,000+ writes per second while maintaining the ability to store and query 1 billion vectors with acceptable performance.## Deta
iled Operation Flow Diagrams

### Vector Upsert Flow

```
Client Application                          SPFresh-Rust System
─────────────────                           ────────────────────────────────────────────────
┌─────────────┐                          ┌──────────────────────────────────────────────┐
│   Client    │      ##     │         PI LAYER          │
│   Request                      │                              
└──────┬────                  │  ┌────────────────────────┐   
       │                     │  │      Reest Processing       │
       ▼                     │                           │        │
┌───────────TTP Reques       │  │rse JSON                  │
│ JSOoad│─────────▶│  │  •  vectors & met │      
    psert)   ed O          peratirams    │  │  chema               │
└───┘                     └─────────────────────┘    
t                      or Upsert         │  Op             │
                              ──────────────────────────────
                                             
### me       rma               id                   
                  sequenceDiaa      rt   ┌──ici──────────────nt───────────────┐
                          │    EMBEDDING LAY            │
                   parti             cB as Loa              d Balancer              
                          pant           │  API───────── as API La(──────────┐       Axum
            )                 │      Input e Check              │
                                     │  │                       │
                     │  • Text or Vector      │        │
                            │───────────────────────┘      │
                    │                 │              │
                          │              ├─────           │
                        │                  │        │    
                                                ▼       ▼           │
                              │  ┌───────────────────┐   │
                         │  │   FastE  │ │ Vector I│
                          Rust Libra)   │ │         │
                          │  │ • Tokeion     │ │ • Use vided │   │
                             │ • Neural  │ │   vtors     
                        paEm             │  │ • Genebed as Embes │ │        r│
                           │  └────┬────────────┬───────┘   
    p                   artici        pant L    ayer                     │
                          │       ───────┬───┘           │
                                         │            
                                    partici                      C++ Interf             │ace participant SPFresh Core
    pAL               as Write           │  ┌──────-Aheg──────────────┐      │
                             │     ocessing             │
                        │  │                      │        
            par              t as        In-Memory Bmalize vectors  artiCloud St        │orage
                          │  │  • r SPFresh    │         
              Ccipat->>      LB: : {PO   │  └ST /"upsert_─────┬─rowv─────────┘         1/ntmes, vecpaces/{ Gdata}]}
                 LB-CS             t Route to               Writmiz           ed Node
                               └────────────┼───────────────
                                              │
                                            ▼
                 ┌──────────────────────────────┐
                                         FI LAYER         │
                                                      │
                               │  ┌────────────────────┐     │
                          │  │  y Managnt         │    
                                             │          │
                            │  │  • Allocat           │    
                                 │ Convert Rust → C types     │
                              ────────────────────────┘          │
                   │                  │                 
                    API-ali        date Re Aut         h     ▼              │
                      │  ┌────────────────────┐          │
                          │  │     I Function Cs        │   
                                                  │      
                                   all SPFresh_AddBatch│          │
                    │  │  •errors           │          │
                           │  └────┬───────────      │
                               │                │                │
    alt pr            mbedovide:         └───────── Cd ins────┼──────────────────tead o to vector
                f v    r  Embed->>       API: Ret            │uredding vector
                               ▼
                       ┌────────────────────────────
                                            C++ INCE            │
                       │                                  │
    A         PI-                   >>API│  ┌─────────────: Batchnt ─────────to C-compati
        bl                  e gs │  │      C Atation │        │
                          │  │                     │          │
                         │  • Convert C → C+ypes         
                             │• Exception        │      
                                ─────────────┬────────────┘ 
            FFI-                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              SPFRESH CORE                    │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      SPFresh Update Layer      │          │
                                         │  │                                │          │
                                         │  │  • Add to in-memory buffer     │          │
                                         │  │  • Write to WAL                │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         │                    ▼                         │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      SPANN Index Layer         │          │
                                         │  │                                │          │
                                         │  │  • Update centroid index       │          │
                                         │  │  • Schedule rebalancing        │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              STORAGE LAYER (GCP)             │
                                         │                                              │
                                         │  ┌────────────────┐   ┌─────────────────┐    │
                                         │  │ Cloud Storage  │   │ Persistent Disk │    │
                                         │  │ • WAL entries  │   │ • Vector data   │    │
                                         │  │ • Durability   │   │ • Metadata      │    │
                                         │  └────────┬───────┘   └────────┬────────┘    │
                                         │           │                    │             │
                                         │           └────────┬───────────┘             │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              RESPONSE HANDLING               │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Result Processing         │          │
                                         │  │                                │          │
                                         │  │  • Format JSON response        │          │
                                         │  │  • Add operation status        │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
┌─────────────┐                                              │
│   Client    │                                              │
│  Response   │◀─────────────────────────────────────────────┘
└─────────────┘
```

### Vector Update Flow

```
Client Application                          SPFresh-Rust System
─────────────────                           ────────────────────────────────────────────────
┌─────────────┐                          ┌──────────────────────────────────────────────┐
│   Client    │                          │                  API LAYER                   │
│   Request   │                          │                                              │
└──────┬──────┘                          │  ┌────────────────────────────────┐          │
       │                                 │  │      Request Processing        │          │
       ▼                                 │  │                                │          │
┌─────────────┐     HTTP Request         │  │  • Parse JSON                  │          │
│ JSON Payload│  ───────────────────────▶│  │  • Extract IDs & fields       │          │
│ (update)    │                          │  │  • Validate schema             │          │
└─────────────┘                          │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │                 FFI LAYER                    │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Memory Management         │          │
                                         │  │                                │          │
                                         │  │  • Allocate buffers            │          │
                                         │  │  • Convert Rust → C types      │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         │                    ▼                         │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      FFI Function Calls        │          │
                                         │  │                                │          │
                                         │  │  • Call SPFresh_Update         │          │
                                         │  │  • Handle errors               │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              C++ INTERFACE                   │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      C API Implementation      │          │
                                         │  │                                │          │
                                         │  │  • Convert C → C++ types       │          │
                                         │  │  • Exception handling          │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              SPFRESH CORE                    │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Document Lookup           │          │
                                         │  │                                │          │
                                         │  │  • Find documents by ID        │          │
                                         │  │  • Verify existence            │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         │                    ▼                         │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Metadata Update           │          │
                                         │  │                                │          │
                                         │  │  • Update metadata fields      │          │
                                         │  │  • Write to WAL                │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              STORAGE LAYER (GCP)             │
                                         │                                              │
                                         │  ┌────────────────┐   ┌─────────────────┐    │
                                         │  │ Cloud Storage  │   │ Persistent Disk │    │
                                         │  │ • WAL entries  │   │ • Metadata      │    │
                                         │  │ • Durability   │   │ • Updates       │    │
                                         │  └────────┬───────┘   └────────┬────────┘    │
                                         │           │                    │             │
                                         │           └────────┬───────────┘             │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              RESPONSE HANDLING               │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Result Processing         │          │
                                         │  │                                │          │
                                         │  │  • Format JSON response        │          │
                                         │  │  • Add operation status        │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
┌─────────────┐                                              │
│   Client    │                                              │
│  Response   │◀─────────────────────────────────────────────┘
└─────────────┘
```

### Conditional Operations Flow (Upsert/Update/Delete)

```
Client Application                          SPFresh-Rust System
─────────────────                           ────────────────────────────────────────────────
┌─────────────┐                          ┌──────────────────────────────────────────────┐
│   Client    │                          │                  API LAYER                   │
│   Request   │                          │                                              │
└──────┬──────┘                          │  ┌────────────────────────────────┐          │
       │                                 │  │      Request Processing        │          │
       ▼                                 │  │                                │          │
┌─────────────┐     HTTP Request         │  │  • Parse JSON                  │          │
│ JSON Payload│  ───────────────────────▶│  │  • Extract data & conditions   │          │
│ (conditional)│                         │  │  • Validate schema             │          │
└─────────────┘                          │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              CONDITION EVALUATION            │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Document Lookup           │          │
                                         │  │                                │          │
                                         │  │  • Find existing documents     │          │
                                         │  │  • Retrieve current state      │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         │                    ▼                         │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Condition Evaluation      │          │
                                         │  │                                │          │
                                         │  │  • Apply condition logic       │          │
                                         │  │  • Determine if operation      │          │
                                         │  │    should proceed              │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │                 FFI LAYER                    │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Memory Management         │          │
                                         │  │                                │          │
                                         │  │  • Allocate buffers            │          │
                                         │  │  • Convert Rust → C types      │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         │                    ▼                         │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      FFI Function Calls        │          │
                                         │  │                                │          │
                                         │  │  • Call appropriate function   │          │
                                         │  │    based on operation type     │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              C++ INTERFACE                   │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      C API Implementation      │          │
                                         │  │                                │          │
                                         │  │  • Convert C → C++ types       │          │
                                         │  │  • Exception handling          │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              SPFRESH CORE                    │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Operation Execution       │          │
                                         │  │                                │          │
                                         │  │  • Execute upsert/update/delete│          │
                                         │  │  • Write to WAL                │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              STORAGE LAYER (GCP)             │
                                         │                                              │
                                         │  ┌────────────────┐   ┌─────────────────┐    │
                                         │  │ Cloud Storage  │   │ Persistent Disk │    │
                                         │  │ • WAL entries  │   │ • Data updates  │    │
                                         │  │ • Durability   │   │ • Metadata      │    │
                                         │  └────────┬───────┘   └────────┬────────┘    │
                                         │           │                    │             │
                                         │           └────────┬───────────┘             │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              RESPONSE HANDLING               │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Result Processing         │          │
                                         │  │                                │          │
                                         │  │  • Format JSON response        │          │
                                         │  │  • Add operation status        │          │
                                         │  │  • Include condition result    │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
┌─────────────┐                                              │
│   Client    │                                              │
│  Response   │◀─────────────────────────────────────────────┘
└─────────────┘
```

### Batch Delete Flow

```
Client Application                          SPFresh-Rust System
─────────────────                           ────────────────────────────────────────────────
┌─────────────┐                          ┌──────────────────────────────────────────────┐
│   Client    │                          │                  API LAYER                   │
│   Request   │                          │                                              │
└──────┬──────┘                          │  ┌────────────────────────────────┐          │
       │                                 │  │      Request Processing        │          │
       ▼                                 │  │                                │          │
┌─────────────┐     HTTP Request         │  │  • Parse JSON                  │          │
│ JSON Payload│  ───────────────────────▶│  │  • Extract document IDs        │          │
│ (batch-delete)│                        │  │  • Validate request            │          │
└─────────────┘                          │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │                 FFI LAYER                    │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Memory Management         │          │
                                         │  │                                │          │
                                         │  │  • Allocate ID buffers         │          │
                                         │  │  • Convert Rust → C types      │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         │                    ▼                         │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      FFI Function Calls        │          │
                                         │  │                                │          │
                                         │  │  • Call SPFresh_Delete         │          │
                                         │  │  • Handle errors               │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              C++ INTERFACE                   │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      C API Implementation      │          │
                                         │  │                                │          │
                                         │  │  • Convert C → C++ types       │          │
                                         │  │  • Exception handling          │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              SPFRESH CORE                    │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Deletion Processing       │          │
                                         │  │                                │          │
                                         │  │  • Mark documents as deleted   │          │
                                         │  │  • Add delete markers to WAL   │          │
                                         │  │  • Schedule garbage collection │          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              STORAGE LAYER (GCP)             │
                                         │                                              │
                                         │  ┌────────────────┐   ┌─────────────────┐    │
                                         │  │ Cloud Storage  │   │ Persistent Disk │    │
                                         │  │ • Delete markers│   │ • Index updates │    │
                                         │  │ • WAL entries  │   │ • Metadata      │    │
                                         │  └────────┬───────┘   └────────┬────────┘    │
                                         │           │                    │             │
                                         │           └────────┬───────────┘             │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
                                                             ▼
                                         ┌──────────────────────────────────────────────┐
                                         │              RESPONSE HANDLING               │
                                         │                                              │
                                         │  ┌────────────────────────────────┐          │
                                         │  │      Result Processing         │          │
                                         │  │                                │          │
                                         │  │  • Format JSON response        │          │
                                         │  │  • Add deletion counts         │          │
                                         │  │  • Include success/failure info│          │
                                         │  └─────────────────┬──────────────┘          │
                                         │                    │                         │
                                         └────────────────────┼─────────────────────────┘
                                                             │
┌─────────────┐                                              │
│   Client    │                                              │
│  Response   │◀─────────────────────────────────────────────┘
└─────────────┘
```>>C++: _AddBatch(vectors, etadata)
    rite operationWAL
    C++->>Mem: rs to in-memory buffer
SPF: Uory inde
    
   nc Storage Operat
        S  ->>GCS: Schedule background PL entries
    end
    
 FFI: Returnperation status
    >>API: Convert rt to Rust types
 ->>Clientucceailure response
    
    Note over SPF: BAPIground Process
    ->>->>SPF: Rebal Periindex if needodic flisk
    SPF->>S
        APow

`rmaid
sequeam
    part anpant LB as Load Bt ClieAxum)
    particir Rust FFI Layer
    pticipant C++ as C++erresh Coface
    participant WAL    icipa-Ahead Loge

    Client->>LTCHpatch_rows" /v1/nam/{: [{id, mns}/updatedates}]
    LB->>API:   over o Write-OptimizCl Node
    I->>API: Validart tot C-compatie e types
    
    C++->>SPF  FFI->>Cescuments by: SPFresh_Update(idst & Auth
 i  
        C++->>WA  alt ent,I-ate operatnts foundL
        C++-> Update mets
        r Async Storerations
       
        
        C++->>FFI: Retu : Persiss status
    elsestocuments not foundies
        C+     turn not f err
   
>>FF result to Rues
    API->: Return success/fnse
```

    3. Conditional  FFI->>Operation FloAPI: ConI: ConL
    particnt SPF CS as GCP Cas 
    prmaidartic API as API Layer
### 2nceDiagram
   articipant LB lancer
    participant Embed  s Embeddin ALayer
    participaPI FFI as Rus as A Layer
    participanPI Layer C++ Interface (Axum
  . participant SPF asticipanh Core
  t V    cipant WALNote otee-Ahead Log
   (Patch) Opnt Mem as In-Memoud Storage

er  Note over Client,LB: {Clieatiffdition}
    LerPOST /vRoute to Wri1/n{nsimize}/conditional-ups
    
    Apartver antalidate Request & Au GCS Cli 
    alt ed inst     API->>Embedxt to vectorEmbed->>API: Return e vector
    
    
    Aonvert to Compatible types
C++: SPFresh_CtionalUvectors,ta, co  
    C++->eck if documexist
    C++-Evaluate condn againsg data
      C++->>S Condition met        C++->>WAperation to      C++->>Mem: s in memory buffer
    PF: Update -memory index
     L->>GCS  par Async Serations
     : Persistentries
                urn succe
      C++->>FFI: Rss statuelse Condition not
        C++->>turn conditioailed status
      
    FFIert result cess/failtos
    API->>Client: ure resp``

### Delete Operati```mermaid
agram
    parent
    paicipant LB as lancer
    pticipant API as API (Axum)
    ticipant FFFFI Layer
    C+SPF as SPF+ as  Interface
    paresh Cor  participant WAite-Ahead Log    participas GCP Cloud St

   ->>LB: DELETE /v1/npaces/ch-delete
    Ne overe t Cldeletes": [id1, i}
    LB->>APo Writeptimized Node
    API->>API: Validat & Auth
    Conver Mark dt to C-comle typeFFI->>C++: _Delete(i   
    AL: Add deleC++->>ocuments eleted in inx
    te WAL
    
    par Asy Operations
      WAL->>GCSPersist delete 
        S Schedule garbage ion
    end
 C++->>FFI: Retperation sts
  a
    FPF:FI->>APIrt result to   API->>Clit: Return suc/failure w    
    N Brocess
    SPF->>Siodic garbage collection
``
d 
### 5. Filter-BDeletperation Flow

```rmaid
sequeagram
    participanient
    parpant LB as Loadalancer
    paas API L
    participant FFI  Rust FFI Layer
    cipant C++ as C++ Inter
    participant Fresh Coremized Node
    pcipant We-Ahead  participant GCS as G Storageent->>LB: DEL /v1/nameses/{ns}/filt   Note over  {"delete_by_fililter_conditions   LB->>APIoute to 
    
  API->>API: Validat & Auth
    APFI: Convert to ible types
    FSPFresh_QueryAndDelet   
    C++->>SPute query find maents
    C++->>S IDs of matching do   
    l matchi
```

## Write Operation Flow Diagrams
# Write Operation Flow Diagrams

### Vector Upsert Flow

```
+----------+     +--------------+     +------------+     +------------+     +------------+     +------------+     +------------+
|  Client  |     |     Load     |     |    API     |     | Embedding  |     |    FFI     |     |    C++     |     |  SPFresh   |
|          |     |   Balancer   |     |   Layer    |     |   Layer    |     |   Layer    |     | Interface  |     |   Core     |
+----+-----+     +------+-------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+
     |                 |                   |                  |                  |                  |                  |
     | POST /upsert    |                   |                  |                  |                  |                  |
     +---------------->|                   |                  |                  |                  |                  |
     |                 |                   |                  |                  |                  |                  |
     |                 | Route request     |                  |                  |                  |                  |
     |                 +------------------>|                  |                  |                  |                  |
     |                 |                   |                  |                  |                  |                  |
     |                 |                   | Validate request |                  |                  |                  |
     |                 |                   +----------------->|                  |                  |                  |
     |                 |                   |                  |                  |                  |                  |
     |                 |                   | Convert text     |                  |                  |                  |
     |                 |                   +----------------->|                  |                  |                  |
     |                 |                   |                  |                  |                  |                  |
     |                 |                   |<-----------------+                  |                  |                  |
     |                 |                   | Return vectors   |                  |                  |                  |
     |                 |                   |                  |                  |                  |                  |
     |                 |                   | Convert to C     |                  |                  |                  |
     |                 |                   +---------------------------------->|                  |                  |
     |                 |                   |                  |                  |                  |                  |
     |                 |                   |                  |                  | SPFresh_AddBatch |                  |
     |                 |                   |                  |                  +----------------->|                  |
     |                 |                   |                  |                  |                  |                  |
     |                 |                   |                  |                  |                  | Write to WAL     |
     |                 |                   |                  |                  |                  +----------------->|
     |                 |                   |                  |                  |                  |                  |
     |                 |                   |                  |                  |                  |                  | Update index
     |                 |                   |                  |                  |                  |                  +----------->
     |                 |                   |                  |                  |                  |                  |
     |                 |                   |                  |                  |                  |<-----------------+
     |                 |                   |                  |                  |<-----------------+                  |
     |                 |                   |<------------------------------------|                  |                  |
     |<----------------+-------------------+                  |                  |                  |                  |
     |                 |                   |                  |                  |                  |                  |
+----+-----+     +------+-------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+
|  Client  |     |     Load     |     |    API     |     | Embedding  |     |    FFI     |     |    C++     |     |  SPFresh   |
|          |     |   Balancer   |     |   Layer    |     |   Layer    |     |   Layer    |     | Interface  |     |   Core     |
+----------+     +--------------+     +------------+     +------------+     +------------+     +------------+     +------------+
```

**Vector Upsert Flow Steps:**

1. Client sends POST /v1/namespaces/{namespace}/upsert with vector data
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. If text is provided instead of vector, embedding service converts it
5. API layer batches vectors and converts to C-compatible types
6. FFI layer calls SPFresh_AddBatch() with vectors, IDs, and metadata
7. C++ interface writes operation to WAL
8. Vectors are added to in-memory buffer
9. In-memory index is updated
10. Asynchronously, WAL entries are persisted to GCP Storage
11. Background rebalancing is scheduled if needed
12. Operation status is returned to client

### Vector Update Flow

```
+----------+     +--------------+     +------------+     +------------+     +------------+     +------------+
|  Client  |     |     Load     |     |    API     |     |    FFI     |     |    C++     |     |  SPFresh   |
|          |     |   Balancer   |     |   Layer    |     |   Layer    |     | Interface  |     |   Core     |
+----+-----+     +------+-------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+
     |                 |                   |                  |                  |                  |
     | PATCH /update   |                   |                  |                  |                  |
     +---------------->|                   |                  |                  |                  |
     |                 |                   |                  |                  |                  |
     |                 | Route request     |                  |                  |                  |
     |                 +------------------>|                  |                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   | Validate request |                  |                  |
     |                 |                   +----------------->|                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   | Convert to C     |                  |                  |
     |                 |                   +----------------->|                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   |                  | SPFresh_Update   |                  |
     |                 |                   |                  +----------------->|                  |
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  | Locate documents |
     |                 |                   |                  |                  +----------------->|
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  |                  | Update metadata
     |                 |                   |                  |                  |                  +----------->
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  |<-----------------+
     |                 |                   |                  |<-----------------+                  |
     |                 |                   |<-----------------+                  |                  |
     |<----------------+-------------------+                  |                  |                  |
     |                 |                   |                  |                  |                  |
+----+-----+     +------+-------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+
|  Client  |     |     Load     |     |    API     |     |    FFI     |     |    C++     |     |  SPFresh   |
|          |     |   Balancer   |     |   Layer    |     |   Layer    |     | Interface  |     |   Core     |
+----------+     +--------------+     +------------+     +------------+     +------------+     +------------+
```

**Vector Update Flow Steps:**

1. Client sends PATCH /v1/namespaces/{namespace}/update with metadata updates
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. API layer converts to C-compatible types
5. FFI layer calls SPFresh_Update() with IDs and fields to update
6. C++ interface locates documents by ID
7. If documents are found:
   a. Update operation is written to WAL
   b. Metadata fields are updated
   c. WAL entries are persisted to GCP Storage
   d. Success status is returned
8. If documents are not found:
   a. Not found error is returned
9. Result is converted to Rust types and returned to client

### Conditional Upsert Flow

```
+----------+     +--------------+     +------------+     +------------+     +------------+     +------------+
|  Client  |     |     Load     |     |    API     |     |    FFI     |     |    C++     |     |  SPFresh   |
|          |     |   Balancer   |     |   Layer    |     |   Layer    |     | Interface  |     |   Core     |
+----+-----+     +------+-------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+
     |                 |                   |                  |                  |                  |
     | POST /conditional-upsert            |                  |                  |                  |
     +---------------->|                   |                  |                  |                  |
     |                 |                   |                  |                  |                  |
     |                 | Route request     |                  |                  |                  |
     |                 +------------------>|                  |                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   | Validate request |                  |                  |
     |                 |                   +----------------->|                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   | Convert to C     |                  |                  |
     |                 |                   +----------------->|                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   |                  | SPFresh_ConditionalUpsert          |
     |                 |                   |                  +----------------->|                  |
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  | Check documents  |
     |                 |                   |                  |                  +----------------->|
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  |                  | Evaluate condition
     |                 |                   |                  |                  |                  +----------->
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  |                  | If condition met:
     |                 |                   |                  |                  |                  | Update index
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  |<-----------------+
     |                 |                   |                  |<-----------------+                  |
     |                 |                   |<-----------------+                  |                  |
     |<----------------+-------------------+                  |                  |                  |
     |                 |                   |                  |                  |                  |
+----+-----+     +------+-------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+
|  Client  |     |     Load     |     |    API     |     |    FFI     |     |    C++     |     |  SPFresh   |
|          |     |   Balancer   |     |   Layer    |     |   Layer    |     | Interface  |     |   Core     |
+----------+     +--------------+     +------------+     +------------+     +------------+     +------------+
```

**Conditional Upsert Flow Steps:**

1. Client sends POST /v1/namespaces/{namespace}/conditional-upsert with data and condition
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. If text is provided instead of vector, embedding service converts it
5. API layer converts to C-compatible types
6. FFI layer calls SPFresh_ConditionalUpsert() with vectors, IDs, metadata, and condition
7. SPFresh Core checks if documents exist
8. SPFresh Core evaluates condition against existing data
9. If condition is met:
   a. Operation is written to WAL
   b. Vectors are added/updated in memory buffer
   c. In-memory index is updated
   d. WAL entries are persisted to GCP Storage
   e. Success status is returned
10. If condition is not met:
    a. Condition failed status is returned
11. Result is converted to Rust types and returned to client

### Batch Delete Flow

```
+----------+     +--------------+     +------------+     +------------+     +------------+     +------------+
|  Client  |     |     Load     |     |    API     |     |    FFI     |     |    C++     |     |  SPFresh   |
|          |     |   Balancer   |     |   Layer    |     |   Layer    |     | Interface  |     |   Core     |
+----+-----+     +------+-------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+
     |                 |                   |                  |                  |                  |
     | DELETE /batch-delete               |                  |                  |                  |
     +---------------->|                   |                  |                  |                  |
     |                 |                   |                  |                  |                  |
     |                 | Route request     |                  |                  |                  |
     |                 +------------------>|                  |                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   | Validate request |                  |                  |
     |                 |                   +----------------->|                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   | Convert to C     |                  |                  |
     |                 |                   +----------------->|                  |                  |
     |                 |                   |                  |                  |                  |
     |                 |                   |                  | SPFresh_Delete   |                  |
     |                 |                   |                  +----------------->|                  |
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  | Mark as deleted  |
     |                 |                   |                  |                  +----------------->|
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  |                  | Add delete markers
     |                 |                   |                  |                  |                  +----------->
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  |                  | Schedule GC
     |                 |                   |                  |                  |                  |
     |                 |                   |                  |                  |<-----------------+
     |                 |                   |                  |<-----------------+                  |
     |                 |                   |<-----------------+                  |                  |
     |<----------------+-------------------+                  |                  |                  |
     |                 |                   |                  |                  |                  |
+----+-----+     +------+-------+     +-----+------+     +-----+------+     +-----+------+     +-----+------+
|  Client  |     |     Load     |     |    API     |     |    FFI     |     |    C++     |     |  SPFresh   |
|          |     |   Balancer   |     |   Layer    |     |   Layer    |     | Interface  |     |   Core     |
+----------+     +--------------+     +------------+     +------------+     +------------+     +------------+
```

**Batch Delete Flow Steps:**

1. Client sends DELETE /v1/namespaces/{namespace}/batch-delete with IDs to delete
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. API layer converts to C-compatible types
5. FFI layer calls SPFresh_Delete() with IDs
6. SPFresh Core marks documents as deleted in index
7. Delete markers are added to WAL
8. Asynchronously:
   a. Delete markers are persisted to GCP Storage
   b. Garbage collection is scheduled
9. Operation status is returned to client
10. Background process periodically performs garbage collection

### Filter-Based Delete Flow

```
Client → Load Balancer → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage

1. Client sends DELETE /v1/namespaces/{namespace}/filter-delete with filter conditions
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. API layer converts to C-compatible types
5. FFI layer calls SPFresh_QueryAndDelete() with filter
6. SPFresh Core executes query to find matching documents
7. SPFresh Core collects IDs of matching documents
8. For each matching document:
   a. Document is marked as deleted
   b. Delete marker is added to WAL
9. Asynchronously:
   a. Delete markers are persisted to GCP Storage
   b. Garbage collection is scheduled
10. Operation status with count is returned to client
```## Op
eration Flow Diagrams (Simplified)

### Vector Upsert Flow

**Flow Path:** Client → Load Balancer → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage

**Steps:**
1. Client sends POST /v1/namespaces/{namespace}/upsert with vector data
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. If text is provided instead of vector, embedding service converts it
5. API layer batches vectors and converts to C-compatible types
6. FFI layer calls SPFresh_AddBatch() with vectors, IDs, and metadata
7. C++ interface writes operation to WAL
8. Vectors are added to in-memory buffer
9. In-memory index is updated
10. Asynchronously, WAL entries are persisted to GCP Storage
11. Background rebalancing is scheduled if needed
12. Operation status is returned to client

### Vector Update Flow

**Flow Path:** Client → Load Balancer → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage

**Steps:**
1. Client sends PATCH /v1/namespaces/{namespace}/update with metadata updates
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. API layer converts to C-compatible types
5. FFI layer calls SPFresh_Update() with IDs and fields to update
6. C++ interface locates documents by ID
7. If documents are found:
   - Update operation is written to WAL
   - Metadata fields are updated
   - WAL entries are persisted to GCP Storage
   - Success status is returned
8. If documents are not found:
   - Not found error is returned
9. Result is converted to Rust types and returned to client

### Conditional Upsert Flow

**Flow Path:** Client → Load Balancer → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage

**Steps:**
1. Client sends POST /v1/namespaces/{namespace}/conditional-upsert with data and condition
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. If text is provided instead of vector, embedding service converts it
5. API layer converts to C-compatible types
6. FFI layer calls SPFresh_ConditionalUpsert() with vectors, IDs, metadata, and condition
7. SPFresh Core checks if documents exist
8. SPFresh Core evaluates condition against existing data
9. If condition is met:
   - Operation is written to WAL
   - Vectors are added/updated in memory buffer
   - In-memory index is updated
   - WAL entries are persisted to GCP Storage
   - Success status is returned
10. If condition is not met:
    - Condition failed status is returned
11. Result is converted to Rust types and returned to client

### Batch Delete Flow

**Flow Path:** Client → Load Balancer → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage

**Steps:**
1. Client sends DELETE /v1/namespaces/{namespace}/batch-delete with IDs to delete
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. API layer converts to C-compatible types
5. FFI layer calls SPFresh_Delete() with IDs
6. SPFresh Core marks documents as deleted in index
7. Delete markers are added to WAL
8. Asynchronously:
   - Delete markers are persisted to GCP Storage
   - Garbage collection is scheduled
9. Operation status is returned to client
10. Background process periodically performs garbage collection

### Filter-Based Delete Flow

**Flow Path:** Client → Load Balancer → API Layer → FFI Layer → C++ Interface → SPFresh Core → Storage

**Steps:**
1. Client sends DELETE /v1/namespaces/{namespace}/filter-delete with filter conditions
2. Load balancer routes to write-optimized node
3. API layer validates request and authentication
4. API layer converts to C-compatible types
5. FFI layer calls SPFresh_QueryAndDelete() with filter
6. SPFresh Core executes query to find matching documents
7. SPFresh Core collects IDs of matching documents
8. For each matching document:
   - Document is marked as deleted
   - Delete marker is added to WAL
9. Asynchronously:
   - Delete markers are persisted to GCP Storage
   - Garbage collection is scheduled
10. Operation status with count is returned to client
##
 Simplified Operation Flow Diagrams

### Vector Upsert Flow (Simplified)

```
Client -> Load Balancer -> API Layer -> Embedding Layer -> FFI Layer -> C++ Interface -> SPFresh Core -> Storage
```

1. Client sends POST request with vector data
2. Load balancer routes to write node
3. API layer validates and processes request
4. Text converted to vectors if needed
5. Data converted to C-compatible format
6. FFI layer calls SPFresh_AddBatch()
7. Data written to WAL and memory buffer
8. In-memory index updated
9. WAL entries persisted to storage
10. Response returned to client

### Vector Update Flow (Simplified)

```
Client -> Load Balancer -> API Layer -> FFI Layer -> C++ Interface -> SPFresh Core -> Storage
```

1. Client sends PATCH request with updates
2. Load balancer routes to write node
3. API layer validates request
4. Data converted to C-compatible format
5. FFI layer calls SPFresh_Update()
6. Documents located by ID
7. If found, metadata updated and written to WAL
8. WAL entries persisted to storage
9. Response returned to client

### Conditional Operations Flow (Simplified)

```
Client -> Load Balancer -> API Layer -> FFI Layer -> C++ Interface -> SPFresh Core -> Storage
```

1. Client sends conditional request
2. Load balancer routes to write node
3. API layer validates request
4. Data converted to C-compatible format
5. FFI layer calls appropriate conditional function
6. Documents located and condition evaluated
7. If condition met, operation performed
8. Changes written to WAL and memory
9. WAL entries persisted to storage
10. Response returned to client

### Batch Delete Flow (Simplified)

```
Client -> Load Balancer -> API Layer -> FFI Layer -> C++ Interface -> SPFresh Core -> Storage
```

1. Client sends DELETE request with IDs
2. Load balancer routes to write node
3. API layer validates request
4. Data converted to C-compatible format
5. FFI layer calls SPFresh_Delete()
6. Documents marked as deleted
7. Delete markers added to WAL
8. WAL entries persisted to storage
9. Garbage collection scheduled
10. Response returned to client