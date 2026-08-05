# ADR-011: Error Model — Bug Termination

## Status

Accepted

**Opened:** August 5th 2026

**Closed:** August 5th 2026

## Context

[ADR-008](ADR-008-Error-Model-Taxonomy.md) recognizes Bug as an error class that terminates the process rather than being reported to the caller. This ADR decides how the process terminates.

Two failures from [`error-scenarios.md`](../architecture/error-scenarios.md) (Transversal) map to Bug, and they arise in different places. A **broken invariant** is a state the library's own code recognizes as violating an assumption it was written under. An **unexpected exception** is one thrown by a component whose contract says it will not throw. The first is detected by the code that reaches the invalid state; the second escapes from the component that produced it.

C++20 offers several ways to end a process:

- [`std::abort`](https://en.cppreference.com/w/cpp/utility/program/abort) ends it immediately.
- [`std::exit`](https://en.cppreference.com/w/cpp/utility/program/exit) ends it after running cleanup registered with `atexit`.
- [`assert`](https://en.cppreference.com/w/cpp/error/assert) ends it only when `NDEBUG` is not defined.
- [`std::terminate`](https://en.cppreference.com/w/cpp/error/terminate) ends it after running a handler that the program can install with [`std::set_terminate`](https://en.cppreference.com/w/cpp/error/set_terminate).

One further language feature is relevant to the second failure: [`noexcept`](https://en.cppreference.com/w/cpp/language/noexcept_spec) marks a function as non-throwing, and any exception that tries to escape it triggers `std::terminate`. So an exception escaping a component can reach `std::terminate` through the language itself, without the library having to catch it.

The choice is guided by two requirements. A bug must be diagnosable: a silent crash leaves nothing to investigate, so the mechanism must let the library emit a diagnostic before the process ends. And it must fire in the production build, since a bug the mechanism only catches during development is not guarded against where it matters.

## Decision

A bug terminates the process through `std::terminate`, with a diagnostic handler installed via `std::set_terminate`. Both failures reach the same call:

- **Broken invariant.** The library calls `std::terminate` directly at the point it detects the invalid state.
- **Unexpected exception.** Marking a function `noexcept` turns any exception that tries to escape it into a call to `std::terminate`.

### Alternatives rejected

**`std::abort` directly.** Rejected on the diagnosability requirement: it runs no handler, so the failure leaves no diagnostic.

**`std::exit`.** Rejected on the diagnosability requirement too: it runs cleanup registered with `atexit`, but not the terminate handler, so it leaves no diagnostic for the same reason as `abort`. It also runs that cleanup on a broken state, which is unsafe.

**`assert` as the production mechanism.** Rejected on the production-build requirement: it is compiled out when `NDEBUG` is defined, so it would not fire in the release build where the guarantee is needed. Remains available as a debug-time aid.

## Consequences

- A bug terminates the process with a diagnostic instead of crashing silently or continuing with untrustworthy state.
- Where the rule "a component must not throw" is stated, and what the component contract promises about it, belongs to the extensibility decision.
- What the handler prints, and how, is an implementation detail.
