# Design Document

## Overview

This document outlines the comprehensive design for modernizing the SPFresh vector search system architecture. The modernization transforms SPFresh into a production-ready, enterprise-grade vector search platform with an 8-layer architecture that combines high-performance C++ core components with modern Rust API layers, multi-cloud storage abstraction, and advanced embedding capabilities.

The design leverages the existing SPFresh/SPANN vector search engine (Microsoft SPTAG-based) while adding modern infrastructure layers for scalability, reliability, and maintainability. The system will support high-throughput operations (10K+ QPS), multi-billion record datasets, and enterprise features like encryption, multi-tenancy, and comprehensive monitoring.

## Architecture Diagrams

### 1. Complete System Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         SPFresh Modernized Architecture                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      1. CLIENT LAYER                                │    │
│  │                                                                     │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐ │    │
│  │  │ Web Apps │  │ Mobile   │  │ Desktop  │  │Analytics │  │  Batch   │ │    │
│  │  │Frontend  │  │   Apps   │  │   Apps   │  │ Clients  │  │Processing│ │    │
│  │  │   🌐    │  │    📱    │  │    💻    │  │    📊    │  │    ⚙️    │ │    │
│  │  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘ │    │
│  │       │             │             │             │             │       │    │
│  │       │HTTP/JSON    │HTTP/JSON    │HTTP/JSON    │WebSocket    │gRPC   │    │
│  │       │             │             │             │             │       │    │
│  └───────┼─────────────┼─────────────┼─────────────┼─────────────┼───────┘    │
│          │             │             │             │             │            │
│          └─────────────┼─────────────┼─────────────┼─────────────┘            │
│                        │             │             │                          │
│                        └─────────────┼─────────────┘                          │
│                                      │                                        │
│                                      ▼                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                   2. LOAD BALANCER LAYER (GCP)                      │    │
│  │                                                                     │    │
│  │  ┌─────────────────────┐    ┌─────────────────────┐                  │    │
│  │  │   Health Checks     │    │   Traffic Routing   │                  │    │
│  │  │ • Service health    │    │ • Round robin       │                  │    │
│  │  │ • Response time     │    │ • Weighted routing  │                  │    │
│  │  │ • Error rates       │    │ • Geo-based routing │                  │    │
│  │  └─────────────────────┘    └─────────────────────┘                  │    │
│  │                                                                     │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                Advanced Features                             │    │    │
│  │  │ • SSL termination    • Rate limiting    • Auto-scaling      │    │    │
│  │  │ • DDoS protection   • Request routing   • Circuit breaker   │    │    │
│  │  │ • Session affinity  • Failover handling • Regional routing  │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│            ┌───────────────┼───────────────┐                               │
│            │               │               │                               │
│            ▼               ▼               ▼                               │
│    ┌─────────────┐ ┌─────────────┐ ┌─────────────┐                         │
│    │  Write      │ │  Read       │ │  Batch      │                         │
│    │  Optimized  │ │  Optimized  │ │  Processing │                         │
│    │  Instances  │ │  Instances  │ │  Instances  │                         │
│    └─────────────┘ └─────────────┘ └─────────────┘                         │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    🔥 SPFRESH HOTCACHE LAYER (NEW)                  │    │
│  │                                                                     │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                   L1 Cache (Memory) - Ultra Hot             │    │    │
│  │  │                                                             │    │    │
│  │  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────┐ │    │    │
│  │  │  │Query Results│ │Vector Cache │ │Index Segments│ │BM25 Terms│ │    │    │
│  │  │  │(1GB LRU)    │ │(2GB HashMap)│ │(4GB Segments)│ │(512MB)  │ │    │    │
│  │  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └────┬────┘ │    │    │
│  │  │         │ Hit           │ Hit           │ Hit          │ Hit   │    │    │
│  │  │         ▼               ▼               ▼              ▼       │    │    │
│  │  │  ┌─────────────────────────────────────────────────────────┐   │    │    │
│  │  │  │          Cache Hit Response (0.1ms)                     │   │    │    │
│  │  │  │  • Return cached data immediately                       │   │    │    │
│  │  │  │  • Update access metrics                                │   │    │    │
│  │  │  │  • Record cache hit for analytics                       │   │    │    │
│  │  │  └─────────────────────────────────────────────────────────┘   │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  │                                                                     │    │
│  │  Cache Miss ▼                                                       │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                   L2 Cache (SSD) - Warm                     │    │    │
│  │  │                                                             │    │    │
│  │  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────┐ │    │    │
│  │  │  │Posting Lists│ │Hot Metadata │ │Freq Vectors │ │Popular  │ │    │    │
│  │  │  │(Recent)     │ │(Popular)    │ │(Accessed)   │ │Terms    │ │    │    │
│  │  │  │100GB NVMe   │ │50GB NVMe    │ │200GB NVMe   │ │50GB NVMe│ │    │    │
│  │  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └────┬────┘ │    │    │
│  │  │         │ Hit           │ Hit           │ Hit          │ Hit   │    │    │
│  │  │         ▼               ▼               ▼              ▼       │    │    │
│  │  │  ┌─────────────────────────────────────────────────────────┐   │    │    │
│  │  │  │      Promote to L1 + Return Data (2.5ms)               │   │    │    │
│  │  │  │  • Load data from SSD                                   │   │    │    │
│  │  │  │  • Promote frequently accessed items to L1              │   │    │    │
│  │  │  │  • Update access patterns                               │   │    │    │
│  │  │  └─────────────────────────────────────────────────────────┘   │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  │                                                                     │    │
│  │  Cache Miss ▼ (Continue to your existing architecture)              │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    3. API LAYER (Rust/Axum)                         │    │
│  │                                                                     │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                  Axum Web Server 🚀                         │    │    │
│  │  ├─────────────────────────────────────────────────────────────┤    │    │
│  │  │ SEARCH & QUERY APIs (8 เส้นหลัก):                            │    │    │
│  │  │ • POST /v1/namespaces/{ns}/search/vector                    │    │    │
│  │  │ • POST /v1/namespaces/{ns}/search/text                      │    │    │
│  │  │ • POST /v1/namespaces/{ns}/search/hybrid                    │    │    │
│  │  │ • POST /v1/namespaces/{ns}/search/bm25                      │    │    │
│  │  │ • POST /v1/namespaces/{ns}/search/multi-vector              │    │    │
│  │  │ • POST /v1/namespaces/{ns}/search/filtered                  │    │    │
│  │  │ • POST /v1/namespaces/{ns}/search/batch                     │    │    │
│  │  │ • GET  /v1/namespaces/{ns}/search/stream                    │    │    │
│  │  │                                                             │    │    │
│  │  │ CRUD OPERATIONS:                                            │    │    │
│  │  │ • POST /v1/namespaces/{ns}/upsert                           │    │    │
│  │  │ • POST /v1/namespaces/{ns}/upsert-columns                   │    │    │
│  │  │ • PATCH /v1/namespaces/{ns}/update                          │    │    │
│  │  │ • PATCH /v1/namespaces/{ns}/update-columns                  │    │    │
│  │  │ • DELETE /v1/namespaces/{ns}/batch-delete                   │    │    │
│  │  │                                                             │    │    │
│  │  │ CONDITIONAL OPERATIONS:                                     │    │    │
│  │  │ • POST /v1/namespaces/{ns}/conditional-upsert               │    │    │
│  │  │ • PATCH /v1/namespaces/{ns}/conditional-update              │    │    │
│  │  │ • DELETE /v1/namespaces/{ns}/conditional-delete             │    │    │
│  │  │ • DELETE /v1/namespaces/{ns}/filter-delete                  │    │    │
│  │  │                                                             │    │    │
│  │  │ MANAGEMENT APIs:                                            │    │    │
│  │  │ • POST /v1/namespaces/{ns}/configure-metric                 │    │    │
│  │  │ • POST/PUT /v1/namespaces/{ns}/schema                       │    │    │
│  │  │ • POST/PUT /v1/namespaces/{ns}/encryption                   │    │    │
│  │  │ • POST /v1/namespaces/{ns}/copy                             │    │    │
│  │  │                                                             │    │    │
│  │  │ WEBHOOK INTERFACES:                                         │    │    │
│  │  │ • POST /v1/webhooks/search-results                          │    │    │
│  │  │ • POST /v1/webhooks/search-status                           │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  │                                                                     │    │
│  │  Features: Async handling • Request validation • Authentication     │    │
│  │           Error handling • Logging & tracing • Rate limiting        │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │              4. SPFRESH QUERY LAYER (Native Engine)                 │    │
│  │                                                                     │    │
│  │  ╔═══════════════════════════════════════════════════════════════╗  │    │
│  │  ║                SPFresh Native Query Engine                    ║  │    │
│  │  ╠═══════════════════════════════════════════════════════════════╣  │    │
│  │  ║                                                               ║  │    │
│  │  ║  ┌─────────────────┐    ┌─────────────────┐                   ║  │    │
│  │  ║  │   Query Parser  │───▶│ Query Optimizer │                   ║  │    │
│  │  ║  │ • Custom JSON   │    │ • Execution plan│                   ║  │    │
│  │  ║  │ • Ranking syntax│    │ • Index selection│                  ║  │    │
│  │  ║  │ • Filter syntax │    │ • Regional route│                   ║  │    │
│  │  ║  └─────────────────┘    └─────────────────┘                   ║  │    │
│  │  ║                                │                              ║  │    │
│  │  ║                                ▼                              ║  │    │
│  │  ║  ┌─────────────────────────────────────────────────────────┐  ║  │    │
│  │  ║  │              Query Execution Engine                     │  ║  │    │
│  │  ║  │                                                         │  ║  │    │
│  │  ║  │ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────┐ │  ║  │    │
│  │  ║  │ │Vector Search│ │ BM25 Search │ │ Attribute   │ │Multi│ │  ║  │    │
│  │  ║  │ │    (ANN)    │ │ (Fulltext)  │ │  Ordering   │ │Query│ │  ║  │    │
│  │  ║  │ └─────────────┘ └─────────────┘ └─────────────┘ └─────┘ │  ║  │    │
│  │  ║  │                                                         │  ║  │    │
│  │  ║  │ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────┐ │  ║  │    │
│  │  ║  │ │  Filtering  │ │ Aggregation │ │ Pagination  │ │Rslt │ │  ║  │    │
│  │  ║  │ │   Engine    │ │   Engine    │ │   Manager   │ │Fmt  │ │  ║  │    │
│  │  ║  │ └─────────────┘ └─────────────┘ └─────────────┘ └─────┘ │  ║  │    │
│  │  ║  └─────────────────────────────────────────────────────────┘  ║  │    │
│  │  ╚═══════════════════════════════════════════════════════════════╝  │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                5. EMBEDDING LAYER (FastEmbedding)                   │    │
│  │                                                                     │    │
│  │  ╔═══════════════════════════════════════════════════════════════╗  │    │
│  │  ║                 FastEmbedding Service                         ║  │    │
│  │  ╠═══════════════════════════════════════════════════════════════╣  │    │
│  │  ║                                                               ║  │    │
│  │  ║  ┌─────────────────┐    ┌─────────────────┐                   ║  │    │
│  │  ║  │Text Preprocessing│───▶│Vector Generation│                   ║  │    │
│  │  ║  │• Tokenization   │    │• Model Inference│                   ║  │    │
│  │  ║  │• Normalization  │    │• Dimensionality │                   ║  │    │
│  │  ║  │• Truncation     │    │• Normalization  │                   ║  │    │
│  │  ║  └─────────────────┘    └─────────────────┘                   ║  │    │
│  │  ║                                │                              ║  │    │
│  │  ║                                ▼                              ║  │    │
│  │  ║  ┌─────────────────────────────────────────────────────────┐  ║  │    │
│  │  ║  │            Batch Processing Engine                      │  ║  │    │
│  │  ║  │ • Optimal batch size: 32                                │  ║  │    │
│  │  ║  │ • Dynamic batching based on load                        │  ║  │    │
│  │  ║  │ • SIMD acceleration                                     │  ║  │    │
│  │  ║  │ • Multi-threading support                               │  ║  │    │
│  │  ║  └─────────────────────────────────────────────────────────┘  ║  │    │
│  │  ╚═══════════════════════════════════════════════════════════════╝  │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                   6. RUST FFI LAYER (Bindings)                      │    │
│  │                                                                     │    │
│  │  ┌───────────────────┐       ┌───────────────────┐                  │    │
│  │  │ Safe Rust Wrapper │       │ Memory Management │                  │    │
│  │  ├───────────────────┤       ├───────────────────┤                  │    │
│  │  │• Error handling   │       │• RAII patterns    │                  │    │
│  │  │• Type safety      │       │• Buffer management│                  │    │
│  │  │• Null safety      │       │• Ownership transfer│                 │    │
│  │  │• Thread safety    │       │• Resource cleanup │                  │    │
│  │  └─────────┬─────────┘       └─────────┬─────────┘                  │    │
│  │            │                           │                            │    │
│  │            └─────────────┬─────────────┘                            │    │
│  │                          │                                          │    │
│  │  ┌────────────────────────────────────────────────────────────┐     │    │
│  │  │              FFI Function Declarations                     │     │    │
│  │  │                                                            │     │    │
│  │  │  extern "C" {                                              │     │    │
│  │  │      fn SPFresh_Create(...) -> *mut SPFreshIndex;          │     │    │
│  │  │      fn SPFresh_AddBatch(...) -> i32;                      │     │    │
│  │  │      fn SPFresh_Search(...) -> i32;                        │     │    │
│  │  │      fn SPFresh_Delete(...) -> i32;                        │     │    │
│  │  │      fn SPFresh_Query(...) -> i32;                         │     │    │
│  │  │      fn SPFresh_Destroy(...);                              │     │    │
│  │  │  }                                                         │     │    │
│  │  └────────────────────────────────────────────────────────────┘     │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                   7. C++ INTERFACE LAYER                            │    │
│  │                                                                     │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                C API Implementation                         │    │    │
│  │  │                                                             │    │    │
│  │  │   C++ implementation (in spfresh_c_api.cpp)                 │    │    │
│  │  │   Added native query interface for SPFresh capabilities     │    │    │
│  │  │                                                             │    │    │
│  │  │   Features:                                                 │    │    │
│  │  │   • Exception handling and conversion to error codes       │    │    │
│  │  │   • Memory management for C++ objects                      │    │    │
│  │  │   • Thread-safe operations                                 │    │    │
│  │  │   • Resource cleanup and RAII                              │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    8. SPFRESH CORE (C++)                            │    │
│  │                                                                     │    │
│  │  ┌────────────────────────────────────────────────────────────┐     │    │
│  │  │                SPFresh Update Layer                        │     │    │
│  │  │                                                            │     │    │
│  │  │  ┌─────────────────┐      ┌─────────────────┐              │     │    │
│  │  │  │ In-Memory Buffer│      │ LIRE Rebalancer │              │     │    │
│  │  │  │   (Memtable)    │─────▶│                 │              │     │    │
│  │  │  └─────────────────┘      └─────────────────┘              │     │    │
│  │  │                                    │                       │     │    │
│  │  └────────────────────────────────────┼───────────────────────┘     │    │
│  │                                       │                             │    │
│  │                                       ▼                             │    │
│  │  ┌────────────────────────────────────────────────────────────┐     │    │
│  │  │                SPANN Index Layer                           │     │    │
│  │  │                                                            │     │    │
│  │  │  ┌─────────────────┐      ┌─────────────────┐              │     │    │
│  │  │  │ Centroid Index  │      │ Posting Lists   │              │     │    │
│  │  │  │  (In-Memory)    │─────▶│  (On-Disk)      │              │     │    │
│  │  │  │ Query-optimized │      │ Query-optimized │              │     │    │
│  │  │  └─────────────────┘      └─────────────────┘              │     │    │
│  │  │                                                            │     │    │
│  │  └────────────────────────────────────┼───────────────────────┘     │    │
│  │                                       │                             │    │
│  │                                       ▼                             │    │
│  │  ┌────────────────────────────────────────────────────────────┐     │    │
│  │  │                SPTAG Base Layer                            │     │    │
│  │  │                                                            │     │    │
│  │  │  ┌─────────────────┐      ┌─────────────────┐              │     │    │
│  │  │  │ Vector Index    │      │ Distance Calc   │              │     │    │
│  │  │  │ Algorithms      │─────▶│ Methods         │              │     │    │
│  │  │  │ Multi-metric    │      │ Native scoring  │              │     │    │
│  │  │  └─────────────────┘      └─────────────────┘              │     │    │
│  │  │                                                            │     │    │
│  │  └────────────────────────────────────────────────────────────┘     │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                9. STORAGE LAYER (Trait-Based)                       │    │
│  │                                                                     │    │
│  │  ╔═══════════════════════════════════════════════════════════════╗  │    │
│  │  ║              Storage Abstraction Layer                        ║  │    │
│  │  ╠═══════════════════════════════════════════════════════════════╣  │    │
│  │  ║                                                               ║  │    │
│  │  ║  ┌─────────────────────────────────────────────────────────┐  ║  │    │
│  │  ║  │              StorageBackend Trait                       │  ║  │    │
│  │  ║  │                                                         │  ║  │    │
│  │  ║  │  • write() / read() / delete()                          │  ║  │    │
│  │  ║  │  • batch_write() / list_keys()                          │  ║  │    │
│  │  ║  │  • get_backend_type() / get_capabilities()              │  ║  │    │
│  │  ║  │  • query_support() / filter_support()                   │  ║  │    │
│  │  ║  └─────────────────────────────────────────────────────────┘  ║  │    │
│  │  ║                                                               ║  │    │
│  │  ╚═══════════════════════════════════════════════════════════════╝  │    │
│  │                                  │                                 │    │
│  │                  ┌───────────────┼───────────────┐                 │    │
│  │                  │               │               │                 │    │
│  │                  ▼               ▼               ▼                 │    │
│  │  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐       │    │
│  │  │ RocksDB Backend │ │Cloud Storage    │ │ Hybrid Backend  │       │    │
│  │  │                 │ │Backend          │ │                 │       │    │
│  │  │• Local SSD      │ │                 │ │• Hot: RocksDB   │       │    │
│  │  │• Fast access    │ │• S3/GCS/Azure   │ │• Cold: Cloud    │       │    │
│  │  │• Limited scale  │ │• Unlimited scale│ │• Smart routing  │       │    │
│  │  │• Low latency    │ │• Higher latency │ │• Cache mgmt     │       │    │
│  │  │• Query-optimized│ │• Query-capable  │ │• Query-aware    │       │    │
│  │  └─────────────────┘ └─────────────────┘ └─────────────────┘       │    │
│  │                                  │                                 │    │
│  │                                  ▼                                 │    │
│  │  ┌─────────────────────────────────────────────────────────────┐   │    │
│  │  │              Storage Router & Manager                       │   │    │
│  │  │                                                             │   │    │
│  │  │  ┌─────────────────┐    ┌─────────────────┐                 │   │    │
│  │  │  │ Configuration   │    │ Runtime Routing │                 │   │    │
│  │  │  │ Management      │    │ Logic           │                 │   │    │
│  │  │  │                 │    │                 │                 │   │    │
│  │  │  │• Backend select │    │• Data locality  │                 │   │    │
│  │  │  │• Region mapping │    │• Load balancing │                 │   │    │
│  │  │  │• Failover rules │    │• Cache decisions│                 │   │    │
│  │  │  │• Query routing  │    │• Query optimize │                 │   │    │
│  │  │  └─────────────────┘    └─────────────────┘                 │   │    │
│  │  │                                                             │   │    │
│  │  │  ┌─────────────────────────────────────────────────────┐   │   │    │
│  │  │  │            Multi-Level Caching                      │   │   │    │
│  │  │  │                                                     │   │   │    │
│  │  │  │  • L1: In-memory (hot data)                         │   │   │    │
│  │  │  │  • L2: Local disk (warm data)                       │   │   │    │
│  │  │  │  • L3: Distributed (SPFresh native)                 │   │   │    │
│  │  │  │  • Cache invalidation and consistency               │   │   │    │
│  │  │  │  • Query result caching                             │   │   │    │
│  │  │  └─────────────────────────────────────────────────────┘   │   │    │
│  │  └─────────────────────────────────────────────────────────────┘   │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 2. SPFresh Cache Warming Strategy (Integrated with Your Architecture)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      SPFresh Cache Warming Strategy                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Analytics & Intelligence                          │    │
│  │                                                                     │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐    │    │
│  │  │Query Pattern│ │Access Freq  │ │Time Pattern │ │ML Predictor │    │    │
│  │  │Analyzer     │ │Tracker      │ │Detector     │ │Engine       │    │    │
│  │  │             │ │             │ │             │ │             │    │    │
│  │  │• Track hot  │ │• Count hits │ │• Peak hours │ │• Predict    │    │    │
│  │  │  queries    │ │• Frequency  │ │• Usage      │ │  future     │    │    │
│  │  │• Popular    │ │  analysis   │ │  patterns   │ │  queries    │    │    │
│  │  │  vectors    │ │• Access     │ │• Seasonal   │ │• Confidence │    │    │
│  │  │• Common     │ │  recency    │ │  trends     │ │  scoring    │    │    │
│  │  │  filters    │ │             │ │             │ │             │    │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘    │    │
│  │         │               │               │               │           │    │
│  │         └───────────────┼───────────────┼───────────────┘           │    │
│  │                         │               │                           │    │
│  │                         ▼               ▼                           │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              Warming Decision Engine                        │    │    │
│  │  │  • Identify hot data candidates                             │    │    │
│  │  │  • Calculate warming priorities                             │    │    │
│  │  │  • Schedule warming tasks                                   │    │    │
│  │  │  • Avoid cache pollution                                    │    │    │
│  │  └─────────────────────────┬───────────────────────────────────┘    │    │
│  └─────────────────────────────┼───────────────────────────────────────┘    │
│                                │                                            │
│                                ▼                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      Warming Execution                              │    │
│  │                                                                     │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐    │    │
│  │  │Proactive    │ │Predictive   │ │Time-based   │ │Event-driven │    │    │
│  │  │Warming      │ │Warming      │ │Warming      │ │Warming      │    │    │
│  │  │             │ │             │ │             │ │             │    │    │
│  │  │• Pre-load   │ │• ML predict │ │• Schedule   │ │• New data   │    │    │
│  │  │  popular    │ │  next       │ │  warming    │ │  triggers   │    │    │
│  │  │  queries    │ │  queries    │ │  before     │ │• Index      │    │    │
│  │  │• Cache hot  │ │• Pattern    │ │  peak       │ │  updates    │    │    │
│  │  │  vectors    │ │  matching   │ │  hours      │ │• Schema     │    │    │
│  │  │• Preload    │ │• Confidence │ │• Maintenance│ │  changes    │    │    │
│  │  │  indexes    │ │  > 70%      │ │  windows    │ │             │    │    │
│  │  └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘    │    │
│  │                                                                     │    │
│  │  Background Workers execute warming through your existing:          │    │
│  │  • API LAYER (Rust/Axum) - for query execution                     │    │
│  │  • SPFRESH QUERY LAYER - for processing                            │    │
│  │  • All your existing layers down to STORAGE LAYER                  │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 3. Integration Point in Your Current Flow

```
Your Current Request Flow:
CLIENT → LOAD BALANCER → API LAYER → SPFRESH QUERY LAYER → ...

With HotCache Added:
CLIENT → LOAD BALANCER → [🔥 HOTCACHE CHECK] → API LAYER → SPFRESH QUERY LAYER → ...
                                │
                                ├─ Cache Hit → Return (0.1ms)
                                │
                                └─ Cache Miss → Continue to your existing layers
                                               → Store result in cache
```

### 4. Cache Integration Implementation

The SPFresh HotCache integrates seamlessly with your existing architecture by adding a cache check layer before your API processing:

```rust
// Cache Integration in API Layer (Rust/Axum)
use spfresh_hotcache::{HotCache, CacheKey, CacheResult};

pub async fn vector_search_handler(
    Path(namespace): Path<String>,
    Json(request): Json<VectorSearchRequest>,
) -> Result<Json<SearchResponse>, ApiError> {
    let trace_id = generate_trace_id();
    
    // 🔥 STEP 1: Check HotCache first
    let cache_key = CacheKey::from_vector_search(&namespace, &request);
    
    match HotCache::get(&cache_key).await {
        CacheResult::Hit(cached_response) => {
            // Cache hit - return immediately (0.1ms)
            info!("Cache hit for query: {}", trace_id);
            return Ok(Json(cached_response));
        }
        CacheResult::Miss => {
            // Cache miss - continue to your existing layers
            info!("Cache miss for query: {}", trace_id);
        }
    }
    
    // 🔥 STEP 2: Continue with your existing architecture
    // API LAYER → SPFRESH QUERY LAYER → EMBEDDING LAYER → ... → STORAGE LAYER
    let response = process_vector_search_through_existing_layers(namespace, request).await?;
    
    // 🔥 STEP 3: Store result in cache for future requests
    HotCache::set(&cache_key, &response).await;
    
    Ok(Json(response))
}

// Your existing processing function remains unchanged
async fn process_vector_search_through_existing_layers(
    namespace: String, 
    request: VectorSearchRequest
) -> Result<SearchResponse, ApiError> {
    // This calls your existing:
    // • SPFRESH QUERY LAYER (Native Engine)
    // • EMBEDDING LAYER (FastEmbedding)  
    // • RUST FFI LAYER (Bindings)
    // • C++ INTERFACE LAYER
    // • SPFRESH CORE (C++)
    // • STORAGE LAYER (Trait-Based)
    
    // Your existing implementation here...
    todo!("Your existing SPFresh processing")
}
```

**Cache Key Generation Strategy:**
```rust
impl CacheKey {
    pub fn from_vector_search(namespace: &str, request: &VectorSearchRequest) -> Self {
        let mut hasher = DefaultHasher::new();
        namespace.hash(&mut hasher);
        request.vector.hash(&mut hasher);
        request.top_k.hash(&mut hasher);
        request.distance_threshold.hash(&mut hasher);
        
        CacheKey {
            hash: hasher.finish(),
            namespace: namespace.to_string(),
            query_type: QueryType::VectorSearch,
            ttl: Duration::from_secs(3600), // 1 hour default
        }
    }
}
```

**Cache Warming Background Service:**
```rust
// Background service that warms cache using your existing API
pub struct CacheWarmer {
    api_client: SPFreshApiClient,
    analytics: CacheAnalytics,
}

impl CacheWarmer {
    pub async fn warm_popular_queries(&self) -> Result<(), WarmingError> {
        let hot_queries = self.analytics.get_hot_queries().await?;
        
        for query in hot_queries {
            // Execute query through your existing API layers
            // This will populate the cache naturally
            let _response = self.api_client.execute_query(query).await?;
        }
        
        Ok(())
    }
}
```

### 5. Complete Data Flow Journey with HotCache Integration

```
Client Application                          SPFresh-Rust System with HotCache
─────────────────                           ────────────────────────────────────────────────
┌─────────────┐                          ┌──────────────────────────────────────────────┐
│   Client    │                          │                LOAD BALANCER                 │
│   Request   │                          │                                              │
└──────┬──────┘                          │  ┌────────────────────────────────┐          │
       │                                 │  │      Traffic Routing           │          │
       ▼                                 │  │  • Health checks               │          │
┌─────────────┐     HTTP Request         │  │  • Load balancing              │          │
│ JSON Payload│  ───────────────────────▶│  │  • Regional routing            │          │
│             │                          │  └─────────────────┬──────────────┘          │
└─────────────┘                          └────────────────────┼─────────────────────────┘
                                                              │
                                                              ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                    🔥 SPFRESH HOTCACHE LAYER                                 │
├──────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌────────────────────────────────┐          ┌─────────────────────────┐     │
│  │      Cache Key Generation      │          │    L1 Cache Check       │     │
│  │                                │          │    (Memory - Ultra Hot) │     │
│  │  • Hash query parameters       │─────────▶│                         │     │
│  │  • Include namespace           │          │  • Query Results (1GB)  │     │
│  │  • Add query type              │          │  • Vector Cache (2GB)   │     │
│  │  • Generate cache key          │          │  • Index Segments (4GB) │     │
│  └────────────────────────────────┘          │  • BM25 Terms (512MB)  │     │
│                                              └─────────┬───────────────┘     │
│                                                        │                     │
│                                              Cache Hit │ (0.1ms)             │
│                                                        ▼                     │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    ✅ L1 CACHE HIT RESPONSE                          │    │
│  │  • Return cached result immediately                                  │    │
│  │  • Update access metrics                                             │    │
│  │  • Log cache hit with trace ID                                       │    │
│  │  • Skip all downstream processing                                    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│  Cache Miss ▼                                                                │
│  ┌────────────────────────────────┐          ┌─────────────────────────┐     │
│  │      L2 Cache Check            │          │    L2 Cache Check       │     │
│  │      (SSD - Warm)              │          │    (NVMe SSD)           │     │
│  │                                │─────────▶│                         │     │
│  │  • Check SSD cache             │          │  • Posting Lists (100GB)│     │
│  │  • Load warm data              │          │  • Hot Metadata (50GB) │     │
│  │  • Promote to L1 if frequent   │          │  • Freq Vectors (200GB)│     │
│  └────────────────────────────────┘          │  • Popular Terms (50GB)│     │
│                                              └─────────┬───────────────┘     │
│                                                        │                     │
│                                              Cache Hit │ (2.5ms)             │
│                                                        ▼                     │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                ✅ L2 CACHE HIT + PROMOTE TO L1                       │    │
│  │  • Load data from SSD                                                │    │
│  │  • Promote frequently accessed items to L1                           │    │
│  │  • Update access patterns                                            │    │
│  │  • Return result (2.5ms)                                             │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│  Cache Miss ▼ (Continue to YOUR EXISTING ARCHITECTURE - UNCHANGED)           │
└──────────────────────────────────────────────────────────────────────────────┘
                                        │
                                        ▼
┌──────────────────────────────────────────────┐
│              🆕 QUERY LAYER                  │
│                                              │
│  ┌────────────────────────────────┐          │
│  │      Query Type Detection      │          │
│  │                                │          │
│  │  • Vector Search?              │          │
│  │  • BM25 Search?                │          │
│  │  • Hybrid Search?              │          │
│  └─────────────────┬──────────────┘          │
│                    │                         │
│                    ▼                         │
│  ┌────────────────────────────────┐          │
│  │      Query Router              │          │
│  │                                │          │
│  │  IF Vector/Text → Embedding    │          │
│  │  IF BM25 → Reverse Index       │          │
│  │  IF Hybrid → Both Engines      │          │
│  └─────────────────┬──────────────┘          │
│                    │                         │
└────────────────────┼─────────────────────────┘
                     │
                     ▼
┌──────────────────────────────────────────────┐
│              EMBEDDING LAYER                 │
│                                              │
│  ┌────────────────────────────────┐          │
│  │      Input Type Check          │          │
│  │                                │          │
│  │  • IF Query Layer → Vector     │          │
│  │  • Text or Vector?             │          │
│  └─────────────────┬──────────────┘          │
│                    │                         │
│                    ├─────────────┐           │
│                    │             │           │
│                    ▼             ▼           │
│  ┌────────────────────┐ ┌────────────────┐   │
│  │   FastEmbedding    │ │ Vector Input   │   │
│  │   (Rust Library)   │ │                │   │
│  │ • Tokenization     │ │ • Use provided │   │
│  │ • Neural embedding │ │   vectors      │   │
│  │ • Generate vectors │ │                │   │
│  └─────────┬──────────┘ └────────┬───────┘   │
│            │                     │           │
│            └──────────┬──────────┘           │
│                       │                      │
│                       ▼                      │
│  ┌────────────────────────────────┐          │
│  │      Vector Processing         │          │
│  │                                │          │
│  │  • Normalize vectors           │          │
│  │  • Format for SPFresh          │          │
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
│  │  • Call C++ functions          │          │
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
│  │      SPFresh Update Layer      │          │
│  │                                │          │
│  │  • In-memory buffer            │          │
│  │  • LIRE Rebalancer             │          │
│  └─────────────────┬──────────────┘          │
│                    │                         │
│                    ▼                         │
│  ┌────────────────────────────────┐          │
│  │      SPANN Index Layer         │          │
│  │                                │          │
│  │  • Centroid index (memory)     │          │
│  │  • Posting lists (disk)        │          │
│  │  • IF BM25 → Reverse Index     │          │
│  └─────────────────┬──────────────┘          │
│                    │                         │
│                    ▼                         │
│  ┌────────────────────────────────┐          │
│  │      SPTAG Base Layer          │          │
│  │                                │          │
│  │  • Vector algorithms           │          │
│  │  • Distance calculations       │          │
│  │  • IF Hybrid → Result Fusion   │          │
│  └─────────────────┬──────────────┘          │
│                    │                         │
└────────────────────┼─────────────────────────┘
                     │
                     ▼
┌──────────────────────────────────────────────┐
│              STORAGE LAYER (Trait-Based)    │
│                                              │
│  ┌────────────────┐   ┌─────────────────┐    │
│  │ RocksDB        │   │ Cloud Storage   │    │
│  │ • Hot data     │   │ • Cold data     │    │
│  │ • Metadata     │   │ • Vector data   │    │
│  │ • Fast access  │   │ • Durability    │    │
│  └────────┬───────┘   └────────┬────────┘    │
│           │                    │             │
│           └────────┬───────────┘             │
│                    │                         │
│                    ▼                         │
│  ┌────────────────────────────────┐          │
│  │      Storage Router & Manager  │          │
│  │                                │          │
│  │  • Backend selection           │          │
│  │  • Multi-level caching         │          │
│  │  • Regional coordination       │          │
│  └─────────────────┬──────────────┘          │
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
│  │  • Add similarity scores       │          │
│  │  • Add metadata                │          │
│  │  • IF Hybrid → Fusion scores   │          │
│  └─────────────────┬──────────────┘          │
│                    │                         │
└────────────────────┼─────────────────────────┘
                     │
┌─────────────┐                                               │
│   Client    │                                               │
│  Response   │◀──────────────────────────────────────────────┘
└─────────────┘
```

### 3. SPFresh HotCache Architecture Integration

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SPFresh HotCache Integration Flow                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Client Request                                                             │
│       │                                                                     │
│       ▼                                                                     │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      API Layer (Rust/Axum)                          │    │
│  │  • Parse request                                                    │    │
│  │  • Generate cache key                                               │    │
│  │  • Check request type (read/write)                                  │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                   SPFresh HotCache Layer                            │    │
│  │                                                                     │    │
│  │  L1 Cache (Memory) - Ultra Hot                                      │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐                    │    │
│  │  │Query Results│ │Vector Cache │ │Index Segments│                   │    │
│  │  │(1GB LRU)    │ │(2GB HashMap)│ │(4GB Segments)│                   │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘                    │    │
│  │         │ Hit           │ Hit           │ Hit                        │    │
│  │         ▼               ▼               ▼                            │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              Cache Hit Response                              │    │    │
│  │  │  • Return cached data                                       │    │    │
│  │  │  • Update access metrics                                    │    │    │
│  │  │  • Record cache hit                                         │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  │                                                                     │    │
│  │  Cache Miss ▼                                                       │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                L2 Cache (SSD) - Warm                        │    │    │
│  │  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐            │    │    │
│  │  │  │Posting Lists│ │Hot Metadata │ │Freq Vectors │            │    │    │
│  │  │  │(Recent)     │ │(Popular)    │ │(Accessed)   │            │    │    │
│  │  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘            │    │    │
│  │  │         │ Hit           │ Hit           │ Hit                │    │    │
│  │  │         ▼               ▼               ▼                    │    │    │
│  │  │  ┌─────────────────────────────────────────────────────┐    │    │    │
│  │  │  │          Promote to L1 + Return Data               │    │    │    │
│  │  │  └─────────────────────────────────────────────────────┘    │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  │                                                                     │    │
│  │  Cache Miss ▼                                                       │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              L3 Storage (Cold) - Compute                    │    │    │
│  │  │  • Execute query on SPFresh Core                            │    │    │
│  │  │  • Store result in appropriate cache level                  │    │    │
│  │  │  • Update cache analytics                                   │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    SPFresh Core Processing                           │    │
│  │  • SPANN Index Layer                                                │    │
│  │  • SPTAG Base Layer                                                 │    │
│  │  • Storage Layer (RocksDB/Cloud)                                    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 4. Cache Warming Strategy Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      SPFresh Cache Warming Architecture                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Analytics & Intelligence Layer                    │    │
│  │                                                                     │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐    │    │
│  │  │Query Pattern│ │Access Freq  │ │Time Pattern │ │ML Predictor │    │    │
│  │  │Analyzer     │ │Tracker      │ │Detector     │ │Engine       │    │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘    │    │
│  │         │               │               │               │           │    │
│  │         └───────────────┼───────────────┼───────────────┘           │    │
│  │                         │               │                           │    │
│  │                         ▼               ▼                           │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              Warming Decision Engine                        │    │    │
│  │  │  • Identify hot data candidates                             │    │    │
│  │  │  • Predict future access patterns                          │    │    │
│  │  │  • Calculate warming priorities                             │    │    │
│  │  │  • Schedule warming tasks                                   │    │    │
│  │  └─────────────────────────┬───────────────────────────────────┘    │    │
│  └─────────────────────────────┼───────────────────────────────────────┘    │
│                                │                                            │
│                                ▼                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      Cache Warming Scheduler                        │    │
│  │                                                                     │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐    │    │
│  │  │Proactive    │ │Predictive   │ │Time-based   │ │Event-driven │    │    │
│  │  │Warming      │ │Warming      │ │Warming      │ │Warming      │    │    │
│  │  │             │ │             │ │             │ │             │    │    │
│  │  │• Popular    │ │• ML-based   │ │• Peak hours │ │• New data   │    │    │
│  │  │  queries    │ │  prediction │ │• Scheduled  │ │• Index      │    │    │
│  │  │• Hot vectors│ │• Pattern    │ │  warming    │ │  updates    │    │    │
│  │  │• Freq index │ │  matching   │ │• Maintenance│ │• Schema     │    │    │
│  │  │  segments   │ │• Confidence │ │  windows    │ │  changes    │    │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘    │    │
│  │         │               │               │               │           │    │
│  │         └───────────────┼───────────────┼───────────────┘           │    │
│  │                         │               │                           │    │
│  │                         ▼               ▼                           │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                Warming Task Queue                           │    │    │
│  │  │  Priority Queue: High → Medium → Low                        │    │    │
│  │  │  • Task scheduling                                          │    │    │
│  │  │  • Resource allocation                                      │    │    │
│  │  │  • Conflict resolution                                      │    │    │
│  │  └─────────────────────────┬───────────────────────────────────┘    │    │
│  └─────────────────────────────┼───────────────────────────────────────┘    │
│                                │                                            │
│                                ▼                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Warming Execution Workers                        │    │
│  │                                                                     │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐    │    │
│  │  │Worker 1     │ │Worker 2     │ │Worker 3     │ │Worker N     │    │    │
│  │  │             │ │             │ │             │ │             │    │    │
│  │  │• Execute    │ │• Execute    │ │• Execute    │ │• Execute    │    │    │
│  │  │  queries    │ │  queries    │ │  queries    │ │  queries    │    │    │
│  │  │• Load data  │ │• Load data  │ │• Load data  │ │• Load data  │    │    │
│  │  │• Update     │ │• Update     │ │• Update     │ │• Update     │    │    │
│  │  │  cache      │ │  cache      │ │  cache      │ │  cache      │    │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘    │    │
│  │         │               │               │               │           │    │
│  │         └───────────────┼───────────────┼───────────────┘           │    │
│  │                         │               │                           │    │
│  │                         ▼               ▼                           │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              Cache Population Results                       │    │    │
│  │  │  • Success/failure tracking                                 │    │    │
│  │  │  • Performance metrics                                      │    │    │
│  │  │  • Cache hit rate improvements                              │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5. Regional Cache Distribution Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                   SPFresh Regional HotCache Distribution                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Global Cache Coordinator                         │    │
│  │  • Cross-region cache synchronization                               │    │
│  │  • Global hot data identification                                   │    │
│  │  • Cache invalidation coordination                                  │    │
│  │  • Regional cache health monitoring                                 │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│            ┌───────────────┼───────────────┐                               │
│            │               │               │                               │
│            ▼               ▼               ▼                               │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐               │
│  │  GCP Region     │ │  AWS Region     │ │ Azure Region    │               │
│  │ asia-southeast1 │ │   us-east-1     │ │   eastasia      │               │
│  ├─────────────────┤ ├─────────────────┤ ├─────────────────┤               │
│  │                 │ │                 │ │                 │               │
│  │ ┌─────────────┐ │ │ ┌─────────────┐ │ │ ┌─────────────┐ │               │
│  │ │Regional     │ │ │ │Regional     │ │ │ │Regional     │ │               │
│  │ │Cache Manager│ │ │ │Cache Manager│ │ │ │Cache Manager│ │               │
│  │ └──────┬──────┘ │ │ └──────┬──────┘ │ │ └──────┬──────┘ │               │
│  │        │        │ │        │        │ │        │        │               │
│  │        ▼        │ │        ▼        │ │        ▼        │               │
│  │ ┌─────────────┐ │ │ ┌─────────────┐ │ │ ┌─────────────┐ │               │
│  │ │L1: Memory   │ │ │ │L1: Memory   │ │ │ │L1: Memory   │ │               │
│  │ │• 8GB Query  │ │ │ │• 8GB Query  │ │ │ │• 8GB Query  │ │               │
│  │ │• 16GB Vector│ │ │ │• 16GB Vector│ │ │ │• 16GB Vector│ │               │
│  │ │• 32GB Index │ │ │ │• 32GB Index │ │ │ │• 32GB Index │ │               │
│  │ └──────┬──────┘ │ │ └──────┬──────┘ │ │ └──────┬──────┘ │               │
│  │        │        │ │        │        │ │        │        │               │
│  │        ▼        │ │        ▼        │ │        ▼        │               │
│  │ ┌─────────────┐ │ │ ┌─────────────┐ │ │ ┌─────────────┐ │               │
│  │ │L2: SSD      │ │ │ │L2: SSD      │ │ │ │L2: SSD      │ │               │
│  │ │• 500GB Warm │ │ │ │• 500GB Warm │ │ │ │• 500GB Warm │ │               │
│  │ │• NVMe PCIe4 │ │ │ │• NVMe PCIe4 │ │ │ │• NVMe PCIe4 │ │               │
│  │ │• 100K IOPS  │ │ │ │• 100K IOPS  │ │ │ │• 100K IOPS  │ │               │
│  │ └──────┬──────┘ │ │ └──────┬──────┘ │ │ └──────┬──────┘ │               │
│  │        │        │ │        │        │ │        │        │               │
│  │        ▼        │ │        ▼        │ │        ▼        │               │
│  │ ┌─────────────┐ │ │ ┌─────────────┐ │ │ ┌─────────────┐ │               │
│  │ │L3: Cloud    │ │ │ │L3: Cloud    │ │ │ │L3: Cloud    │ │               │
│  │ │• GCS        │ │ │ │• S3         │ │ │ │• Blob       │ │               │
│  │ │• Unlimited  │ │ │ │• Unlimited  │ │ │ │• Unlimited  │ │               │
│  │ │• Cold Data  │ │ │ │• Cold Data  │ │ │ │• Cold Data  │ │               │
│  │ └─────────────┘ │ │ └─────────────┘ │ │ └─────────────┘ │               │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘               │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Cache Synchronization Flow                       │    │
│  │                                                                     │    │
│  │  Hot Data Identification:                                           │    │
│  │  Region A ──────────────▶ Global Coordinator ◀────────────── Region C │    │
│  │                                    ▲                                │    │
│  │                                    │                                │    │
│  │                              Region B                               │    │
│  │                                                                     │    │
│  │  Cache Warming Propagation:                                         │    │
│  │  Global Coordinator ──────▶ All Regions (Async)                     │    │
│  │                                                                     │    │
│  │  Invalidation Broadcast:                                            │    │
│  │  Write Region ──────▶ Global Coordinator ──────▶ All Other Regions  │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 6. Post-Write Processing Flow

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Post-Write Processing                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. Success Response                                                │
│  {                                                                  │
│    "status": "ok",                                                  │
│    "inserted": 3,                                                   │
│    "updated": 0,                                                    │
│    "deleted": 0,                                                    │
│    "errors": []                                                     │
│  }                                                                  │
│                                                                     │
│  2. Async Index Building Trigger                                    │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│  │ WAL Monitor │────▶│Index Queue  │────▶│Index Builder│            │
│  │  (Polling)  │     │  (In-Memory)│     │  (Workers)  │            │
│  └─────────────┘     └─────────────┘     └─────────────┘            │
│                                                                     │
│  3. Cache Invalidation                                              │
│  - Invalidate affected namespace cache entries                      │
│  - Update namespace metadata (doc count, last write time)           │
│  - Trigger pre-warming if configured                                │
└─────────────────────────────────────────────────────────────────────┘
```

### 7. Post-Query Processing Flow

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Post-Query Processing                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. Search Response                                                 │
│  {                                                                  │
│    "status": "ok",                                                  │
│    "results": [...],                                                │
│    "total_count": 5,                                                │
│    "search_time_ms": 15,                                            │
│    "search_id": "search-2025-07-23-abc123"                         │
│  }                                                                  │
│                                                                     │
│  2. Query Analytics & Caching                                       │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│  │Query Logger │────▶│Result Cache │────▶│Analytics DB │            │
│  │(Metrics)    │     │(Redis/Mem) │     │(ClickHouse) │            │
│  └─────────────┘     └─────────────┘     └─────────────┘            │
│                                                                     │
│  3. Search Optimization                                             │
│  - Cache popular query results for faster retrieval                │
│  - Log search patterns and performance metrics                     │
│  - Update query frequency counters                                 │
│  - Trigger index optimization if needed                            │
│                                                                     │
│  4. Real-time Monitoring                                            │
│  - Track search latency and throughput                             │
│  - Monitor result quality and relevance scores                     │
│  - Alert on performance degradation                                │
│  - Update search recommendation models                              │
└─────────────────────────────────────────────────────────────────────┘
```

### 8. SPFresh Reverse Index Architecture

```
┌───────────────────────┐
│                       │
│    USER SEARCH        │
│    REQUEST            │
│                       │
└──────────┬────────────┘
           │
           ▼
┌──────────────────────┐
│                      │
│   API LAYER          │
│   (Rust/Axum)        │
│                      │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│                      │
│   QUERY PARSER       │
│                      │
└────────────┬─────────┘
             │
┌───────────────┬──────────────────┬─────────────────┐
│               │                  │                 │
▼               ▼                  ▼                 ▼
┌───────────────────┐ ┌────────────────────┐ ┌───────────────────┐ ┌───────────────────┐
│ METADATA FILTER   │ │ VECTOR SIMILARITY  │ │ FULLTEXT SEARCH   │ │ PAGINATION &      │
│ PROCESSING        │ │ SEARCH             │ │ PROCESSING        │ │ SORTING           │
└────────┬──────────┘ └─────────┬──────────┘ └────────┬──────────┘ └────────┬──────────┘
         │                      │                     │                     │
         ▼                      ▼                     ▼                     │
┌───────────────────┐ ┌────────────────────┐ ┌───────────────────┐          │
│  METADATA INDEX   │ │  VECTOR INDEX      │ │  FULLTEXT INDEX   │          │
│                   │ │                    │ │                   │          │
│  Field → IDs      │ │  Centroid → IDs    │ │  Term → IDs      │          │
│                   │ │                    │ │                   │          │
│  category:        │ │  Centroid 1        │ │  "smartphone"     │          │
│  electronics      │ │  ↓                 │ │  ↓                │          │
│  ↓                │ │  [ID1, ID5, ID9]   │ │  [ID1, ID7]       │          │
│  [ID1, ID5, ID9]  │ │                    │ │                   │          │
│                   │ │  Centroid 2        │ │  "wireless"       │          │
│  price:           │ │  ↓                 │ │  ↓                │          │
│  range(0,100)     │ │  [ID2, ID7, ID8]   │ │  [ID5, ID9]       │          │
│  ↓                │ │                    │ │                   │          │
│  [ID2, ID7, ID8]  │ │                    │ │                   │          │
└────────┬──────────┘ └─────────┬──────────┘ └────────┬──────────┘          │
         │                      │                     │                     │
         └──────────┬───────────┴─────────────────────┘                     │
                    │                                                       │
                    ▼                                                       │
┌────────────────────┐                                           ││  RESULT CANDIDATE  │                                           │
│  SET               │                                           │
└────────┬───────────┘                                           │
         │                                                       │
         └─────────────────┬───────────────────────────────────┬─┘
                           │                                   │
                           ▼                                   │
┌────────────────────┐                         ││  ID MAPPING        │                         │
│                    │                         │
│  Vector ID → Meta  │                         │
│                    │                         │
│  ID1 → {title: "A",│                         │
│        category: "B"}                        │
│                    │                         │
│  ID2 → {title: "C",│                         │
│        category: "D"}                        │
│                    │                         │
└────────┬───────────┘                         │
         │                                     │
         ▼                                     │
┌────────────────────┐                         │
│                    │                         │
│  FINAL RESULTS     │◀────────────────────────┘
│                    │
└────────┬───────────┘
         │
         ▼
┌────────────────────┐
│                    │
│  USER RESPONSE     │
│                    │
└────────────────────┘
```

### 9. Cache Performance Monitoring Dashboard

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                     SPFresh HotCache Performance Dashboard                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                        Cache Hit Rates                              │    │
│  │                                                                     │    │
│  │  L1 Cache (Memory):     ████████████████████████ 95.2%              │    │
│  │  L2 Cache (SSD):        ████████████████████     87.8%              │    │
│  │  L3 Cache (Cloud):      ████████████             65.4%              │    │
│  │  Overall Hit Rate:      ████████████████████████ 91.7%              │    │
│  │                                                                     │    │
│  │  Cache Miss Rate:       ████                     8.3%               │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      Response Time Metrics                          │    │
│  │                                                                     │    │
│  │  L1 Hit Response:       0.1ms   ████████████████████████████████    │    │
│  │  L2 Hit Response:       2.5ms   ████████████████████████████████    │    │
│  │  L3 Hit Response:       15ms    ████████████████████████████████    │    │
│  │  Cache Miss:            45ms    ████████████████████████████████    │    │
│  │                                                                     │    │
│  │  Average Response:      3.2ms   ████████████████████████████████    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                       Memory Utilization                            │    │
│  │                                                                     │    │
│  │  L1 Query Cache:        ████████████████████     7.2GB / 8GB        │    │
│  │  L1 Vector Cache:       ████████████████████████ 14.8GB / 16GB      │    │
│  │  L1 Index Cache:        ████████████████████████ 28.5GB / 32GB      │    │
│  │                                                                     │    │
│  │  L2 SSD Usage:          ████████████████████████ 420GB / 500GB      │    │
│  │  L3 Cloud Usage:        ████████████████████████ 2.1TB / ∞          │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      Cache Operations/sec                           │    │
│  │                                                                     │    │
│  │  Cache Reads:           ████████████████████████ 8,500 ops/sec      │    │
│  │  Cache Writes:          ████████████████████     3,200 ops/sec      │    │
│  │  Cache Evictions:       ████████                 1,100 ops/sec      │    │
│  │  Cache Promotions:      ████████████             2,400 ops/sec      │    │
│  │                                                                     │    │
│  │  Warming Tasks:         ████████████████████████ 450 tasks/min      │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                        Top Hot Queries                              │    │
│  │                                                                     │    │
│  │  1. "smartphone accessories"     ████████████████████ 2,341 hits    │    │
│  │  2. "wireless charging"          ████████████████████ 1,987 hits    │    │
│  │  3. "bluetooth headphones"       ████████████████████ 1,654 hits    │    │
│  │  4. "laptop computers"           ████████████████████ 1,432 hits    │    │
│  │  5. "gaming peripherals"         ████████████████████ 1,298 hits    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                     Regional Cache Status                           │    │
│  │                                                                     │    │
│  │  GCP asia-southeast1:   ████████████████████████ Healthy (91.2%)    │    │
│  │  AWS us-east-1:         ████████████████████████ Healthy (93.7%)    │    │
│  │  Azure eastasia:        ████████████████████████ Healthy (89.4%)    │    │
│  │                                                                     │    │
│  │  Sync Status:           ████████████████████████ All regions synced │    │
│  │  Last Sync:             2 minutes ago                               │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

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
│                      6. C++ INTERFACE LAYER                                    │
│  C API Wrapper │ Exception Handling │ Resource Management                   │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                       7. SPFRESH CORE (C++)                                 │
│  Update Layer │ SPANN Index │ SPTAG Base │ Vector Algorithms                │
└─────────────────────────────────────────────────────────────────────────────┘
                                   │
                                   ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                    8. STORAGE LAYER (Multi-Cloud)                           │
│  RocksDB │ Cloud Storage │ Memory Cache │ Persistent Disk                   │
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

**Search Webhook Interfaces และ API Endpoints**:

ระบบ SPFresh รองรับ 8 เส้น API หลักสำหรับการ Query และ Search Operations:

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
        // ===== SEARCH & QUERY APIs (8 เส้นหลัก) =====
        
        // 1. Vector Similarity Search - ค้นหาด้วย Vector
        .route("/v1/namespaces/:namespace/search/vector", post(vector_search_handler))
        
        // 2. Text Search with Embedding - ค้นหาด้วย Text (แปลงเป็น Vector อัตโนมัติ)
        .route("/v1/namespaces/:namespace/search/text", post(text_search_handler))
        
        // 3. Hybrid Search - ค้นหาแบบผสม Vector + BM25
        .route("/v1/namespaces/:namespace/search/hybrid", post(hybrid_search_handler))
        
        // 4. BM25 Full-Text Search - ค้นหาแบบ Full-Text
        .route("/v1/namespaces/:namespace/search/bm25", post(bm25_search_handler))
        
        // 5. Multi-Vector Search - ค้นหาด้วย Vector หลายตัวพร้อมกัน
        .route("/v1/namespaces/:namespace/search/multi-vector", post(multi_vector_search_handler))
        
        // 6. Filtered Search - ค้นหาพร้อม Metadata Filtering
        .route("/v1/namespaces/:namespace/search/filtered", post(filtered_search_handler))
        
        // 7. Batch Search - ค้นหาแบบ Batch หลาย Query พร้อมกัน
        .route("/v1/namespaces/:namespace/search/batch", post(batch_search_handler))
        
        // 8. Real-time Search Stream - ค้นหาแบบ Real-time Stream
        .route("/v1/namespaces/:namespace/search/stream", get(stream_search_handler))
        
        // ===== WEBHOOK INTERFACES =====
        
        // Search Result Webhook - รับผลลัพธ์การค้นหาแบบ Async
        .route("/v1/webhooks/search-results", post(search_webhook_handler))
        
        // Search Status Webhook - รับสถานะการค้นหา
        .route("/v1/webhooks/search-status", post(search_status_webhook_handler))
        
        // ===== ADDITIONAL QUERY API =====
        
        // General Query API - รองรับ Query หลากหลายรูปแบบ
        .route("/v1/namespaces/:namespace/query", post(general_query_handler))
        
        // ===== CRUD OPERATIONS =====
        .route("/v1/namespaces/:namespace/upsert", post(upsert_handler))
        .route("/v1/namespaces/:namespace/upsert-columns", post(upsert_columns_handler))
        .route("/v1/namespaces/:namespace/update", patch(update_handler))
        .route("/v1/namespaces/:namespace/update-columns", patch(update_columns_handler))
        .route("/v1/namespaces/:namespace/batch-delete", delete(batch_delete_handler))
        
        // ===== CONDITIONAL OPERATIONS =====
        .route("/v1/namespaces/:namespace/conditional-upsert", post(conditional_upsert_handler))
        .route("/v1/namespaces/:namespace/conditional-update", patch(conditional_update_handler))
        .route("/v1/namespaces/:namespace/conditional-delete", delete(conditional_delete_handler))
        .route("/v1/namespaces/:namespace/filter-delete", delete(filter_delete_handler))
        
        // ===== CONFIGURATION & SCHEMA APIs =====
        .route("/v1/namespaces/:namespace/configure-metric", post(configure_metric_handler))
        .route("/v1/namespaces/:namespace/schema", post(create_schema_handler))
        .route("/v1/namespaces/:namespace/schema", put(update_schema_handler))
        .route("/v1/namespaces/:namespace/encryption", post(configure_encryption_handler))
        .route("/v1/namespaces/:namespace/encryption", put(update_encryption_handler))
        
        // ===== NAMESPACE OPERATIONS =====
        .route("/v1/namespaces/:namespace/copy", post(copy_namespace_handler))
        
        // ===== MANAGEMENT APIs =====
        .route("/v1/namespaces/:namespace/stats", get(namespace_stats_handler))
        .route("/v1/health", get(health_check_handler))
        .route("/v1/ready", get(readiness_check_handler))
        
        .layer(
            ServiceBuilder::new()
                .layer(TraceLayer::new_for_http())
                .layer(CompressionLayer::new())
                .layer(CorsLayer::permissive())
                .layer(middleware::from_fn(auth_middleware))
                .layer(middleware::from_fn(rate_limit_middleware))
                .layer(middleware::from_fn(webhook_validation_middleware))
        )
}
```

**Search API Request/Response Models**:

```rust
use serde::{Deserialize, Serialize};
use validator::{Validate, ValidationError};
use chrono::{DateTime, Utc};

// ===== 1. Vector Similarity Search =====
#[derive(Debug, Deserialize, Validate)]
pub struct VectorSearchRequest {
    #[validate(length(min = 1, max = 2048))]
    pub vector: Vec<f32>,
    #[validate(range(min = 1, max = 1000))]
    pub top_k: usize,
    pub include_metadata: Option<bool>,
    pub include_vectors: Option<bool>,
    pub distance_threshold: Option<f32>,
}

// ===== 2. Text Search with Embedding =====
#[derive(Debug, Deserialize, Validate)]
pub struct TextSearchRequest {
    #[validate(length(min = 1, max = 10000))]
    pub text: String,
    #[validate(range(min = 1, max = 1000))]
    pub top_k: usize,
    pub embedding_model: Option<String>,
    pub include_metadata: Option<bool>,
    pub preprocessing_options: Option<TextPreprocessingOptions>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct TextPreprocessingOptions {
    pub normalize: bool,
    pub remove_stopwords: bool,
    pub max_tokens: Option<usize>,
    pub language: Option<String>,
}

// ===== 3. Hybrid Search =====
#[derive(Debug, Deserialize, Validate)]
pub struct HybridSearchRequest {
    #[validate(length(min = 1, max = 10000))]
    pub text: String,
    pub vector: Option<Vec<f32>>,
    #[validate(range(min = 1, max = 1000))]
    pub top_k: usize,
    #[validate(range(min = 0.0, max = 1.0))]
    pub vector_weight: f32,  // น้ำหนักของ Vector Search (0.0-1.0)
    #[validate(range(min = 0.0, max = 1.0))]
    pub bm25_weight: f32,    // น้ำหนักของ BM25 Search (0.0-1.0)
    pub fusion_method: Option<FusionMethod>,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum FusionMethod {
    RRF,           // Reciprocal Rank Fusion
    WeightedSum,   // Weighted Sum
    CombSUM,       // Combination Sum
    CombMNZ,       // Combination Max-Min-Z
}

// ===== 4. BM25 Full-Text Search =====
#[derive(Debug, Deserialize, Validate)]
pub struct BM25SearchRequest {
    #[validate(length(min = 1, max = 10000))]
    pub query: String,
    #[validate(range(min = 1, max = 1000))]
    pub top_k: usize,
    pub fields: Option<Vec<String>>,  // ระบุ field ที่ต้องการค้นหา
    pub boost_fields: Option<HashMap<String, f32>>,  // น้ำหนักของแต่ละ field
    pub bm25_params: Option<BM25Parameters>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct BM25Parameters {
    pub k1: f32,  // Term frequency saturation parameter (default: 1.2)
    pub b: f32,   // Length normalization parameter (default: 0.75)
}

// ===== 5. Multi-Vector Search =====
#[derive(Debug, Deserialize, Validate)]
pub struct MultiVectorSearchRequest {
    #[validate(length(min = 1, max = 100))]
    pub vectors: Vec<Vec<f32>>,
    #[validate(range(min = 1, max = 1000))]
    pub top_k: usize,
    pub aggregation_method: AggregationMethod,
    pub weights: Option<Vec<f32>>,  // น้ำหนักของแต่ละ vector
}

#[derive(Debug, Deserialize, Serialize)]
pub enum AggregationMethod {
    Average,      // เฉลี่ยคะแนน
    Maximum,      // คะแนนสูงสุด
    Minimum,      // คะแนนต่ำสุด
    WeightedSum,  // ผลรวมถ่วงน้ำหนัก
}

// ===== 6. Filtered Search =====
#[derive(Debug, Deserialize, Validate)]
pub struct FilteredSearchRequest {
    pub vector: Option<Vec<f32>>,
    pub text: Option<String>,
    #[validate(range(min = 1, max = 1000))]
    pub top_k: usize,
    pub filters: Vec<MetadataFilter>,
    pub filter_logic: Option<FilterLogic>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct MetadataFilter {
    pub field: String,
    pub operator: FilterOperator,
    pub value: serde_json::Value,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum FilterOperator {
    Equals,
    NotEquals,
    GreaterThan,
    LessThan,
    GreaterThanOrEqual,
    LessThanOrEqual,
    In,
    NotIn,
    Contains,
    StartsWith,
    EndsWith,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum FilterLogic {
    And,
    Or,
}

// ===== 7. Batch Search =====
#[derive(Debug, Deserialize, Validate)]
pub struct BatchSearchRequest {
    #[validate(length(min = 1, max = 100))]
    pub queries: Vec<SearchQuery>,
    pub parallel_execution: Option<bool>,
    pub timeout_seconds: Option<u64>,
}

#[derive(Debug, Deserialize, Serialize)]
#[serde(tag = "type")]
pub enum SearchQuery {
    Vector(VectorSearchRequest),
    Text(TextSearchRequest),
    Hybrid(HybridSearchRequest),
    BM25(BM25SearchRequest),
    Filtered(FilteredSearchRequest),
}

// ===== 8. Real-time Search Stream =====
#[derive(Debug, Deserialize, Validate)]
pub struct StreamSearchRequest {
    pub subscription_id: String,
    pub search_config: SearchQuery,
    pub stream_options: StreamOptions,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct StreamOptions {
    pub buffer_size: Option<usize>,
    pub flush_interval_ms: Option<u64>,
    pub max_results_per_batch: Option<usize>,
}

// ===== Common Response Models =====
#[derive(Debug, Serialize)]
pub struct SearchResponse {
    pub results: Vec<SearchResult>,
    pub total_count: usize,
    pub search_time_ms: u64,
    pub metadata: SearchMetadata,
}

#[derive(Debug, Serialize)]
pub struct SearchResult {
    pub id: String,
    pub score: f32,
    pub metadata: Option<HashMap<String, serde_json::Value>>,
    pub vector: Option<Vec<f32>>,
    pub text_snippet: Option<String>,
    pub rank: usize,
}

#[derive(Debug, Serialize)]
pub struct SearchMetadata {
    pub search_type: String,
    pub namespace: String,
    pub timestamp: DateTime<Utc>,
    pub index_version: u64,
    pub total_vectors: usize,
    pub search_params: serde_json::Value,
}

// ===== Webhook Models =====
#[derive(Debug, Serialize)]
pub struct SearchWebhookPayload {
    pub webhook_id: String,
    pub search_id: String,
    pub status: SearchStatus,
    pub results: Option<SearchResponse>,
    pub error: Option<String>,
    pub timestamp: DateTime<Utc>,
}

#[derive(Debug, Serialize)]
pub enum SearchStatus {
    Started,
    InProgress,
    Completed,
    Failed,
    Timeout,
}

// ===== ADDITIONAL API REQUEST MODELS =====

// General Query Request - รองรับ Query หลากหลายรูปแบบ
#[derive(Debug, Deserialize, Validate)]
pub struct GeneralQueryRequest {
    pub query_type: QueryType,
    pub parameters: serde_json::Value,
    #[validate(range(min = 1, max = 1000))]
    pub top_k: usize,
    pub options: Option<QueryOptions>,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum QueryType {
    Vector,
    Text,
    Hybrid,
    BM25,
    MultiVector,
    Filtered,
    SQL,  // SQL-like query support
    GraphQL,  // GraphQL query support
}

#[derive(Debug, Deserialize, Serialize)]
pub struct QueryOptions {
    pub timeout_ms: Option<u64>,
    pub include_metadata: Option<bool>,
    pub include_vectors: Option<bool>,
    pub explain_plan: Option<bool>,  // Return query execution plan
}

// ===== CRUD Request Models =====
#[derive(Debug, Deserialize, Validate)]
pub struct UpsertRequest {
    #[validate(length(min = 1, max = 1000))]
    pub upsert_rows: Vec<UpsertRow>,
    pub options: Option<UpsertOptions>,
}

#[derive(Debug, Deserialize, Validate)]
pub struct UpsertRow {
    #[validate(length(min = 1, max = 255))]
    pub id: String,
    #[validate(custom = "validate_vector")]
    pub vector: Option<Vec<f32>>,
    pub text: Option<String>,  // สำหรับ auto-embedding
    pub metadata: Option<serde_json::Value>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct UpsertOptions {
    pub auto_generate_embedding: Option<bool>,
    pub embedding_model: Option<String>,
    pub conflict_resolution: Option<ConflictResolution>,
    pub batch_size: Option<usize>,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum ConflictResolution {
    Replace,    // Replace existing record
    Merge,      // Merge with existing metadata
    Skip,       // Skip if exists
    Error,      // Return error if exists
}

// Upsert Columns Request - อัปเดตเฉพาะ columns ที่ระบุ
#[derive(Debug, Deserialize, Validate)]
pub struct UpsertColumnsRequest {
    #[validate(length(min = 1, max = 1000))]
    pub rows: Vec<ColumnUpsertRow>,
    pub columns: Vec<String>,  // ระบุ columns ที่ต้องการอัปเดต
    pub options: Option<UpsertOptions>,
}

#[derive(Debug, Deserialize, Validate)]
pub struct ColumnUpsertRow {
    #[validate(length(min = 1, max = 255))]
    pub id: String,
    pub values: HashMap<String, serde_json::Value>,  // column_name -> value
}

// Update Request Models
#[derive(Debug, Deserialize, Validate)]
pub struct UpdateRequest {
    #[validate(length(min = 1, max = 1000))]
    pub updates: Vec<UpdateRow>,
    pub options: Option<UpdateOptions>,
}

#[derive(Debug, Deserialize, Validate)]
pub struct UpdateRow {
    #[validate(length(min = 1, max = 255))]
    pub id: String,
    pub vector: Option<Vec<f32>>,
    pub text: Option<String>,
    pub metadata: Option<serde_json::Value>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct UpdateOptions {
    pub create_if_not_exists: Option<bool>,
    pub partial_update: Option<bool>,  // Allow partial metadata updates
    pub version_check: Option<bool>,   // Check version before update
}

// Update Columns Request
#[derive(Debug, Deserialize, Validate)]
pub struct UpdateColumnsRequest {
    #[validate(length(min = 1, max = 1000))]
    pub rows: Vec<ColumnUpdateRow>,
    pub columns: Vec<String>,
    pub options: Option<UpdateOptions>,
}

#[derive(Debug, Deserialize, Validate)]
pub struct ColumnUpdateRow {
    #[validate(length(min = 1, max = 255))]
    pub id: String,
    pub values: HashMap<String, serde_json::Value>,
    pub version: Option<u64>,  // For optimistic locking
}

// ===== CONDITIONAL OPERATIONS =====

// Conditional Upsert Request
#[derive(Debug, Deserialize, Validate)]
pub struct ConditionalUpsertRequest {
    #[validate(length(min = 1, max = 1000))]
    pub rows: Vec<ConditionalUpsertRow>,
    pub options: Option<UpsertOptions>,
}

#[derive(Debug, Deserialize, Validate)]
pub struct ConditionalUpsertRow {
    #[validate(length(min = 1, max = 255))]
    pub id: String,
    pub vector: Option<Vec<f32>>,
    pub text: Option<String>,
    pub metadata: Option<serde_json::Value>,
    pub conditions: Vec<Condition>,  // เงื่อนไขสำหรับการ upsert
}

// Conditional Update Request
#[derive(Debug, Deserialize, Validate)]
pub struct ConditionalUpdateRequest {
    #[validate(length(min = 1, max = 1000))]
    pub updates: Vec<ConditionalUpdateRow>,
    pub options: Option<UpdateOptions>,
}

#[derive(Debug, Deserialize, Validate)]
pub struct ConditionalUpdateRow {
    #[validate(length(min = 1, max = 255))]
    pub id: String,
    pub vector: Option<Vec<f32>>,
    pub text: Option<String>,
    pub metadata: Option<serde_json::Value>,
    pub conditions: Vec<Condition>,
}

// Conditional Delete Request
#[derive(Debug, Deserialize, Validate)]
pub struct ConditionalDeleteRequest {
    #[validate(length(min = 1, max = 1000))]
    pub ids: Vec<String>,
    pub conditions: Vec<Condition>,
    pub options: Option<DeleteOptions>,
}

// Filter Delete Request - ลบตาม filter conditions
#[derive(Debug, Deserialize, Validate)]
pub struct FilterDeleteRequest {
    pub filters: Vec<MetadataFilter>,
    pub filter_logic: Option<FilterLogic>,
    pub max_delete_count: Option<usize>,  // จำกัดจำนวนที่ลบ
    pub dry_run: Option<bool>,  // ทดสอบก่อนลบจริง
    pub options: Option<DeleteOptions>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Condition {
    pub field: String,
    pub operator: ConditionOperator,
    pub value: serde_json::Value,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum ConditionOperator {
    Exists,
    NotExists,
    Equals,
    NotEquals,
    GreaterThan,
    LessThan,
    VersionEquals,
    VersionGreaterThan,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct DeleteOptions {
    pub soft_delete: Option<bool>,  // Soft delete instead of hard delete
    pub backup_before_delete: Option<bool>,
    pub cascade_delete: Option<bool>,  // Delete related records
}

// ===== CONFIGURATION & SCHEMA APIs =====

// Configure Metric Request
#[derive(Debug, Deserialize, Validate)]
pub struct ConfigureMetricRequest {
    pub metric_type: MetricType,
    pub parameters: MetricParameters,
    pub apply_to_existing: Option<bool>,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum MetricType {
    Cosine,
    Euclidean,
    DotProduct,
    Manhattan,
    Hamming,
    Jaccard,
    Custom(String),
}

#[derive(Debug, Deserialize, Serialize)]
pub struct MetricParameters {
    pub normalization: Option<bool>,
    pub weight_vector: Option<Vec<f32>>,
    pub custom_params: Option<HashMap<String, serde_json::Value>>,
}

// Schema Management Request
#[derive(Debug, Deserialize, Validate)]
pub struct SchemaRequest {
    pub schema: NamespaceSchema,
    pub migration_strategy: Option<MigrationStrategy>,
    pub validate_existing_data: Option<bool>,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum MigrationStrategy {
    Strict,      // Fail if data doesn't match new schema
    Lenient,     // Try to convert existing data
    Additive,    // Only allow adding new fields
    Background,  // Migrate data in background
}

// Encryption Configuration Request
#[derive(Debug, Deserialize, Validate)]
pub struct EncryptionRequest {
    pub encryption_config: EncryptionConfig,
    pub apply_to_existing: Option<bool>,
    pub key_rotation: Option<KeyRotationConfig>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct EncryptionConfig {
    pub algorithm: EncryptionAlgorithm,
    pub key_id: String,
    pub encrypt_vectors: bool,
    pub encrypt_metadata: bool,
    pub encrypt_at_rest: bool,
    pub encrypt_in_transit: bool,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum EncryptionAlgorithm {
    AES256GCM,
    ChaCha20Poly1305,
    AES256CBC,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct KeyRotationConfig {
    pub rotation_interval_days: u32,
    pub auto_rotation: bool,
    pub backup_old_keys: bool,
}

// ===== NAMESPACE OPERATIONS =====

// Copy Namespace Request
#[derive(Debug, Deserialize, Validate)]
pub struct CopyNamespaceRequest {
    #[validate(length(min = 1, max = 255))]
    pub target_namespace: String,
    pub copy_options: CopyOptions,
    pub filters: Option<Vec<MetadataFilter>>,  // Copy only matching records
}

#[derive(Debug, Deserialize, Serialize)]
pub struct CopyOptions {
    pub copy_vectors: bool,
    pub copy_metadata: bool,
    pub copy_schema: bool,
    pub copy_configuration: bool,
    pub overwrite_existing: bool,
    pub batch_size: Option<usize>,
    pub parallel_copy: Option<bool>,
}

// ===== RESPONSE MODELS =====

#[derive(Debug, Serialize)]
pub struct OperationResponse {
    pub success: bool,
    pub affected_count: usize,
    pub operation_id: String,
    pub timestamp: DateTime<Utc>,
    pub errors: Option<Vec<OperationError>>,
    pub warnings: Option<Vec<String>>,
}

#[derive(Debug, Serialize)]
pub struct OperationError {
    pub id: Option<String>,
    pub error_code: String,
    pub message: String,
    pub details: Option<serde_json::Value>,
}

#[derive(Debug, Serialize)]
pub struct SchemaResponse {
    pub schema: NamespaceSchema,
    pub migration_status: Option<MigrationStatus>,
    pub validation_results: Option<ValidationResults>,
}

#[derive(Debug, Serialize)]
pub struct MigrationStatus {
    pub status: String,
    pub progress_percentage: f32,
    pub estimated_completion: Option<DateTime<Utc>>,
    pub errors: Vec<String>,
}

#[derive(Debug, Serialize)]
pub struct ValidationResults {
    pub valid_records: usize,
    pub invalid_records: usize,
    pub validation_errors: Vec<ValidationError>,
}

#[derive(Debug, Serialize)]
pub struct CopyNamespaceResponse {
    pub operation_id: String,
    pub status: CopyStatus,
    pub copied_records: usize,
    pub total_records: usize,
    pub progress_percentage: f32,
    pub estimated_completion: Option<DateTime<Utc>>,
}

#[derive(Debug, Serialize)]
pub enum CopyStatus {
    Started,
    InProgress,
    Completed,
    Failed,
    Cancelled,
}
```

**API Handler Implementations**:

```rust
use axum::{
    extract::{Path, Query, State},
    http::StatusCode,
    response::Json,
    Extension,
};
use std::sync::Arc;

// ===== Search API Handlers =====

// 1. Vector Similarity Search Handler
pub async fn vector_search_handler(
    Path(namespace): Path<String>,
    State(app_state): State<Arc<AppState>>,
    Extension(user): Extension<AuthenticatedUser>,
    Json(request): Json<VectorSearchRequest>,
) -> Result<Json<SearchResponse>, ApiError> {
    // Validate vector dimensions
    let index_config = app_state.get_namespace_config(&namespace).await?;
    if request.vector.len() != index_config.dimension {
        return Err(ApiError::InvalidVectorDimension);
    }
    
    // Perform vector search
    let search_results = app_state.search_engine
        .vector_search(&namespace, &request.vector, request.top_k)
        .await?;
    
    // Format response
    let response = SearchResponse {
        results: search_results.into_iter().map(|r| SearchResult {
            id: r.id,
            score: r.score,
            metadata: if request.include_metadata.unwrap_or(true) { r.metadata } else { None },
            vector: if request.include_vectors.unwrap_or(false) { r.vector } else { None },
            text_snippet: None,
            rank: r.rank,
        }).collect(),
        total_count: search_results.len(),
        search_time_ms: search_results.search_time_ms,
        metadata: SearchMetadata {
            search_type: "vector".to_string(),
            namespace: namespace.clone(),
            timestamp: Utc::now(),
            index_version: index_config.version,
            total_vectors: index_config.total_vectors,
            search_params: serde_json::to_value(&request)?,
        },
    };
    
    Ok(Json(response))
}

// 2. Text Search Handler
pub async fn text_search_handler(
    Path(namespace): Path<String>,
    State(app_state): State<Arc<AppState>>,
    Extension(user): Extension<AuthenticatedUser>,
    Json(request): Json<TextSearchRequest>,
) -> Result<Json<SearchResponse>, ApiError> {
    // Generate embedding from text
    let embedding = app_state.embedding_service
        .generate_embedding(&request.text, request.embedding_model.as_deref())
        .await?;
    
    // Convert to vector search
    let vector_request = VectorSearchRequest {
        vector: embedding,
        top_k: request.top_k,
        include_metadata: request.include_metadata,
        include_vectors: Some(false),
        distance_threshold: None,
    };
    
    // Perform search
    let search_results = app_state.search_engine
        .vector_search(&namespace, &vector_request.vector, vector_request.top_k)
        .await?;
    
    let response = SearchResponse {
        results: search_results.into_iter().map(|r| SearchResult {
            id: r.id,
            score: r.score,
            metadata: r.metadata,
            vector: None,
            text_snippet: extract_text_snippet(&r.metadata, &request.text),
            rank: r.rank,
        }).collect(),
        total_count: search_results.len(),
        search_time_ms: search_results.search_time_ms,
        metadata: SearchMetadata {
            search_type: "text".to_string(),
            namespace: namespace.clone(),
            timestamp: Utc::now(),
            index_version: 0,
            total_vectors: 0,
            search_params: serde_json::to_value(&request)?,
        },
    };
    
    Ok(Json(response))
}

// 3. Hybrid Search Handler
pub async fn hybrid_search_handler(
    Path(namespace): Path<String>,
    State(app_state): State<Arc<AppState>>,
    Extension(user): Extension<AuthenticatedUser>,
    Json(request): Json<HybridSearchRequest>,
) -> Result<Json<SearchResponse>, ApiError> {
    // Validate weights sum to 1.0
    if (request.vector_weight + request.bm25_weight - 1.0).abs() > 0.001 {
        return Err(ApiError::InvalidWeights("Vector and BM25 weights must sum to 1.0".to_string()));
    }
    
    // Generate vector if not provided
    let vector = match request.vector {
        Some(v) => v,
        None => app_state.embedding_service
            .generate_embedding(&request.text, None)
            .await?,
    };
    
    // Perform parallel searches
    let (vector_results, bm25_results) = tokio::try_join!(
        app_state.search_engine.vector_search(&namespace, &vector, request.top_k * 2),
        app_state.search_engine.bm25_search(&namespace, &request.text, request.top_k * 2)
    )?;
    
    // Fuse results based on fusion method
    let fused_results = match request.fusion_method.unwrap_or(FusionMethod::RRF) {
        FusionMethod::RRF => fuse_results_rrf(vector_results, bm25_results, request.vector_weight, request.bm25_weight),
        FusionMethod::WeightedSum => fuse_results_weighted_sum(vector_results, bm25_results, request.vector_weight, request.bm25_weight),
        FusionMethod::CombSUM => fuse_results_comb_sum(vector_results, bm25_results),
        FusionMethod::CombMNZ => fuse_results_comb_mnz(vector_results, bm25_results),
    };
    
    // Take top-k results
    let final_results: Vec<_> = fused_results.into_iter().take(request.top_k).collect();
    
    let response = SearchResponse {
        results: final_results.into_iter().map(|r| SearchResult {
            id: r.id,
            score: r.score,
            metadata: r.metadata,
            vector: None,
            text_snippet: extract_text_snippet(&r.metadata, &request.text),
            rank: r.rank,
        }).collect(),
        total_count: request.top_k,
        search_time_ms: 0, // Calculate actual time
        metadata: SearchMetadata {
            search_type: "hybrid".to_string(),
            namespace: namespace.clone(),
            timestamp: Utc::now(),
            index_version: 0,
            total_vectors: 0,
            search_params: serde_json::to_value(&request)?,
        },
    };
    
    Ok(Json(response))
}

// 4. BM25 Search Handler
pub async fn bm25_search_handler(
    Path(namespace): Path<String>,
    State(app_state): State<Arc<AppState>>,
    Extension(user): Extension<AuthenticatedUser>,
    Json(request): Json<BM25SearchRequest>,
) -> Result<Json<SearchResponse>, ApiError> {
    let search_results = app_state.search_engine
        .bm25_search_with_params(
            &namespace,
            &request.query,
            request.top_k,
            request.fields.as_ref(),
            request.boost_fields.as_ref(),
            request.bm25_params.as_ref(),
        )
        .await?;
    
    let response = SearchResponse {
        results: search_results.into_iter().map(|r| SearchResult {
            id: r.id,
            score: r.score,
            metadata: r.metadata,
            vector: None,
            text_snippet: extract_text_snippet(&r.metadata, &request.query),
            rank: r.rank,
        }).collect(),
        total_count: search_results.len(),
        search_time_ms: search_results.search_time_ms,
        metadata: SearchMetadata {
            search_type: "bm25".to_string(),
            namespace: namespace.clone(),
            timestamp: Utc::now(),
            index_version: 0,
            total_vectors: 0,
            search_params: serde_json::to_value(&request)?,
        },
    };
    
    Ok(Json(response))
}

// 5. Multi-Vector Search Handler
pub async fn multi_vector_search_handler(
    Path(namespace): Path<String>,
    State(app_state): State<Arc<AppState>>,
    Extension(user): Extension<AuthenticatedUser>,
    Json(request): Json<MultiVectorSearchRequest>,
) -> Result<Json<SearchResponse>, ApiError> {
    // Validate vector dimensions
    let index_config = app_state.get_namespace_config(&namespace).await?;
    for vector in &request.vectors {
        if vector.len() != index_config.dimension {
            return Err(ApiError::InvalidVectorDimension);
        }
    }
    
    // Perform searches for all vectors in parallel
    let search_futures: Vec<_> = request.vectors.iter()
        .map(|vector| app_state.search_engine.vector_search(&namespace, vector, request.top_k * 2))
        .collect();
    
    let all_results = futures::future::try_join_all(search_futures).await?;
    
    // Aggregate results based on method
    let aggregated_results = match request.aggregation_method {
        AggregationMethod::Average => aggregate_average(all_results, request.weights.as_ref()),
        AggregationMethod::Maximum => aggregate_maximum(all_results),
        AggregationMethod::Minimum => aggregate_minimum(all_results),
        AggregationMethod::WeightedSum => aggregate_weighted_sum(all_results, request.weights.as_ref()),
    };
    
    let final_results: Vec<_> = aggregated_results.into_iter().take(request.top_k).collect();
    
    let response = SearchResponse {
        results: final_results.into_iter().map(|r| SearchResult {
            id: r.id,
            score: r.score,
            metadata: r.metadata,
            vector: None,
            text_snippet: None,
            rank: r.rank,
        }).collect(),
        total_count: request.top_k,
        search_time_ms: 0,
        metadata: SearchMetadata {
            search_type: "multi_vector".to_string(),
            namespace: namespace.clone(),
            timestamp: Utc::now(),
            index_version: 0,
            total_vectors: 0,
            search_params: serde_json::to_value(&request)?,
        },
    };
    
    Ok(Json(response))
}

// 6. Filtered Search Handler
pub async fn filtered_search_handler(
    Path(namespace): Path<String>,
    State(app_state): State<Arc<AppState>>,
    Extension(user): Extension<AuthenticatedUser>,
    Json(request): Json<FilteredSearchRequest>,
) -> Result<Json<SearchResponse>, ApiError> {
    // Build filter query
    let filter_query = build_filter_query(&request.filters, request.filter_logic.unwrap_or(FilterLogic::And))?;
    
    // Perform search based on input type
    let search_results = if let Some(vector) = request.vector {
        app_state.search_engine
            .vector_search_with_filter(&namespace, &vector, request.top_k, &filter_query)
            .await?
    } else if let Some(text) = request.text {
        let embedding = app_state.embedding_service.generate_embedding(&text, None).await?;
        app_state.search_engine
            .vector_search_with_filter(&namespace, &embedding, request.top_k, &filter_query)
            .await?
    } else {
        return Err(ApiError::InvalidRequest("Either vector or text must be provided".to_string()));
    };
    
    let response = SearchResponse {
        results: search_results.into_iter().map(|r| SearchResult {
            id: r.id,
            score: r.score,
            metadata: r.metadata,
            vector: None,
            text_snippet: None,
            rank: r.rank,
        }).collect(),
        total_count: search_results.len(),
        search_time_ms: search_results.search_time_ms,
        metadata: SearchMetadata {
            search_type: "filtered".to_string(),
            namespace: namespace.clone(),
            timestamp: Utc::now(),
            index_version: 0,
            total_vectors: 0,
            search_params: serde_json::to_value(&request)?,
        },
    };
    
    Ok(Json(response))
}

// 7. Batch Search Handler
pub async fn batch_search_handler(
    Path(namespace): Path<String>,
    State(app_state): State<Arc<AppState>>,
    Extension(user): Extension<AuthenticatedUser>,
    Json(request): Json<BatchSearchRequest>,
) -> Result<Json<Vec<SearchResponse>>, ApiError> {
    let timeout = Duration::from_secs(request.timeout_seconds.unwrap_or(30));
    
    if request.parallel_execution.unwrap_or(true) {
        // Execute searches in parallel
        let search_futures: Vec<_> = request.queries.into_iter()
            .map(|query| execute_search_query(&app_state, &namespace, query))
            .collect();
        
        let results = tokio::time::timeout(timeout, futures::future::try_join_all(search_futures))
            .await
            .map_err(|_| ApiError::Timeout("Batch search timeout".to_string()))??;
        
        Ok(Json(results))
    } else {
        // Execute searches sequentially
        let mut results = Vec::new();
        for query in request.queries {
            let result = tokio::time::timeout(timeout, execute_search_query(&app_state, &namespace, query))
                .await
                .map_err(|_| ApiError::Timeout("Search query timeout".to_string()))??;
            results.push(result);
        }
        
        Ok(Json(results))
    }
}

// 8. Stream Search Handler (WebSocket)
pub async fn stream_search_handler(
    ws: WebSocketUpgrade,
    Path(namespace): Path<String>,
    State(app_state): State<Arc<AppState>>,
    Extension(user): Extension<AuthenticatedUser>,
    Query(params): Query<StreamSearchRequest>,
) -> Response {
    ws.on_upgrade(move |socket| handle_search_stream(socket, namespace, app_state, user, params))
}

async fn handle_search_stream(
    mut socket: WebSocket,
    namespace: String,
    app_state: Arc<AppState>,
    user: AuthenticatedUser,
    request: StreamSearchRequest,
) {
    let mut interval = tokio::time::interval(
        Duration::from_millis(request.stream_options.flush_interval_ms.unwrap_or(1000))
    );
    
    loop {
        tokio::select! {
            _ = interval.tick() => {
                // Execute search query
                match execute_search_query(&app_state, &namespace, request.search_config.clone()).await {
                    Ok(results) => {
                        let stream_response = StreamSearchResponse {
                            subscription_id: request.subscription_id.clone(),
                            results,
                            timestamp: Utc::now(),
                        };
                        
                        if socket.send(Message::Text(serde_json::to_string(&stream_response).unwrap())).await.is_err() {
                            break;
                        }
                    }
                    Err(e) => {
                        let error_response = StreamErrorResponse {
                            subscription_id: request.subscription_id.clone(),
                            error: e.to_string(),
                            timestamp: Utc::now(),
                        };
                        
                        if socket.send(Message::Text(serde_json::to_string(&error_response).unwrap())).await.is_err() {
                            break;
                        }
                    }
                }
            }
            msg = socket.recv() => {
                match msg {
                    Some(Ok(Message::Close(_))) => break,
                    Some(Err(_)) => break,
                    None => break,
                    _ => continue,
                }
            }
        }
    }
}
```

**Webhook Implementation**:

```rust
// Webhook Handler for Search Results
pub async fn search_webhook_handler(
    State(app_state): State<Arc<AppState>>,
    Extension(webhook_config): Extension<WebhookConfig>,
    Json(payload): Json<SearchWebhookPayload>,
) -> Result<StatusCode, ApiError> {
    // Validate webhook signature
    validate_webhook_signature(&webhook_config, &payload)?;
    
    // Process webhook payload
    match payload.status {
        SearchStatus::Completed => {
            // Handle successful search completion
            app_state.webhook_processor
                .process_search_completion(payload)
                .await?;
        }
        SearchStatus::Failed => {
            // Handle search failure
            app_state.webhook_processor
                .process_search_failure(payload)
                .await?;
        }
        _ => {
            // Handle other statuses
            app_state.webhook_processor
                .process_search_status_update(payload)
                .await?;
        }
    }
    
    Ok(StatusCode::OK)
}

// Webhook Handler for Search Status Updates
pub async fn search_status_webhook_handler(
    State(app_state): State<Arc<AppState>>,
    Extension(webhook_config): Extension<WebhookConfig>,
    Json(payload): Json<SearchStatusUpdate>,
) -> Result<StatusCode, ApiError> {
    // Validate webhook signature
    validate_webhook_signature(&webhook_config, &payload)?;
    
    // Update search status in database
    app_state.search_status_tracker
        .update_status(&payload.search_id, payload.status, payload.progress)
        .await?;
    
    // Notify subscribers if any
    app_state.notification_service
        .notify_search_status_update(&payload)
        .await?;
    
    Ok(StatusCode::OK)
}
```

**Authentication and Authorization**:
- JWT-based authentication with configurable providers
- Role-based access control (RBAC) for namespace operations
- API key authentication for service-to-service communication
- Rate limiting per user/API key with configurable limits
- Webhook signature validation for security

**Async Request Handling**:
- Tokio-based async runtime for optimal performance
- Connection pooling for database and external service connections
- Request timeout handling with configurable limits
- Graceful shutdown with connection draining
- WebSocket support for real-time streaming

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

## Post-Write Processing

### Overview

หลังจากการดำเนินการ write operations (upsert, update, delete) เสร็จสิ้น ระบบจะทำการ post-processing เพื่อให้แน่ใจว่าข้อมูลถูกจัดการอย่างสมบูรณ์:

```
┌─────────────────────────────────────────────────────────────────────┐
│                    Post-Write Processing                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. Success Response                                                │
│  {                                                                  │
│    "status": "ok",                                                  │
│    "inserted": 3,                                                   │
│    "updated": 0,                                                    │
│    "deleted": 0,                                                    │
│    "errors": []                                                     │
│  }                                                                  │
│                                                                     │
│  2. Async Index Building Trigger                                    │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│  │ WAL Monitor │────▶│Index Queue  │────▶│Index Builder│           │
│  │  (Polling)  │     │  (In-Memory)│     │  (Workers)  │            │
│  └─────────────┘     └─────────────┘     └─────────────┘            │
│                                                                     │
│  3. Cache Invalidation                                              │
│  - Invalidate affected namespace cache entries                      │
│  - Update namespace metadata (doc count, last write time)           │
│  - Trigger pre-warming if configured                                │
└─────────────────────────────────────────────────────────────────────┘
```

### Post-Write Processing Components

**1. Response Generation**:
```rust
#[derive(Debug, Serialize)]
pub struct WriteOperationResponse {
    pub status: String,
    pub inserted: usize,
    pub updated: usize,
    pub deleted: usize,
    pub errors: Vec<OperationError>,
    pub operation_id: String,
    pub processing_time_ms: u64,
    pub index_build_triggered: bool,
}
```

**2. Async Index Building**:
```rust
pub struct IndexBuildTrigger {
    wal_monitor: WALMonitor,
    index_queue: Arc<Mutex<VecDeque<IndexBuildTask>>>,
    workers: Vec<IndexBuilderWorker>,
}

impl IndexBuildTrigger {
    pub async fn trigger_index_build(&self, namespace: &str, operation_type: OperationType) {
        let task = IndexBuildTask {
            namespace: namespace.to_string(),
            operation_type,
            priority: self.calculate_priority(operation_type),
            created_at: Utc::now(),
        };
        
        self.index_queue.lock().await.push_back(task);
        self.notify_workers().await;
    }
}
```

**3. Cache Management**:
```rust
pub struct CacheInvalidator {
    namespace_cache: Arc<NamespaceCache>,
    search_cache: Arc<SearchResultCache>,
    metadata_cache: Arc<MetadataCache>,
}

impl CacheInvalidator {
    pub async fn invalidate_after_write(&self, namespace: &str, affected_ids: &[String]) {
        // Invalidate namespace-level cache
        self.namespace_cache.invalidate(namespace).await;
        
        // Invalidate search result cache for affected documents
        for id in affected_ids {
            self.search_cache.invalidate_by_document_id(id).await;
        }
        
        // Update namespace metadata
        self.update_namespace_metadata(namespace).await;
        
        // Trigger cache pre-warming if configured
        if self.should_prewarm(namespace).await {
            self.trigger_cache_prewarming(namespace).await;
        }
    }
}
```

## Webhook Interfaces

### Comprehensive Webhook System

ระบบ SPFresh รองรับ webhook interfaces ที่ครอบคลุมสำหรับการดำเนินการต่างๆ:

### 1. Upsert Operation Webhook

```rust
// Upsert Request Example
POST /v1/namespaces/product-catalog/upsert
{
  "upsert_rows": [
    {
      "id": "product-123",
      "text": "Wireless Smartphone Charger. Fast charging wireless pad for smartphones. Compatible with all Qi-enabled devices.",
      "metadata": {
        "title": "Wireless Smartphone Charger",
        "description": "Fast charging wireless pad for smartphones",
        "category": "electronics",
        "price": 29.99,
        "tags": ["wireless", "charging", "smartphone"],
        "in_stock": true,
        "created_at": "2025-06-15T10:30:00Z"
      }
    },
    {
      "id": "product-456",
      "text": "Bluetooth Headphones with noise cancellation. Over-ear design with premium audio quality and 20-hour battery life.",
      "metadata": {
        "title": "Bluetooth Headphones",
        "description": "Noise-cancelling over-ear headphones",
        "category": "electronics",
        "price": 89.99,
        "tags": ["bluetooth", "audio", "headphones"],
        "in_stock": true,
        "created_at": "2025-06-14T09:15:00Z"
      }
    }
  ]
}

// Response
{
  "status": "ok",
  "inserted": 2,
  "updated": 0,
  "errors": [],
  "operation_id": "op-2025-07-23-abcdef123456",
  "processing_time_ms": 45,
  "index_build_triggered": true
}
```

### 2. Update Operation Webhook

```rust
// Update Request Example
PATCH /v1/namespaces/product-catalog/update
{
  "patch_rows": [
    {
      "id": "product-123",
      "metadata": {
        "price": 24.99,
        "in_stock": false,
        "updated_at": "2025-07-23T14:25:00Z"
      }
    }
  ]
}

// Response
{
  "status": "ok",
  "inserted": 0,
  "updated": 1,
  "errors": [],
  "operation_id": "op-2025-07-23-ghijkl789012",
  "processing_time_ms": 23,
  "index_build_triggered": false
}
```

### 3. Conditional Upsert Operation Webhook

```rust
// Conditional Upsert Request Example
POST /v1/namespaces/product-catalog/conditional-upsert
{
  "upsert_rows": [
    {
      "id": "product-789",
      "text": "Smart Watch with fitness tracking features. Includes heart rate monitor, step counter, and sleep tracking.",
      "metadata": {
        "title": "Smart Watch",
        "description": "Fitness tracking smartwatch with heart rate monitor",
        "category": "wearables",
        "price": 149.99,
        "tags": ["smartwatch", "fitness", "wearable"],
        "in_stock": true
      }
    }
  ],
  "conditions": [
    {
      "field": "price",
      "operator": "GreaterThan",
      "value": 100
    }
  ]
}

// Response
{
  "status": "ok",
  "inserted": 1,
  "updated": 0,
  "condition_met": true,
  "errors": [],
  "operation_id": "op-2025-07-23-mnopqr345678",
  "processing_time_ms": 67
}
```

### 4. Batch Delete Operation Webhook

```rust
// Batch Delete Request Example
DELETE /v1/namespaces/product-catalog/batch-delete
{
  "deletes": ["product-123", "product-456"],
  "options": {
    "soft_delete": false,
    "backup_before_delete": true
  }
}

// Response
{
  "status": "ok",
  "deleted": 2,
  "errors": [],
  "operation_id": "op-2025-07-23-stuvwx901234",
  "processing_time_ms": 34,
  "backup_location": "s3://spfresh-backups/product-catalog/2025-07-23/"
}
```

### 5. Advanced Search Operations

#### Text-Based Search with Filtering

```rust
// Advanced Search Request Example
POST /v1/namespaces/product-catalog/search/text
{
  "text": "wireless charging for smartphones",
  "filters": [
    {
      "field": "category",
      "operator": "Equals",
      "value": "electronics"
    },
    {
      "field": "price",
      "operator": "LessThan",
      "value": 50
    },
    {
      "field": "in_stock",
      "operator": "Equals",
      "value": true
    }
  ],
  "filter_logic": "And",
  "top_k": 5,
  "include_metadata": true,
  "include_vectors": false,
  "preprocessing_options": {
    "normalize": true,
    "remove_stopwords": true,
    "language": "en"
  }
}

// Response
{
  "results": [
    {
      "id": "product-123",
      "score": 0.92,
      "rank": 1,
      "metadata": {
        "title": "Wireless Smartphone Charger",
        "description": "Fast charging wireless pad for smartphones",
        "category": "electronics",
        "price": 24.99,
        "tags": ["wireless", "charging", "smartphone"],
        "in_stock": true
      },
      "text_snippet": "Wireless Smartphone Charger. Fast charging wireless pad for smartphones..."
    }
  ],
  "total_count": 1,
  "search_time_ms": 15,
  "metadata": {
    "search_type": "text",
    "namespace": "product-catalog",
    "timestamp": "2025-07-23T14:30:00Z",
    "index_version": 42,
    "total_vectors": 10000,
    "search_params": {
      "text": "wireless charging for smartphones",
      "filters_applied": 3,
      "preprocessing_applied": true
    }
  }
}
```

#### Hybrid Search Operation

```rust
// Hybrid Search Request Example
POST /v1/namespaces/product-catalog/search/hybrid
{
  "text": "premium audio headphones",
  "vector_weight": 0.7,
  "bm25_weight": 0.3,
  "fusion_method": "RRF",
  "top_k": 10,
  "filters": [
    {
      "field": "category",
      "operator": "In",
      "value": ["electronics", "audio"]
    }
  ]
}

// Response
{
  "results": [
    {
      "id": "product-456",
      "score": 0.89,
      "rank": 1,
      "metadata": {
        "title": "Bluetooth Headphones",
        "description": "Noise-cancelling over-ear headphones",
        "category": "electronics",
        "price": 89.99,
        "tags": ["bluetooth", "audio", "headphones"],
        "in_stock": true
      },
      "text_snippet": "Bluetooth Headphones with noise cancellation. Over-ear design with premium audio quality..."
    }
  ],
  "total_count": 1,
  "search_time_ms": 28,
  "metadata": {
    "search_type": "hybrid",
    "namespace": "product-catalog",
    "timestamp": "2025-07-23T14:35:00Z",
    "fusion_method": "RRF",
    "vector_weight": 0.7,
    "bm25_weight": 0.3,
    "search_params": {
      "vector_search_results": 15,
      "bm25_search_results": 12,
      "fused_results": 10
    }
  }
}
```

#### Multi-Vector Search Operation

```rust
// Multi-Vector Search Request Example
POST /v1/namespaces/product-catalog/search/multi-vector
{
  "vectors": [
    [0.1, 0.2, 0.3, ...],  // Vector for "smartphone"
    [0.4, 0.5, 0.6, ...],  // Vector for "wireless"
    [0.7, 0.8, 0.9, ...]   // Vector for "charging"
  ],
  "weights": [0.5, 0.3, 0.2],
  "aggregation_method": "WeightedSum",
  "top_k": 5
}

// Response
{
  "results": [
    {
      "id": "product-123",
      "score": 0.94,
      "rank": 1,
      "metadata": {
        "title": "Wireless Smartphone Charger",
        "category": "electronics",
        "price": 24.99
      }
    }
  ],
  "total_count": 1,
  "search_time_ms": 42,
  "metadata": {
    "search_type": "multi_vector",
    "aggregation_method": "WeightedSum",
    "vectors_used": 3,
    "weights_applied": true
  }
}
```

### 6. Insert Webhook with Auto-Embedding

```rust
// Insert with Auto-Embedding Request
POST /v1/namespaces/product-catalog/upsert
{
  "upsert_rows": [
    {
      "id": "product-001",
      "text": "Red shoes with leather sole",
      "metadata": {
        "category": "footwear",
        "color": "red",
        "material": "leather",
        "score": 5
      }
    },
    {
      "id": "product-002", 
      "text": "Blue cotton t-shirt",
      "metadata": {
        "category": "clothing",
        "color": "blue",
        "material": "cotton",
        "score": 4
      }
    }
  ],
  "options": {
    "auto_generate_embedding": true,
    "embedding_model": "all-MiniLM-L6-v2",
    "conflict_resolution": "Replace"
  }
}

// Response
{
  "status": "ok",
  "inserted": 2,
  "updated": 0,
  "errors": [],
  "operation_id": "op-2025-07-23-xyz789",
  "processing_time_ms": 156,
  "embedding_generation": {
    "model_used": "all-MiniLM-L6-v2",
    "vectors_generated": 2,
    "embedding_time_ms": 89
  },
  "index_build_triggered": true
}
```

### 7. Real-time Stream Search Webhook

```rust
// WebSocket Stream Search Connection
GET /v1/namespaces/product-catalog/search/stream
Upgrade: websocket

// Stream Search Configuration
{
  "subscription_id": "stream-search-001",
  "search_config": {
    "type": "Text",
    "text": "smartphone accessories",
    "top_k": 3,
    "include_metadata": true
  },
  "stream_options": {
    "flush_interval_ms": 1000,
    "max_results_per_batch": 10,
    "buffer_size": 100
  }
}

// Streaming Response (sent every flush_interval_ms)
{
  "subscription_id": "stream-search-001",
  "results": [
    {
      "id": "product-123",
      "score": 0.92,
      "metadata": {
        "title": "Wireless Smartphone Charger",
        "price": 24.99,
        "in_stock": true
      }
    }
  ],
  "timestamp": "2025-07-23T14:40:00Z",
  "batch_number": 1,
  "total_results_in_batch": 1
}
```

### Webhook Security and Validation

```rust
pub struct WebhookValidator {
    secret_key: String,
    timeout_seconds: u64,
}

impl WebhookValidator {
    pub fn validate_signature(&self, payload: &str, signature: &str) -> Result<bool, WebhookError> {
        let expected_signature = self.generate_signature(payload)?;
        Ok(constant_time_eq(signature.as_bytes(), expected_signature.as_bytes()))
    }
    
    pub fn validate_timestamp(&self, timestamp: i64) -> Result<bool, WebhookError> {
        let current_time = Utc::now().timestamp();
        let age = current_time - timestamp;
        Ok(age <= self.timeout_seconds as i64)
    }
}
```

## SPFresh Warm Cache System

### Overview

Inspired by TurboPuffer's warm cache, we're implementing **SPFresh HotCache** - an intelligent, multi-tiered caching system designed specifically for high-performance vector search operations.

```
┌─────────────────────────────────────────────────────────────────────┐
│                      SPFresh HotCache Architecture                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  L1 Cache (Memory) - Ultra Hot Data                                │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│  │ Query Cache │     │Vector Cache │     │Index Cache  │            │
│  │ (Results)   │     │ (Embeddings)│     │ (Centroids) │            │
│  └─────────────┘     └─────────────┘     └─────────────┘            │
│                                                                     │
│  L2 Cache (SSD) - Warm Data                                        │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│  │Posting Lists│     │ Metadata    │     │ Freq. Vecs  │            │
│  │   (Recent)  │     │  (Popular)  │     │ (Accessed)  │            │
│  └─────────────┘     └─────────────┘     └─────────────┘            │
│                                                                     │
│  L3 Storage (Cloud/Disk) - Cold Data                               │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│  │Full Dataset │     │ Backups     │     │ Archives    │            │
│  │             │     │             │     │             │            │
│  └─────────────┘     └─────────────┘     └─────────────┘            │
└─────────────────────────────────────────────────────────────────────┘
```

### Core Components

#### 1. HotCache Manager

```rust
use std::sync::Arc;
use tokio::sync::RwLock;
use dashmap::DashMap;
use lru::LruCache;

pub struct SPFreshHotCache {
    // L1 Cache - Ultra Hot (Memory)
    query_cache: Arc<RwLock<LruCache<String, CachedSearchResult>>>,
    vector_cache: Arc<DashMap<String, CachedVector>>,
    index_cache: Arc<RwLock<CachedIndexSegments>>,
    
    // L2 Cache - Warm (SSD)
    ssd_cache: Arc<SSDCacheManager>,
    
    // Cache Intelligence
    access_tracker: Arc<AccessTracker>,
    cache_optimizer: Arc<CacheOptimizer>,
    
    // Configuration
    config: HotCacheConfig,
    metrics: Arc<CacheMetrics>,
}

#[derive(Debug, Clone)]
pub struct HotCacheConfig {
    // L1 Memory limits
    pub max_query_cache_size: usize,      // 1GB
    pub max_vector_cache_size: usize,     // 2GB  
    pub max_index_cache_size: usize,      // 4GB
    
    // L2 SSD limits
    pub max_ssd_cache_size: usize,        // 100GB
    
    // Intelligence settings
    pub warmup_threshold: f64,            // 0.8 (80% access frequency)
    pub eviction_policy: EvictionPolicy,
    pub preload_popular_queries: bool,
    pub adaptive_sizing: bool,
}

impl SPFreshHotCache {
    pub async fn new(config: HotCacheConfig) -> Self {
        Self {
            query_cache: Arc::new(RwLock::new(
                LruCache::new(config.max_query_cache_size)
            )),
            vector_cache: Arc::new(DashMap::new()),
            index_cache: Arc::new(RwLock::new(CachedIndexSegments::new())),
            ssd_cache: Arc::new(SSDCacheManager::new(&config).await),
            access_tracker: Arc::new(AccessTracker::new()),
            cache_optimizer: Arc::new(CacheOptimizer::new()),
            config,
            metrics: Arc::new(CacheMetrics::new()),
        }
    }
}
```

#### 2. Intelligent Cache Warming

```rust
pub struct CacheWarmer {
    cache: Arc<SPFreshHotCache>,
    analytics: Arc<QueryAnalytics>,
    scheduler: Arc<WarmupScheduler>,
}

impl CacheWarmer {
    // Proactive warming based on patterns
    pub async fn warm_popular_data(&self) -> Result<(), CacheError> {
        // 1. Identify hot queries from analytics
        let hot_queries = self.analytics.get_popular_queries(24).await?; // Last 24h
        
        // 2. Pre-execute and cache results
        for query in hot_queries {
            if !self.cache.query_cache.read().await.contains(&query.hash) {
                let result = self.execute_and_cache_query(query).await?;
                self.cache.store_query_result(&query.hash, result).await?;
            }
        }
        
        // 3. Warm frequently accessed vectors
        let hot_vectors = self.analytics.get_frequent_vectors(1000).await?;
        for vector_id in hot_vectors {
            self.cache.preload_vector(&vector_id).await?;
        }
        
        // 4. Cache popular index segments
        let hot_segments = self.analytics.get_active_index_segments().await?;
        for segment in hot_segments {
            self.cache.preload_index_segment(&segment).await?;
        }
        
        Ok(())
    }
    
    // Predictive warming based on time patterns
    pub async fn predictive_warm(&self) -> Result<(), CacheError> {
        let predictions = self.analytics.predict_next_hour_queries().await?;
        
        for predicted_query in predictions {
            // Pre-warm likely queries before they're requested
            if predicted_query.confidence > 0.7 {
                self.background_warm_query(predicted_query.query).await?;
            }
        }
        
        Ok(())
    }
}
```

#### 3. Multi-Level Cache Access

```rust
impl SPFreshHotCache {
    // Smart cache lookup with automatic promotion
    pub async fn get_search_results(&self, query_hash: &str) -> Option<CachedSearchResult> {
        // L1: Check query cache first (fastest)
        if let Some(result) = self.query_cache.read().await.get(query_hash) {
            self.metrics.record_l1_hit().await;
            return Some(result.clone());
        }
        
        // L2: Check SSD cache
        if let Some(result) = self.ssd_cache.get_query_result(query_hash).await {
            self.metrics.record_l2_hit().await;
            
            // Promote to L1 if frequently accessed
            if self.should_promote_to_l1(query_hash).await {
                self.query_cache.write().await.put(query_hash.to_string(), result.clone());
            }
            
            return Some(result);
        }
        
        // L3: Cache miss - will need to compute
        self.metrics.record_cache_miss().await;
        None
    }
    
    // Intelligent vector retrieval
    pub async fn get_vector(&self, vector_id: &str) -> Option<Vec<f32>> {
        // L1: Memory cache
        if let Some(cached_vec) = self.vector_cache.get(vector_id) {
            self.access_tracker.record_access(vector_id).await;
            return Some(cached_vec.vector.clone());
        }
        
        // L2: SSD cache
        if let Some(vector) = self.ssd_cache.get_vector(vector_id).await {
            // Promote to L1 if hot
            if self.access_tracker.is_hot(vector_id).await {
                self.vector_cache.insert(
                    vector_id.to_string(),
                    CachedVector {
                        vector: vector.clone(),
                        last_accessed: Utc::now(),
                        access_count: self.access_tracker.get_count(vector_id).await,
                    }
                );
            }
            return Some(vector);
        }
        
        None
    }
}
```

#### 4. Adaptive Cache Management

```rust
pub struct CacheOptimizer {
    memory_monitor: MemoryMonitor,
    performance_tracker: PerformanceTracker,
}

impl CacheOptimizer {
    // Dynamic cache sizing based on system resources
    pub async fn optimize_cache_sizes(&self, cache: &SPFreshHotCache) -> Result<(), CacheError> {
        let system_memory = self.memory_monitor.available_memory().await;
        let cache_hit_rates = cache.metrics.get_hit_rates().await;
        
        // Adjust L1 cache sizes based on hit rates and memory pressure
        if system_memory.pressure > 0.8 {
            // High memory pressure - reduce cache sizes
            self.shrink_caches(cache, 0.8).await?;
        } else if cache_hit_rates.overall < 0.6 {
            // Low hit rate - increase cache sizes if memory allows
            self.expand_caches(cache, 1.2).await?;
        }
        
        // Rebalance between different cache types
        self.rebalance_cache_allocation(cache).await?;
        
        Ok(())
    }
    
    // Smart eviction based on access patterns
    pub async fn smart_eviction(&self, cache: &SPFreshHotCache) -> Result<(), CacheError> {
        // Custom eviction beyond simple LRU
        let candidates = self.identify_eviction_candidates(cache).await?;
        
        for candidate in candidates {
            match candidate.cache_type {
                CacheType::Query => {
                    // Demote to L2 instead of evicting completely
                    if let Some(result) = cache.query_cache.write().await.pop(&candidate.key) {
                        cache.ssd_cache.store_query_result(&candidate.key, result).await?;
                    }
                }
                CacheType::Vector => {
                    // Archive frequently accessed vectors to SSD
                    if candidate.access_frequency > 10 {
                        if let Some((_, cached_vec)) = cache.vector_cache.remove(&candidate.key) {
                            cache.ssd_cache.store_vector(&candidate.key, cached_vec.vector).await?;
                        }
                    } else {
                        // Completely evict rarely used vectors
                        cache.vector_cache.remove(&candidate.key);
                    }
                }
                CacheType::Index => {
                    // Move index segments to SSD
                    cache.demote_index_segment(&candidate.key).await?;
                }
            }
        }
        
        Ok(())
    }
}
```

#### 5. Cache Analytics and Monitoring

```rust
pub struct CacheMetrics {
    // Hit rates
    l1_hits: Arc<AtomicU64>,
    l2_hits: Arc<AtomicU64>,
    cache_misses: Arc<AtomicU64>,
    
    // Performance metrics
    avg_query_time: Arc<RwLock<f64>>,
    cache_memory_usage: Arc<AtomicU64>,
    
    // Access patterns
    hot_data_tracker: Arc<DashMap<String, AccessPattern>>,
}

impl CacheMetrics {
    pub async fn get_cache_efficiency_report(&self) -> CacheEfficiencyReport {
        let total_requests = self.l1_hits.load(Ordering::Relaxed) + 
                           self.l2_hits.load(Ordering::Relaxed) + 
                           self.cache_misses.load(Ordering::Relaxed);
        
        CacheEfficiencyReport {
            overall_hit_rate: (self.l1_hits.load(Ordering::Relaxed) + 
                              self.l2_hits.load(Ordering::Relaxed)) as f64 / total_requests as f64,
            l1_hit_rate: self.l1_hits.load(Ordering::Relaxed) as f64 / total_requests as f64,
            l2_hit_rate: self.l2_hits.load(Ordering::Relaxed) as f64 / total_requests as f64,
            avg_response_time: *self.avg_query_time.read().await,
            memory_efficiency: self.calculate_memory_efficiency().await,
            top_hot_queries: self.get_hottest_queries(10).await,
            cache_size_recommendations: self.recommend_cache_sizes().await,
        }
    }
}
```

#### 6. Integration with SPFresh Core

```rust
// Integration point in search handlers
pub async fn vector_search_with_cache(
    cache: &SPFreshHotCache,
    spfresh_core: &SPFreshCore,
    query: &VectorSearchRequest,
) -> Result<SearchResponse, SearchError> {
    let query_hash = calculate_query_hash(query);
    
    // Try cache first
    if let Some(cached_result) = cache.get_search_results(&query_hash).await {
        // Verify cache freshness
        if cached_result.is_fresh(&cache.config.ttl) {
            cache.metrics.record_cache_hit().await;
            return Ok(cached_result.into_response());
        }
    }
    
    // Cache miss - execute search
    let start_time = Instant::now();
    let search_result = spfresh_core.vector_search(query).await?;
    let search_time = start_time.elapsed();
    
    // Cache the result
    let cached_result = CachedSearchResult {
        results: search_result.clone(),
        cached_at: Utc::now(),
        search_time_ms: search_time.as_millis() as u64,
        query_hash: query_hash.clone(),
    };
    
    cache.store_search_result(&query_hash, cached_result).await?;
    
    Ok(search_result)
}
```

### Cache Configuration Examples

```rust
// Production configuration
let production_config = HotCacheConfig {
    max_query_cache_size: 1_000_000,      // 1M queries
    max_vector_cache_size: 10_000_000,    // 10M vectors  
    max_index_cache_size: 1000,           // 1K index segments
    max_ssd_cache_size: 100 * 1024 * 1024 * 1024, // 100GB
    warmup_threshold: 0.8,
    eviction_policy: EvictionPolicy::SmartLRU,
    preload_popular_queries: true,
    adaptive_sizing: true,
};

// Development configuration
let dev_config = HotCacheConfig {
    max_query_cache_size: 10_000,
    max_vector_cache_size: 100_000,
    max_index_cache_size: 100,
    max_ssd_cache_size: 1024 * 1024 * 1024, // 1GB
    warmup_threshold: 0.5,
    eviction_policy: EvictionPolicy::LRU,
    preload_popular_queries: false,
    adaptive_sizing: false,
};
```

## SPFresh HotCache Integration Architecture

### Warm Cache Data Flow Integration

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    SPFresh HotCache Integration Flow                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  Client Request                                                             │
│       │                                                                     │
│       ▼                                                                     │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      API Layer (Rust/Axum)                          │    │
│  │  • Parse request                                                    │    │
│  │  • Generate cache key                                               │    │
│  │  • Check request type (read/write)                                  │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                   SPFresh HotCache Layer                            │    │
│  │                                                                     │    │
│  │  L1 Cache (Memory) - Ultra Hot                                      │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐                    │    │
│  │  │Query Results│ │Vector Cache │ │Index Segments│                   │    │
│  │  │(1GB LRU)    │ │(2GB HashMap)│ │(4GB Segments)│                   │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘                    │    │
│  │         │ Hit           │ Hit           │ Hit                        │    │
│  │         ▼               ▼               ▼                            │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              Cache Hit Response                              │    │    │
│  │  │  • Return cached data                                       │    │    │
│  │  │  • Update access metrics                                    │    │    │
│  │  │  • Record cache hit                                         │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  │                                                                     │    │
│  │  Cache Miss ▼                                                       │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                L2 Cache (SSD) - Warm                        │    │    │
│  │  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐            │    │    │
│  │  │  │Posting Lists│ │Hot Metadata │ │Freq Vectors │            │    │    │
│  │  │  │(Recent)     │ │(Popular)    │ │(Accessed)   │            │    │    │
│  │  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘            │    │    │
│  │  │         │ Hit           │ Hit           │ Hit                │    │    │
│  │  │         ▼               ▼               ▼                    │    │    │
│  │  │  ┌─────────────────────────────────────────────────────┐    │    │    │
│  │  │  │          Promote to L1 + Return Data               │    │    │    │
│  │  │  └─────────────────────────────────────────────────────┘    │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  │                                                                     │    │
│  │  Cache Miss ▼                                                       │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              L3 Storage (Cold) - Compute                    │    │    │
│  │  │  • Execute query on SPFresh Core                            │    │    │
│  │  │  • Store result in appropriate cache level                  │    │    │
│  │  │  • Update cache analytics                                   │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                            │                                                │
│                            ▼                                                │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    SPFresh Core Processing                           │    │
│  │  • SPANN Index Layer                                                │    │
│  │  • SPTAG Base Layer                                                 │    │
│  │  • Storage Layer (RocksDB/Cloud)                                    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Cache Warming Strategy Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      SPFresh Cache Warming Architecture                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Analytics & Intelligence Layer                    │    │
│  │                                                                     │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐    │    │
│  │  │Query Pattern│ │Access Freq  │ │Time Pattern │ │ML Predictor │    │    │
│  │  │Analyzer     │ │Tracker      │ │Detector     │ │Engine       │    │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘    │    │
│  │         │               │               │               │           │    │
│  │         └───────────────┼───────────────┼───────────────┘           │    │
│  │                         │               │                           │    │
│  │                         ▼               ▼                           │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              Warming Decision Engine                        │    │    │
│  │  │  • Identify hot data candidates                             │    │    │
│  │  │  • Predict future access patterns                          │    │    │
│  │  │  • Calculate warming priorities                             │    │    │
│  │  │  • Schedule warming tasks                                   │    │    │
│  │  └─────────────────────────┬───────────────────────────────────┘    │    │
│  └─────────────────────────────┼───────────────────────────────────────┘    │
│                                │                                            │
│                                ▼                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      Cache Warming Scheduler                        │    │
│  │                                                                     │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐    │    │
│  │  │Proactive    │ │Predictive   │ │Time-based   │ │Event-driven │    │    │
│  │  │Warming      │ │Warming      │ │Warming      │ │Warming      │    │    │
│  │  │             │ │             │ │             │ │             │    │    │
│  │  │• Popular    │ │• ML-based   │ │• Peak hours │ │• New data   │    │    │
│  │  │  queries    │ │  prediction │ │• Scheduled  │ │• Index      │    │    │
│  │  │• Hot vectors│ │• Pattern    │ │  warming    │ │  updates    │    │    │
│  │  │• Freq index │ │  matching   │ │• Maintenance│ │• Schema     │    │    │
│  │  │  segments   │ │• Confidence │ │  windows    │ │  changes    │    │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘    │    │
│  │         │               │               │               │           │    │
│  │         └───────────────┼───────────────┼───────────────┘           │    │
│  │                         │               │                           │    │
│  │                         ▼               ▼                           │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │                Warming Task Queue                           │    │    │
│  │  │  Priority Queue: High → Medium → Low                        │    │    │
│  │  │  • Task scheduling                                          │    │    │
│  │  │  • Resource allocation                                      │    │    │
│  │  │  • Conflict resolution                                      │    │    │
│  │  └─────────────────────────┬───────────────────────────────────┘    │    │
│  └─────────────────────────────┼───────────────────────────────────────┘    │
│                                │                                            │
│                                ▼                                            │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Warming Execution Workers                        │    │
│  │                                                                     │    │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐    │    │
│  │  │Worker 1     │ │Worker 2     │ │Worker 3     │ │Worker N     │    │    │
│  │  │             │ │             │ │             │ │             │    │    │
│  │  │• Execute    │ │• Execute    │ │• Execute    │ │• Execute    │    │    │
│  │  │  queries    │ │  queries    │ │  queries    │ │  queries    │    │    │
│  │  │• Load data  │ │• Load data  │ │• Load data  │ │• Load data  │    │    │
│  │  │• Update     │ │• Update     │ │• Update     │ │• Update     │    │    │
│  │  │  cache      │ │  cache      │ │  cache      │ │  cache      │    │    │
│  │  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘    │    │
│  │         │               │               │               │           │    │
│  │         └───────────────┼───────────────┼───────────────┘           │    │
│  │                         │               │                           │    │
│  │                         ▼               ▼                           │    │
│  │  ┌─────────────────────────────────────────────────────────────┐    │    │
│  │  │              Cache Population Results                       │    │    │
│  │  │  • Success/failure tracking                                 │    │    │
│  │  │  • Performance metrics                                      │    │    │
│  │  │  • Cache hit rate improvements                              │    │    │
│  │  └─────────────────────────────────────────────────────────────┘    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Regional Cache Distribution Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                   SPFresh Regional HotCache Distribution                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Global Cache Coordinator                         │    │
│  │  • Cross-region cache synchronization                               │    │
│  │  • Global hot data identification                                   │    │
│  │  • Cache invalidation coordination                                  │    │
│  │  • Regional cache health monitoring                                 │    │
│  └─────────────────────────┬───────────────────────────────────────────┘    │
│                            │                                                │
│            ┌───────────────┼───────────────┐                               │
│            │               │               │                               │
│            ▼               ▼               ▼                               │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐               │
│  │  GCP Region     │ │  AWS Region     │ │ Azure Region    │               │
│  │ asia-southeast1 │ │   us-east-1     │ │   eastasia      │               │
│  ├─────────────────┤ ├─────────────────┤ ├─────────────────┤               │
│  │                 │ │                 │ │                 │               │
│  │ ┌─────────────┐ │ │ ┌─────────────┐ │ │ ┌─────────────┐ │               │
│  │ │Regional     │ │ │ │Regional     │ │ │ │Regional     │ │               │
│  │ │Cache Manager│ │ │ │Cache Manager│ │ │ │Cache Manager│ │               │
│  │ └──────┬──────┘ │ │ └──────┬──────┘ │ │ └──────┬──────┘ │               │
│  │        │        │ │        │        │ │        │        │               │
│  │        ▼        │ │        ▼        │ │        ▼        │               │
│  │ ┌─────────────┐ │ │ ┌─────────────┐ │ │ ┌─────────────┐ │               │
│  │ │L1: Memory   │ │ │ │L1: Memory   │ │ │ │L1: Memory   │ │               │
│  │ │• 8GB Query  │ │ │ │• 8GB Query  │ │ │ │• 8GB Query  │ │               │
│  │ │• 16GB Vector│ │ │ │• 16GB Vector│ │ │ │• 16GB Vector│ │               │
│  │ │• 32GB Index │ │ │ │• 32GB Index │ │ │ │• 32GB Index │ │               │
│  │ └──────┬──────┘ │ │ └──────┬──────┘ │ │ └──────┬──────┘ │               │
│  │        │        │ │        │        │ │        │        │               │
│  │        ▼        │ │        ▼        │ │        ▼        │               │
│  │ ┌─────────────┐ │ │ ┌─────────────┐ │ │ ┌─────────────┐ │               │
│  │ │L2: SSD      │ │ │ │L2: SSD      │ │ │ │L2: SSD      │ │               │
│  │ │• 500GB Warm │ │ │ │• 500GB Warm │ │ │ │• 500GB Warm │ │               │
│  │ │• NVMe PCIe4 │ │ │ │• NVMe PCIe4 │ │ │ │• NVMe PCIe4 │ │               │
│  │ │• 100K IOPS  │ │ │ │• 100K IOPS  │ │ │ │• 100K IOPS  │ │               │
│  │ └──────┬──────┘ │ │ └──────┬──────┘ │ │ └──────┬──────┘ │               │
│  │        │        │ │        │        │ │        │        │               │
│  │        ▼        │ │        ▼        │ │        ▼        │               │
│  │ ┌─────────────┐ │ │ ┌─────────────┐ │ │ ┌─────────────┐ │               │
│  │ │L3: Cloud    │ │ │ │L3: Cloud    │ │ │ │L3: Cloud    │ │               │
│  │ │• GCS        │ │ │ │• S3         │ │ │ │• Blob       │ │               │
│  │ │• Unlimited  │ │ │ │• Unlimited  │ │ │ │• Unlimited  │ │               │
│  │ │• Cold Data  │ │ │ │• Cold Data  │ │ │ │• Cold Data  │ │               │
│  │ └─────────────┘ │ │ └─────────────┘ │ │ └─────────────┘ │               │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘               │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                    Cache Synchronization Flow                       │    │
│  │                                                                     │    │
│  │  Hot Data Identification:                                           │    │
│  │  Region A ──────────────▶ Global Coordinator ◀────────────── Region C │    │
│  │                                    ▲                                │    │
│  │                                    │                                │    │
│  │                              Region B                               │    │
│  │                                                                     │    │
│  │  Cache Warming Propagation:                                         │    │
│  │  Global Coordinator ──────▶ All Regions (Async)                     │    │
│  │                                                                     │    │
│  │  Invalidation Broadcast:                                            │    │
│  │  Write Region ──────▶ Global Coordinator ──────▶ All Other Regions  │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Cache Performance Monitoring Dashboard

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                     SPFresh HotCache Performance Dashboard                   │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                        Cache Hit Rates                              │    │
│  │                                                                     │    │
│  │  L1 Cache (Memory):     ████████████████████████ 95.2%              │    │
│  │  L2 Cache (SSD):        ████████████████████     87.8%              │    │
│  │  L3 Cache (Cloud):      ████████████             65.4%              │    │
│  │  Overall Hit Rate:      ████████████████████████ 91.7%              │    │
│  │                                                                     │    │
│  │  Cache Miss Rate:       ████                     8.3%               │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      Response Time Metrics                          │    │
│  │                                                                     │    │
│  │  L1 Hit Response:       0.1ms   ████████████████████████████████    │    │
│  │  L2 Hit Response:       2.5ms   ████████████████████████████████    │    │
│  │  L3 Hit Response:       15ms    ████████████████████████████████    │    │
│  │  Cache Miss:            45ms    ████████████████████████████████    │    │
│  │                                                                     │    │
│  │  Average Response:      3.2ms   ████████████████████████████████    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                       Memory Utilization                            │    │
│  │                                                                     │    │
│  │  L1 Query Cache:        ████████████████████     7.2GB / 8GB        │    │
│  │  L1 Vector Cache:       ████████████████████████ 14.8GB / 16GB      │    │
│  │  L1 Index Cache:        ████████████████████████ 28.5GB / 32GB      │    │
│  │                                                                     │    │
│  │  L2 SSD Usage:          ████████████████████████ 420GB / 500GB      │    │
│  │  L3 Cloud Usage:        ████████████████████████ 2.1TB / ∞          │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                      Cache Operations/sec                           │    │
│  │                                                                     │    │
│  │  Cache Reads:           ████████████████████████ 8,500 ops/sec      │    │
│  │  Cache Writes:          ████████████████████     3,200 ops/sec      │    │
│  │  Cache Evictions:       ████████                 1,100 ops/sec      │    │
│  │  Cache Promotions:      ████████████             2,400 ops/sec      │    │
│  │                                                                     │    │
│  │  Warming Tasks:         ████████████████████████ 450 tasks/min      │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                        Top Hot Queries                              │    │
│  │                                                                     │    │
│  │  1. "smartphone accessories"     ████████████████████ 2,341 hits    │    │
│  │  2. "wireless charging"          ████████████████████ 1,987 hits    │    │
│  │  3. "bluetooth headphones"       ████████████████████ 1,654 hits    │    │
│  │  4. "laptop computers"           ████████████████████ 1,432 hits    │    │
│  │  5. "gaming peripherals"         ████████████████████ 1,298 hits    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                     Regional Cache Status                           │    │
│  │                                                                     │    │
│  │  GCP asia-southeast1:   ████████████████████████ Healthy (91.2%)    │    │
│  │  AWS us-east-1:         ████████████████████████ Healthy (93.7%)    │    │
│  │  Azure eastasia:        ████████████████████████ Healthy (89.4%)    │    │
│  │                                                                     │    │
│  │  Sync Status:           ████████████████████████ All regions synced │    │
│  │  Last Sync:             2 minutes ago                               │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

This comprehensive design provides a solid foundation for implementing the modernized SPFresh architecture with all the required layers, components, and features while maintaining high performance, scalability, and reliability. The **SPFresh HotCache** system is now fully integrated with TurboPuffer-inspired warm cache capabilities, multi-tiered caching, intelligent warming strategies, and comprehensive monito