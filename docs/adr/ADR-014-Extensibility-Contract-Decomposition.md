# ADR-014: Extensibility — Contract Decomposition

## Status

Accepted

**Opened:** August 7th 2026

**Closed:** August 8th 2026


## Context

[`decisions-map.md`](../architecture/decisions-map.md) (Extensibility) opens three questions about how a user adds a new component to the pipeline. This ADR decides the first: how many contracts the extension surface exposes. Inside that question `decisions-map.md` names three cut points: whether acquisition and deserialization are one contract or two, whether cutting the byte stream into record-sized chunks is part of deserialization or a responsibility of its own, and whether serialization and delivery are one contract or two.

The starting point is the five responsibilities of [`conceptual-model.md`](../architecture/conceptual-model.md), Stage 3: acquisition, deserialization, processing, serialization, delivery. Each may map one-to-one to a contract, fuse with a neighbour, or split into several.

A contract is the form the pipeline requires from a component: what it receives from the pipeline and what it returns to it. What the component does internally, and which origin, destination or format it handles, is not part of the contract.

Two criteria guided the decision. First, **asymmetric reuse**: if a user can reasonably want to reuse the library's half of a responsibility and provide the other half themselves, the two halves must be separate contracts. Fusing them forces the user to reimplement code the library already ships. Second, **input/output shape**: two operations belong to the same contract only if they receive the same kind of input and return the same kind of output. Operations that differ on either side are separate contracts even if they sit under the same conceptual responsibility.

## Decision

The extension surface exposes eight contracts:

| Responsibility | Contracts |
|---|---|
| Acquisition | `BytesAcquisition` |
| Deserialization | `RecordDelimitation`, `RecordParsing` |
| Processing | `RecordValidation`, `RecordFiltering`, `RecordTransformation` |
| Serialization | `RecordSerialization` |
| Delivery | `BytesDelivery` |

**Acquisition, serialization, delivery** map one-to-one to their responsibility.

**Deserialization splits into two contracts.** A user with an exotic record boundary (multiline log entries, for example) can reuse the library's parser for the payload of each entry and provide only the boundary detection. If deserialization were one contract, the user would have to reimplement the parser too. `RecordDelimitation` finds record boundaries in the byte stream; `RecordParsing` interprets the bytes of a delimited record as a `Record` (see [ADR-003](ADR-003-Data-Model-Record-Contract.md)).

**Processing splits into three contracts.** Validation, filtering and transformation differ on their output. Filtering returns a yes/no that decides whether the record moves forward or is silently discarded. Validation returns a yes/no that decides whether the record has passed a domain rule; a failed validation is a record error (see [`error-scenarios.md`](../architecture/error-scenarios.md), Processing, and [ADR-008](ADR-008-Error-Model-Taxonomy.md)). Transformation returns a record, possibly modified. Three output shapes, three contracts.

**Serialization and delivery stay separate.** Symmetrical to acquisition and deserialization: a user who wants to write JSON (library-provided format) to S3 (user-provided destination) writes only the delivery side and reuses `RecordSerialization`. Fusing them would force the user to reimplement the serializer too.

### Alternatives rejected

**Any structure that keeps deserialization as one contract.** Rejected on asymmetric reuse: a user with an exotic record boundary has to reimplement the parser to change the delimiter. This rules out one-contract-per-responsibility (five contracts) and every variant that keeps deserialization undivided.

**Any structure that keeps processing as one contract.** Rejected on input/output shape: validation, filtering and transformation do not share the same output, so they cannot sit under one contract.

**Fuse acquisition and deserialization** (bytes-in, records-out as a single contract). Rejected on asymmetric reuse: a user who wants to read a supported format from an unsupported origin (a CSV from S3) has to reimplement the CSV parser.

**Fuse serialization and delivery** (records-in, bytes-at-destination as a single contract). Rejected on asymmetric reuse: a user who wants to write a supported format to an unsupported destination (JSON to S3) has to reimplement the JSON serializer.

## Consequences

- The extension surface has eight contracts. The library ships a component per format or per origin/destination it supports; the user writes components against the same eight contracts to extend the library.
- The shape of each contract (which operations, how end-of-stream is signalled, whether lifecycle operations are needed) is not decided here.
- The C++ mechanism that realises each contract (inheritance, concept, type erasure, other) is not decided here.
- The `Record` type flowing through the record-side contracts is the one decided in [ADR-001](ADR-001-Data-Model-Value-Types.md), [ADR-002](ADR-002-Data-Model-Value-Representation-Mechanism.md) and [ADR-003](ADR-003-Data-Model-Record-Contract.md).