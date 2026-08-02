# ADR-005: Execution Model — Granularity

## Status

Accepted

**Opened:** August 1st 2026

**Closed:** August 2nd 2026

## Context

[`decisions-backlog.md`](../architecture/decisions-backlog.md) (Execution model) opens three questions about how data moves from stage to stage. This ADR decides the first: whether the engine processes one record at a time, or a group of records (batch).

The decision is visible in the public API: it determines the signature that a user-written component (functional driver 1) has to implement when producing, transforming, or consuming records.

Functional driver 5 rules out the extreme case where the batch is the whole dataset: an unbounded input has no end to wait for. The open question is between one record at a time vs grouping records in batches of some size N.

## Decision

The engine processes one record at a time.

Two points from the spike justify this. First, the smallest unit does not close the door to grouping later: the engine can still read, process, or write records in groups internally as a runtime optimization, without user-written components knowing. A batched contract does the opposite: it forces every component to speak in groups, even when the component only needs to see the next record. This aligns with quality attribute 1 in [`architectural-drivers.md`](../architecture/architectural-drivers.md): the accepted sacrifice is performance, not the ability to add performance optimizations later where they are actually justified.

Second, the one-at-a-time unit is uniform across the two natures of input the tool has to handle (bounded and unbounded; see [`conceptual-model.md`](../architecture/conceptual-model.md), Stage 1). Prior art from this domain converges on the same principle: Apache Beam ([Beam Programming Guide - PCollection size and boundedness](https://beam.apache.org/documentation/programming-guide/#size-and-boundedness)) treats bounded and unbounded input with the same programming model. Apache Flink ([Flink documentation - Execution Mode](https://nightlies.apache.org/flink/flink-docs-stable/docs/dev/datastream/execution_mode/)) exposes a single DataStream API for both, and internally selects between STREAMING and BATCH execution modes without changing the user-facing contract.

### Alternatives rejected

**Batched contract (groups of size N).** Rejected: it exposes a grouping decision (the size N, when to close a group, what to do with a partial group at the end of a bounded input) to every component, including user-written ones. That complexity is paid by every component even if they only need to see one record. Quality attribute 3 (clean, readable, hard-to-misuse public API) favors the smaller unit.

**User-selectable mode (one-at-a-time or batched, chosen at configuration time).** Rejected: no functional requirement in [`requirements.md`](../architecture/requirements.md) forces two coexisting contracts. Adding a mode that no requirement demands enlarges the public API, doubles the paths to implement and test, and adds a choice the user has to understand. YAGNI applies.

## Consequences

- User-written components implement a contract that speaks in single records. The exact shape of that contract is not fixed here; flow direction and synchrony are decided in separate ADRs.
- The engine remains free to read, process, or write records in groups internally as a future optimization, without changing the public contract.
- No decision on batch size, batch flushing, or partial batches is needed. Those questions do not arise under this contract.