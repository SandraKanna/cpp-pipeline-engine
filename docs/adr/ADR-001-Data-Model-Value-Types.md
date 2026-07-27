# ADR-001: Data Model — Value Types

## Status

Accepted

**Opened:** July 23rd 2026

**Closed:** July 27th 2026

## Context

A record is a set of named fields, each holding a value (see [`conceptual-model.md`](../architecture/conceptual-model.md), Stage 3). This ADR defines which set of value categories a field can hold, the question the rest of the data model builds on. The value exists only while it lives inside the application as part of a record; before and after, the data is bytes, outside this decision's reach.

[`requirements.md`](../architecture/requirements.md) (Scope) admits structured data: a format qualifies when its data is a set of named fields. The set of qualifying formats is open and does not need to be enumerated here.

A spike raised two questions. First, where the set of value categories comes from, given that formats disagree on typing:

- **CSV** and **logs** carry no type in their grammar: every field is text (`42` and `forty two` are the same type in the file).
- **JSON** encodes type in its grammar: `42` (number) and `"42"` (string) are lexically distinct, as are `true`/`false` and `null`.

Second, whether a value that is itself a nested structure (a JSON object or array) is kept as such, or flattened into scalar fields at deserialization. Flattening converts a nested structure into flat scalar fields (`{"coords":{"lat":40.4}}` becomes a field `coords.lat`). 

Prior art from this domain: Kafka Connect offers flattening as an optional Single Message Transform ([Confluent — Flatten SMT](https://docs.confluent.io/kafka-connectors/transforms/current/flatten.html)), separate from its data model, and does not flatten arrays.

## Decision

A `Value` is one of the categories the [JSON grammar](https://www.json.org/json-en.html) defines for a value:

- **Scalars** (hold no further values): `string`, `number`, `boolean` (the `true`/`false` values), `null`.
- **Composites** (hold further values): `object`, accessed by name, and `array`, accessed by position.

Two points from the spike justify this set.

The categories come from JSON. A typed format determines what a value can be; JSON does that and covers the full set. Other typed formats (structured YAML, for example) add nothing new, and text formats like CSV add nothing at all.

Composites are included because nested structure is kept, not flattened. The JSON grammar allows nesting, which makes `Value` recursive: its base case is a scalar (the chain ends), its recursive case is a composite (the chain continues). Flattening would lose the shape of the source document, so any flattening stays outside the data model.

The set is stated in domain terms, independent of any C++ type. How each category is represented in C++, and whether `std::variant` can hold them all, is the mechanism decision. How a record holds those values is decided later.

### Alternatives rejected

**Flatten to scalars.** A flat model would flatten at deserialization, leaving `Value` with scalars only. Rejected: it discards structure the format carried, and rebuilding the original nesting from field names is ambiguous (a delimiter inside a name cannot be told from real nesting).

**Text only (every value a string).** Rejected for the reason given above in Context: a single string type collapses the distinctions JSON encodes in its grammar (`42` vs `"42"`), losing information the format carried.

## Consequences

- `Value` is a recursive type: a composite holds further values, and the recursion ends at scalars. The mechanism decision must support this.
- No C++ types are defined here. Mapping each category to a concrete representation is left to the mechanism and physical levels.
- Adding a typed format later does not reopen this set; it reuses the same categories.