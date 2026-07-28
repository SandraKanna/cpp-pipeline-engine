# ADR-004: Data Model — Numeric Value Representation

## Status

Accepted

**Opened:** July 28th 2026

**Closed:** July 28th 2026

## Context

[ADR-001](ADR-001-Data-Model-Value-Types.md) fixes `number` as one of the value categories a field can hold. [ADR-002](ADR-002-Data-Model-Value-Representation-Mechanism.md) fixes `std::variant` as the mechanism. This ADR decides which C++ type represents `number` inside that variant.

It is the only category that needs a decision. The rest map directly: `string` to `std::string`, `boolean` to `bool`, `null` to the variant's empty state, and the composites to standard containers. `number` is different because JSON has a single `number` type while C++ has several numeric types with different precision and range. One has to be chosen, and the choice is visible to anyone using the variant.

[RFC 8259, section 6](https://www.rfc-editor.org/rfc/rfc8259#section-6) is the primary source. JSON accepts numbers of any size, but the specification lets an implementation cap the precision and range it supports, and names IEEE 754 binary64 (double precision) as the level that gives good interoperability. Numbers beyond it, like `1E400` or a thirty-digit decimal, are flagged as interoperability problems.

The tool does no arithmetic on values ([`requirements.md`](../architecture/requirements.md), Scope): it reads them in and writes them back. The concern is therefore round-trip fidelity of a value from input to output, not the correctness of any computation on it.

## Decision

`number` is represented as a single `double` (IEEE 754 binary64).

The model guarantees `double`'s precision and range. A number within that range comes out exactly as it went in. A number beyond it (an integer above 2^53, a value too large for `double`, a decimal with too many digits) is approximated to the nearest `double`. Choosing `double` is not a shortfall from the JSON standard: RFC 8259 promises nothing above binary64.

### Alternatives rejected

**Two numeric types (`std::int64_t` and `double`).** This keeps large whole numbers exact, such as 64-bit identifiers or nanosecond timestamps, which `double` rounds above 2^53. Rejected: it puts a second numeric type in the public `Value`, so every reader has to check integer or floating-point on every number, for exactness the tool does not need since it never does arithmetic. RFC 8259 caps at `double` anyway. Quality attribute 3, keep the common case simple (see [`architectural-drivers.md`](../architecture/architectural-drivers.md)), favors one type.

**Arbitrary precision (a big-number type, or the raw text kept as a string).** This keeps every number exact. Rejected: it makes every numeric value heavier to preserve precision RFC 8259 does not even promise.

## Consequences

- `number` is one type (`double`). Every reader handles it the same way.
- To keep a large whole number exact (a 64-bit identifier, a nanosecond timestamp), send it as a `string`. A bare number beyond `double`'s range is approximated.
- `Infinity` and `NaN` cannot come from JSON: RFC 8259 forbids them. Other sources are not decided here.
- Adding an exact integer type later means adding a type to `Value`, which breaks the contract, the same as adding any value kind ([ADR-002](ADR-002-Data-Model-Value-Representation-Mechanism.md)).