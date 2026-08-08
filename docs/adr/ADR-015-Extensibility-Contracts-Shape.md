# ADR-015: Extensibility — Contract Shape

## Status

Accepted

**Opened:** August 8th 2026

**Closed:** August 8th 2026

## Context

[ADR-014](ADR-014-Extensibility-Contract-Decomposition.md) fixes the eight contracts the extension surface exposes. This ADR decides the shape of those contracts at the logical level: which operations the user implements, how end of stream is signalled, whether operations for setup or teardown belong to the contract. The physical level (exact C++ signatures) is not decided here.

Two criteria guided the decision:

1. **Cost of an operation to the contract.** Every operation a contract exposes is one every component implementing that contract has to provide. An operation only some components need is paid by all of them.
2. **Overlap with the language.** Setting up and releasing the resources an object owns is already handled by C++ constructors and destructors. A contract-level operation for the same purpose competes with a mechanism the language provides for free.

## Decision

**One operation per contract, input unit to output unit**

Each contract exposes one operation. The unit of input and output is what tells the eight contracts apart.

| Contract | Input from the engine | Output to the engine |
|---|---|---|
| `BytesAcquisition` | call to produce bytes | bytes |
| `RecordDelimitation` | bytes | zero or more delimited records, as bytes |
| `RecordParsing` | bytes of one delimited record | one `Record` |
| `RecordValidation` | one `Record` | pass or fail |
| `RecordFiltering` | one `Record` | keep or drop |
| `RecordTransformation` | one `Record` | one `Record` |
| `RecordSerialization` | one `Record` | bytes |
| `BytesDelivery` | bytes | nothing (side effect: the bytes reach the destination) |

The table shows what tells the contracts apart, not everything an operation returns. Two outcomes are decided in other ADRs and left out here: any operation can fail (the failure is a record or pipeline error, per [ADR-008](ADR-008-Error-Model-Taxonomy.md) and [ADR-009](ADR-009-Error-Model-Representation.md)), and the two contracts the engine calls repeatedly can reach end of stream (see below).

**End of stream signalled in the response of the operation**

Only the contracts the engine calls repeatedly to produce more can reach a natural end: `BytesAcquisition` and `RecordDelimitation`. The rest either take one unit and return one unit (parsing, validation, filtering, transformation, serialization), or consume without producing (`BytesDelivery`). For them, end of stream propagates from upstream: once the producing contract signals it, the engine stops calling the ones after it.

For the two contracts that can reach an end, the signal travels in the response of the same operation, the shape iterators already use in C++: the call that hands back the next unit is the same call that can report there is none. Returning zero bytes, as in most byte-stream APIs, signals end of stream.

**No operations for setup or teardown in the contract**

The contract contains no `open()`, `close()`, or equivalent. Each component sets up its resources in its constructor and releases them in its destructor. A contract-level operation for the same purpose adds nothing on top of the language mechanism, and forces two-phase construction: the object exists after the constructor but is not usable until the extra call runs, a state neither the constructor nor the destructor accounts for.

### Alternatives rejected

**Multiple operations per contract**, for example one operation to read data and another to query how many bytes are still available, or a `has_next()` alongside the exchange. Rejected on criterion 1: it costs every component an operation only some of them need. A separate `has_next()` also opens a gap, since the stream can end between the query and the next exchange.

**Signalling end of stream through an exception.** Rejected: [ADR-009](ADR-009-Error-Model-Representation.md) reserves exceptions for actual failures, and end of stream is a normal outcome (see [`error-scenarios.md`](../architecture/error-scenarios.md), Vocabulary).

**Contract-level operations for setup and teardown** (an `open()` and a `close()` on every contract). Rejected on criterion 2: constructors and destructors already handle those responsibilities.

## Consequences

- Every extension contract exposes one operation. Its physical form is not decided here.
- `RecordDelimitation` may yield several delimited records, one, or none in a single call, depending on how much the bytes it receives cover. How it holds bytes across calls is internal to it, not part of the contract.
- Component lifecycle stays inside each component. The engine constructs, uses, and destroys components; it does not participate in setting them up or tearing them down.
- Resources a component acquires during its lifetime, not at construction, are not covered by the contract; managing them is internal to the component.
