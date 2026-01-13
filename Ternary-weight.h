ternary_weights.h

Purpose
This header defines the kernel-side “weight artifact” format and the only allowed interface for loading model weights into an untrusted compute rail (Rail 3). It is not an AI runtime API. It is an enforcement contract: what a weight blob looks like, how it is verified, how it is mapped, and what metadata Bay-0/Airlock can trust without parsing “model logic.”

Design goals

Deterministic: fixed structs, fixed sizes where possible, bounded parsing.

Capability-friendly: loader can validate then map pages; callers never get raw storage authority.

Restart-safe: supports hinge cycles and rail restarts without “learning state” persistence.

Vendor-neutral: does not assume a specific NPU; describes memory regions and copy plan.

Hash-first: content addressed, chainable, and auditable.


What it is not

Not a tokenizer, sampler, or inference graph interface.

Not a model format like GGUF; this is a minimal “sealed container” wrapper suitable for a sovereign kernel lane.

Not a cloud/telemetry format.


How it’s used (flow)

1. Vault (or a trusted packager tool) produces a sealed weight bundle: header + manifest + shards.


2. Airlock/Bay-0 validates the bundle header and manifest hashes (constant time-ish loops, bounded).


3. Rail-3 loader receives only: a handle/capability to the validated bundle and a mapping plan.


4. Rail-3 copies/maps shards into its private memory/NPU window.


5. On hinge reset: Rail-3 dies, volatile weight staging dies, Vault bundle remains immutable.



Copy-paste header (drop-in)

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
  Ternary Weights Container
  ------------------------
  Kernel-first, enforcement-oriented.
  The container is validated in trusted plane (State -1 / Bay-0).
  Rail-3 only receives a capability/handle after validation.
*/

/* FourCC helpers */
#define TW_FOURCC(a,b,c,d) ((uint32_t)(a) | ((uint32_t)(b)<<8) | ((uint32_t)(c)<<16) | ((uint32_t)(d)<<24))

/* Magic/version */
#define TW_MAGIC   TW_FOURCC('T','W','G','T')   /* "TWGT" */
#define TW_VERSION 0x0001

/* Hash sizes */
#define TW_HASH_LEN_32 32   /* SHA-256 or BLAKE3-256 */
#define TW_HASH_LEN_64 64   /* SHA-512 or BLAKE3-512 if you choose */

/* Limits (keep bounded for kernel parsing) */
#define TW_MAX_SHARDS     4096
#define TW_MAX_MANIFEST   (64 * 1024)   /* manifest upper bound */
#define TW_MAX_NAME       32
#define TW_MAX_COPY_OPS   8192

/* Target architectures / rails (informational, not trust) */
typedef enum {
    TW_RAIL_UNKNOWN = 0,
    TW_RAIL_1_SENSOR = 1,
    TW_RAIL_2_MEDIA  = 2,
    TW_RAIL_3_REASON = 3
} tw_rail_t;

/* Quantization tags (informational; validation is by hash, not by trusting these fields) */
typedef enum {
    TW_Q_UNKNOWN = 0,
    TW_Q_BITNET_158B = 1, /* BitNet 1.58b style (ternary-ish weights) */
    TW_Q_INT8  = 2,
    TW_Q_INT4  = 3,
    TW_Q_FP16  = 4
} tw_quant_t;

/* Weight storage type */
typedef enum {
    TW_STORE_UNKNOWN = 0,
    TW_STORE_UFS     = 1,
    TW_STORE_NVME    = 2,
    TW_STORE_MRAM    = 3,
    TW_STORE_SPI_NOR = 4
} tw_store_t;

/* Validation policy */
typedef enum {
    TW_VALIDATE_NONE = 0,     /* dev only */
    TW_VALIDATE_HEADER = 1,   /* header checks only */
    TW_VALIDATE_MANIFEST = 2, /* header + manifest hash */
    TW_VALIDATE_FULL = 3      /* header + manifest + all shard hashes */
} tw_validate_t;

/*
  Container layout (on disk / in Vault):
    [tw_container_hdr_t]
    [manifest bytes ...]  (size = manifest_size)
    [shard data ...]      (each shard referenced by manifest)
*/

/* Fixed header. Keep this small and stable. */
typedef struct __attribute__((packed)) {
    uint32_t magic;              /* TW_MAGIC */
    uint16_t version;            /* TW_VERSION */
    uint16_t header_size;        /* sizeof(tw_container_hdr_t) */
    uint32_t flags;              /* future use */
    uint64_t created_unix_ns;    /* provenance */
    uint32_t manifest_size;      /* bytes immediately following header */
    uint32_t shard_count;        /* number of tw_shard_desc_t entries in manifest */
    uint32_t manifest_hash_alg;  /* e.g., 1=SHA256, 2=BLAKE3 */
    uint8_t  manifest_hash[TW_HASH_LEN_32]; /* hash(manifest bytes) */

    /* Informational identity (not trusted without hash) */
    char     model_name[TW_MAX_NAME];  /* "bitnet_158b" etc */
    uint32_t quant;               /* tw_quant_t */
    uint32_t logical_rail;        /* intended rail (usually TW_RAIL_3_REASON) */
    uint32_t storage_kind;        /* tw_store_t */
    uint32_t reserved0;

    /* Full container hash (optional but useful for audit logs) */
    uint32_t container_hash_alg;  /* same enum as above */
    uint8_t  container_hash[TW_HASH_LEN_32]; /* hash(header+manifest+shards) or 0s if unused */
} tw_container_hdr_t;

/*
  Manifest entries.
  Manifest is a packed array of tw_shard_desc_t of length shard_count.
  This is what Bay-0 validates (bounded loop).
*/
typedef struct __attribute__((packed)) {
    uint32_t shard_id;           /* sequential or content-defined */
    uint32_t shard_type;         /* 0=weights,1=tokenizer,2=kv_init,3=aux */
    uint64_t offset;             /* byte offset from start of container (0 = header start) */
    uint64_t size;               /* shard byte size */
    uint32_t hash_alg;           /* 1=SHA256, 2=BLAKE3 */
    uint8_t  hash[TW_HASH_LEN_32]; /* hash(shard bytes) */
} tw_shard_desc_t;

/*
  Mapping / copy plan produced by trusted plane after validation.
  This prevents Rail-3 from “inventing” where to write.
*/
typedef enum {
    TW_OP_COPY = 0,     /* copy bytes into target */
    TW_OP_MAP  = 1,     /* map pages into target (if supported) */
    TW_OP_ZERO = 2      /* zero target range */
} tw_op_kind_t;

typedef struct __attribute__((packed)) {
    uint32_t kind;        /* tw_op_kind_t */
    uint32_t shard_id;    /* which shard */
    uint64_t src_off;     /* offset within shard */
    uint64_t dst_pa;      /* target physical address OR device window PA */
    uint64_t len;         /* bytes */
    uint32_t dst_attr;    /* cache/device attrs (implementation-defined) */
    uint32_t reserved;
} tw_copy_op_t;

/* Result codes for loader/enforcer */
typedef enum {
    TW_OK = 0,
    TW_ERR_BAD_MAGIC = 1,
    TW_ERR_BAD_VERSION = 2,
    TW_ERR_BAD_HEADER = 3,
    TW_ERR_MANIFEST_TOO_BIG = 4,
    TW_ERR_SHARD_COUNT = 5,
    TW_ERR_HASH_MISMATCH = 6,
    TW_ERR_BOUNDS = 7,
    TW_ERR_POLICY = 8,
    TW_ERR_UNSUPPORTED = 9
} tw_status_t;

/*
  Trusted-plane functions (State -1 / Bay-0 side)
  These are interfaces; implementation lives in your trusted PD.
*/

/* Validate container header + manifest + shards under a strict policy. */
tw_status_t tw_validate_container(const tw_container_hdr_t *hdr,
                                  const uint8_t *manifest_bytes,
                                  uint32_t manifest_size,
                                  const uint8_t *container_base,
                                  uint64_t container_size,
                                  tw_validate_t policy);

/* Build a bounded copy/map plan for Rail-3, based on validated manifest. */
tw_status_t tw_build_copy_plan(const tw_shard_desc_t *shards,
                               uint32_t shard_count,
                               tw_copy_op_t *out_ops,
                               uint32_t *inout_op_count,
                               uint64_t rail3_weight_window_pa,
                               uint64_t rail3_weight_window_len);

/*
  Untrusted rail function (Rail-3)
  Consumes only the plan; never parses the container.
*/
tw_status_t tw_execute_copy_plan(const tw_copy_op_t *ops,
                                uint32_t op_count,
                                const uint8_t *container_base,
                                uint64_t container_size);

#ifdef __cplusplus
}
#endif

Notes you should put right above the header in the repo (short, but decisive)

The trusted plane validates hashes; the rail does not.

The rail never receives direct access to storage. It receives a validated pointer/handle and a copy plan.

The manifest is bounded. If it exceeds TW_MAX_MANIFEST or shard_count exceeds TW_MAX_SHARDS, reject.

All offsets are bounds-checked against container_size before any copy/map.

Hash algorithm enum is fixed and only allows what you ship (start with SHA-256, upgrade to BLAKE3 later).
