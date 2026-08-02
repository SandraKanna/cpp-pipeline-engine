# ADR-007: Execution Model — Synchrony

## Status

Accepted

**Opened:** August 2nd 2026

**Closed:** August 2nd 2026

## Context

[`decisions-backlog.md`](../architecture/decisions-backlog.md) (Execution model) opens three questions about how data moves from stage to stage. [ADR-005](ADR-005-Execution-Model-Granularity.md) decided the first: the engine processes one record at a time. [ADR-006](ADR-006-Execution-Model-Flow-Direction.md) decided the second: the consumer pulls records from the producer. This ADR decides the third: whether the consumer's call for the next record blocks until the record is available (a blocking model), or returns immediately with a placeholder that will hold the record once it arrives (an asynchronous model).

The decision is visible in the public API. It determines how the call returns its result, and user-written components (functional driver 1) implement against it.

## Decision

The call for the next record is blocking.

Two points from the spike justify this. First, an asynchronous call is only useful when there is something else to do while it waits. Today nothing else happens while the engine waits for the next record: it processes one record at a time ([ADR-005](ADR-005-Execution-Model-Granularity.md)) and the consumer initiates each request ([ADR-006](ADR-006-Execution-Model-Flow-Direction.md)), so an asynchronous call would still end up waiting in practice. Adopting it now would add complexity for a performance gain the engine cannot realize yet. This aligns with quality attribute 1 in [`architectural-drivers.md`](../architecture/architectural-drivers.md): the design favors simplicity over performance when the latter provides no immediate benefit.

Second, blocking is simpler to write against. A blocking component just returns the next record when it has one. An asynchronous component has to return a placeholder for a value that will exist later, using a `std::future`, a coroutine, or a callback, each of them easy to get wrong. Functional driver 1 (the user writes their own components) is best served by the shape any C++ programmer can use without learning the language's newer concurrency features. The [standard library's iterators](https://en.cppreference.com/w/cpp/iterator) and the [C++20 ranges library](https://en.cppreference.com/w/cpp/ranges), already cited in [ADR-006](ADR-006-Execution-Model-Flow-Direction.md), are blocking too, which supports quality attribute 3 (clean, readable, hard-to-misuse public API).

### Alternatives rejected

**Asynchronous (the call returns before the record is ready).** Rejected: no functional requirement in [`requirements.md`](../architecture/requirements.md) demands asynchrony, and its cost (a heavier contract for every user-written component) buys nothing the engine can use today (see Decision above).

## Consequences

- User-written components implement a contract where the call for the next record blocks until the record is available. Any C++ programmer can write one without the language's newer concurrency features.
- If the engine ever gains concurrency between records or between components, that is a change internal to the engine. A blocking component can be wrapped by the engine in a separate thread, without changing the component's contract.
