# ADR-008: Error Model — Taxonomy

## Status

Accepted

**Opened:** August 3rd 2026

**Closed:** August 4th 2026


## Context

[`decisions-map.md`](../architecture/decisions-map.md) (Errors) opens three questions about how the library treats failures: which classes of error the model recognizes (taxonomy), how each class is represented in the type system (modeling), and what runtime behavior each class triggers (handling). This ADR decides the first. [`error-scenarios.md`](../architecture/error-scenarios.md) lists every failure the library can meet, grouped by stage plus a transversal section for failures that can arise anywhere. It does not classify them; this ADR does.

Two properties separate those failures. **Scope**: a failure either blocks the pipeline, or is local to one record and leaves the next record independent of it. **Nature**: a failure is either expected (real-world input or configuration disagreeing with what a stage assumed) or a bug (a broken invariant inside the library or a component). The caller can handle expected failures but not bugs, because the process state can no longer be trusted.

## Decision

The error model recognizes four classes:

**Configuration error.** Expected, blocks the pipeline, detected during setup. The user's configuration is invalid and no data has flowed yet.

**Pipeline error.** Expected, blocks the pipeline, detected at runtime. Execution cannot continue: the next record cannot be produced or trusted.

**Record error.** Expected, local to one record, detected at runtime. The record is lost or unusable; the next record is independent.

**Bug.** A broken invariant inside the library or a component. Falls under the "bug" pole of the Nature axis, not among the three classes the caller handles.

### Alternatives rejected

**Merge Bug into Pipeline error.** Both prevent the pipeline from producing more output, so at first glance they look like the same class. Rejected: they differ on the Nature axis. A bug is not an expected outcome, so it cannot be modeled and reported the same way as a Pipeline error. This matches the standard C++ distinction between expected failures (reported) and programming errors (aborted), documented by [Microsoft's guidance on modern C++ error handling](https://learn.microsoft.com/en-us/cpp/cpp/errors-and-exception-handling-modern-cpp?view=msvc-170), by Phil Nash's [CppCon 2024 talk on modern C++ error handling](https://www.youtube.com/watch?v=_EaCzkDPgbg), and by the [C++ Core Guidelines on error handling](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#s-errors).

**Merge Configuration error into Pipeline error.** Both are expected failures that block execution. Rejected: they arise at different moments (setup, before any bytes flow, versus runtime), produced by different code paths (library-only validation versus library plus user components). Callers naturally distinguish the two moments.


## Mapping

Every failure listed in [`error-scenarios.md`](../architecture/error-scenarios.md) maps to exactly one class.

| Section | Failure | Class |
|---|---|---|
| Setup | Missing required stage | Configuration error |
| Setup | Unknown stage type | Configuration error |
| Setup | Invalid stage parameters | Configuration error |
| Setup | Unreadable configuration input | Configuration error |
| Setup | Incompatible pipeline composition | Configuration error |
| Acquisition | Origin not found | Pipeline error |
| Acquisition | Access denied | Pipeline error |
| Acquisition | Origin unreachable | Pipeline error |
| Acquisition | Read failure mid-stream | Pipeline error |
| Acquisition | Origin exhausted before expected | Pipeline error |
| Deserialization | Malformed input | Record error |
| Deserialization | Truncated input | Record error or Pipeline error (see below) |
| Deserialization | Duplicate field name in a record | Record error |
| Deserialization | Unrepresentable value | Record error |
| Processing | Missing required field | Record error |
| Processing | Wrong value type | Record error |
| Processing | Value out of range | Record error |
| Processing | Failed validation rule | Record error |
| Processing | User processing step signals a domain-specific failure | Record error |
| Serialization | Value not expressible in the target format | Record error |
| Serialization | Missing field required by the target format | Record error |
| Serialization | Encoding incompatibility | Record error |
| Delivery | Destination not found | Pipeline error |
| Delivery | Access denied | Pipeline error |
| Delivery | Destination unreachable | Pipeline error |
| Delivery | Write failure mid-stream | Pipeline error |
| Delivery | Destination capacity exceeded | Pipeline error |
| Delivery | Destination rejects the payload | Pipeline error |
| Transversal | Resource exhaustion | Pipeline error |
| Transversal | Broken invariant inside a component | Bug |
| Transversal | Unexpected failure escaping from a component | Bug |

**Truncated input** is the one failure whose class depends on the format. [`error-scenarios.md`](../architecture/error-scenarios.md) gives two examples: a CSV row cut mid-field, where the row is lost but the next row is independent (Record error), and a JSON object without its closing brace, where the parser cannot resynchronize with the byte stream and subsequent records cannot be trusted (Pipeline error). The distinction is whether the format's grammar lets the parser recover to the next record boundary.

## Consequences

- Adding a new failure to [`error-scenarios.md`](../architecture/error-scenarios.md) means placing it in one of the four classes.
- Modeling and handling, each decided in a separate ADR, build on these classes: two failures in the same class share their representation and their runtime behavior.
- Whether the runtime behavior of a class is global or configurable per component belongs to handling.