# ADR-009: Error Model — Representation (Modeling)

## Status

Accepted

**Opened:** August 4th 2026

**Closed:** August 4th 2026

## Context

[ADR-008](ADR-008-Error-Model-Taxonomy.md) recognizes four classes of error, three of which are expected outcomes the caller can handle: configuration error, pipeline error and record error. This ADR decides how those three classes are represented in the C++ type system.

The mechanisms C++20 offers to signal a failure from a function are: exceptions, return codes (an enum plus an out-parameter), `std::optional<T>`, and `expected<T, E>`, a return type that carries either the value on success or an error on failure. `std::expected` entered the standard in C++23; under the C++20 constraint ([`requirements.md`](../architecture/requirements.md), Constraints) the pattern is not part of the standard library. How it is provided is decided in a separate ADR.

The choice of the mechanism uses two criteria:

1. **Visibility in the signature.** A mechanism that puts the failure in the return type forces the caller to acknowledge it. One that reports the failure through a separate channel (an exception thrown up the call stack, a status variable set aside) does not: if the code ignores the failure, it will compile without warning.
2. **Cost on the expected path.** How often the failure happens turns cost into a real factor. A configuration error happens once at startup: the cost of the mechanism is paid once. A record error can happen on many of the records the pipeline processes: the cost is paid on each one.

## Decision

Each of the three expected classes is represented as `expected<T, E>`, with `E` identifying the class:

- Configuration error is carried by `expected<T, ConfigError>`.
- Pipeline error is carried by `expected<T, PipelineError>`.
- Record error is carried by `expected<T, RecordError>`.

`T` is a placeholder for the success value of the operation. Which operations exist, and what each returns, is out of scope here.

### Alternatives rejected

**Exceptions.** Rejected on both criteria. Exceptions are invisible in the signature: a caller who forgets to catch one gets a terminated process instead of a value they are forced to check. And on the expected path, throwing pays the stack-unwinding cost every time it occurs. For record error, where a bad row in a large file is a normal outcome, that cost would be paid on records that are not exceptional. The [C++ Core Guidelines on error handling](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#s-errors) name the same principle: throw for what a function cannot do, not for what its caller can reasonably handle. Phil Nash's [CppCon 2024 talk on modern C++ error handling](https://www.youtube.com/watch?v=_EaCzkDPgbg) makes the point about cost: when a failure is a habitual outcome the caller must react to, exceptions destroy performance, and types like `std::expected` fit better.

**Return codes (an enum plus an out-parameter).** Rejected on the first criterion. The failure is in the return type, but the caller is not forced to check it: an ignored return value compiles without warning, and any `nodiscard` protection is opt-in per function. Return codes also force the caller to construct the output object before knowing whether it is valid, which does not compose with move-only types (a `Pipeline` that owns its stages, for example).

**`std::optional<T>`.** Rejected on a different ground. `optional` is visible in the return type and cheap, but signals only whether there is a value, without telling the caller what failed. The three error types (`ConfigError`, `PipelineError`, `RecordError`) exist precisely to carry that information.

## Consequences

- The public operations that can fail with a configuration, pipeline, or record error return `expected<T, E>`. The mapping from operation to class is decided in separate ADRs.
- The concrete implementation of `expected<T, E>` in C++20 is decided in ADR-010. When the project moves to C++23, `std::expected` from the standard library replaces it and the include changes; no other code is affected.
