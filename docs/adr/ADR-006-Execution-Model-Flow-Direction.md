# ADR-006: Execution Model — Flow Direction

## Status

Accepted

**Opened:** August 2nd 2026

**Closed:** August 2nd 2026

## Context

[`decisions-backlog.md`](../architecture/decisions-backlog.md) (Execution model) opens three questions about how data moves from stage to stage. [ADR-005](ADR-005-Execution-Model-Granularity.md) decided the first: the engine processes one record at a time. This ADR decides the second: which side of a component-to-component connection initiates each exchange. Two shapes are possible: the consumer requests the next record from the producer when ready (a pull model), or the producer delivers the next record to the consumer as soon as it has one (a push model).

The decision is visible in the public API. It determines which side initiates each exchange, and user-written components (functional driver 1) implement against it.

## Decision

The consumer pulls records from the producer.

Two points from the spike justify this. First, pull avoids the backpressure problem by construction. In a push contract, when the producer runs faster than the consumer, records accumulate between them: the engine has to buffer, drop, or signal back to slow the producer down. ReactiveX documents this trade-off explicitly ([ReactiveX - Backpressure](https://reactivex.io/documentation/operators/backpressure.html)), and Apache Flink treats it as an operational concern of its runtime ([Flink documentation - Monitoring back pressure](https://nightlies.apache.org/flink/flink-docs-stable/docs/ops/monitoring/back_pressure/)). In a pull contract, the consumer only asks when it is ready to process the next record, so no buffer or backwards signal is needed. Functional driver 5 (unbounded input) benefits directly: the contract itself keeps the consumer from being overwhelmed, without extra machinery.

Second, pull is the shape C++ programmers already know for iterating over a sequence. The [standard library's iterators](https://en.cppreference.com/w/cpp/iterator) and the [C++20 ranges library](https://en.cppreference.com/w/cpp/ranges) both expose this shape: elements are produced only when the consumer asks for them. This aligns with quality attribute 3 in [`architectural-drivers.md`](../architecture/architectural-drivers.md) (clean, readable, hard-to-misuse public API).

### Alternatives rejected

**Push (producer delivers records to the consumer).** Rejected: no functional requirement in [`requirements.md`](../architecture/requirements.md) demands a push model, and the backpressure cost (see Decision above) is a problem that pull avoids.

## Consequences

- User-written components implement a contract where the consumer initiates each exchange. The exact shape of that contract is not fixed here; synchrony is decided in a separate ADR.
- If a data source is push-based (a network socket that hands over bytes as they arrive, for instance), the component that wraps it has to bridge the two shapes internally: buffer incoming records and hand them out on demand. This cost is paid inside that specific component, not by the rest of the pipeline.