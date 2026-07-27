# ADR-003: Data Model — Record Contract

## Status

Accepted

**Opened:** July 27th 2026

**Closed:** July 27th 2026

## Context

This ADR decides how field names map to values inside a record. A record is a set of named fields (see [`conceptual-model.md`](../architecture/conceptual-model.md), Stage 3); [ADR-001](ADR-001-Data-Model-Value-Types.md) decides which categories a value can hold and [ADR-002](ADR-002-Data-Model-Value-Representation-Mechanism.md) decides the C++ mechanism that represents it. This is the last piece of the data model: the name-to-`Value` mapping.

What follows defines the properties a record must have. A spike raised two questions where the input formats disagree on how a set of fields behaves.

First, name uniqueness. No format guarantees that field names are unique within a record. JSON keys SHOULD be unique but the grammar admits duplicates, and a receiver's behavior on a duplicated name is left undefined ([RFC 8259, section 4](https://www.rfc-editor.org/rfc/rfc8259#section-4)). CSV places no restriction on duplicated header names. YAML requires unique keys in its spec but parsers vary in practice. So a duplicate name can arrive, and the record cannot pretend it is impossible.

Second, field order. CSV is positional: a cell is identified by its position, with the first row counted as row 1 ([W3C — Tabular Data Primer](https://www.w3.org/TR/tabular-data-primer/)). A JSON object, by grammar, is an unordered set of name-value pairs. So one format encodes field order as meaningful and another treats it as incidental.

## Decision

A record is an ordered associative mapping from unique string names to `Value`s.

Three properties define it:

**A name maps to at most one value.** Field names are unique within a record. A duplicate name is malformed input, handed to the error model (a separate decision, not yet made) rather than stored. This keeps access by name unambiguous: a lookup returns one value or nothing.

**Field order is preserved.** The record keeps the order in which deserialization delivers the fields (their insertion order) and does not reorder them. For CSV this is the header position; for JSON it is the textual order of appearance. This lets serialization reproduce the field order the source carried.

**A field name is a string.** CSV headers, JSON keys and log field names are all text. A name is not a `Value`: a `Value` can be composite (an object or an array), a name never is. The name is the key of the mapping, a plain string; the `Value` is what it maps to.

These properties govern every name-to-`Value` mapping, not only the top-level record. A composite `object` ([ADR-001](ADR-001-Data-Model-Value-Types.md)) is the same name-to-`Value` mapping in a nested position, so the same contract applies.

### Alternatives rejected

**Allow several values per name.** A multimap-style container would preserve every duplicate the format carried. Rejected: access by name would return "possibly several values" for every consumer and every user-written component, a cost paid on all records even though duplicates are rare and malformed. RFC 8259 treats a repeated name as undefined receiver behavior, not as valid structure, so admitting it into the model propagates that ambiguity. Quality attribute 3 (make common mistakes hard, keep the common case simple; see [`architectural-drivers.md`](../architecture/architectural-drivers.md)) favors the unambiguous mapping.

**Unordered mapping.** Dropping order is simpler, but CSV encodes field order positionally, and losing it prevents serialization from reproducing the source field order. Preserving insertion order is a structural property: it does not require interpreting the meaning of the data, which the scope excludes. Fidelity to the source wins over the simplification.

## Consequences

- A duplicate field name cannot be stored in a record. Handling it (skip, error, route) belongs to the error model, which is not yet decided.
- The concrete C++ container is left open, constrained only by the contract: it must provide unique keys and preserve insertion order.