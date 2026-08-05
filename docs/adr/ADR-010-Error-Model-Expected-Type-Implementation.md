# ADR-010: Error Model — `expected<T, E>` implementation in C++20

## Status

Accepted

**Opened:** August 4th 2026

**Closed:** August 5th 2026

## Context

[ADR-009](ADR-009-Error-Model-Representation.md) represents each expected error class as `expected<T, E>`. `std::expected` entered the standard in C++23; the project targets C++20 ([`requirements.md`](../architecture/requirements.md), Constraints), where the pattern is not part of the standard library. This ADR decides how it is provided until the project moves to C++23.

Two options:

- A custom implementation inside the library.
- An external library that mirrors `std::expected`'s API.

The choice uses two criteria:

1. **Cost of maintenance.** How much code the project owns and has to keep correct.
2. **Cost of removal at the C++23 migration.** How much of the codebase changes when `std::expected` from the standard library replaces the current implementation. Two dimensions: replacing the dependency itself (namespace, include), and refactoring the code that uses it (member function names, if the external library's API does not match the final `std::expected` API).

## Decision

`expected<T, E>` is provided by [`expected-lite`](https://github.com/martinmoene/expected-lite), a header-only library that implements the [`P0323` proposal](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0323r12.html) which was standardized as `std::expected` in C++23. Its member function names (`.transform()`, `.and_then()`, `.or_else()`) match the ones adopted by the standard.

Both criteria favor it: the project owns no code, and at the C++23 migration the only changes needed are the dependency and the namespace under which the type is imported.

### Alternatives rejected

**Custom implementation.** A correct implementation of the pattern is not trivial. Rejected on the maintenance criterion: the project would own hundreds of lines of subtle code with no advantage over a well-established external one.

**[`tl::expected`](https://github.com/TartanLlama/expected).** Also a header-only library that implements the pattern for C++ before C++23. Rejected on the second criterion: it implements an earlier draft of the proposal ([`P0323R3`, 2017](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0323r3.pdf)), and the C++23 committee later adopted different names for some of its member functions. Its `.map()` and `.map_error()` correspond to `std::expected`'s `.transform()` and `.transform_error()`, which means migrating to C++23 requires refactoring every call site that uses those methods, not only the dependency itself.

**[`p-ranav/result`](https://github.com/p-ranav/result).** A C++20 result type inspired by Rust's `Result<T>`. Rejected on the second criterion: its API is not a mirror of `std::expected`; migrating to C++23 would require refactoring every call site.

## Consequences

- The library gains a build-time dependency on `expected-lite`. The integration mechanism (CMake `FetchContent`, a package manager, a submodule, or another) is a dependency management decision, not decided here.
- When the project moves to C++23, the dependency is removed and `nonstd::expected` is replaced by `std::expected`: a change of include and of namespace only.
- Whether `nonstd::expected` is exposed directly in the public API or hidden behind a project-local alias is a decision on API surface, not decided here.
