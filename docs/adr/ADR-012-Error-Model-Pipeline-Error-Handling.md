# ADR-012: Error Model — Pipeline Error Handling

## Status

Accepted

**Opened:** August 6th 2026

**Closed:** August 6th 2026

## Context

[ADR-008](ADR-008-Error-Model-Taxonomy.md) recognizes Pipeline error as an expected error class that blocks execution: once it occurs, the next record cannot be produced or trusted. [ADR-009](ADR-009-Error-Model-Representation.md) represents it as `expected<T, PipelineError>`, so the failure is returned to the caller.

Given that shape, one question remains: does the engine do anything between the failure and the return? The literature on pipeline resilience offers three mechanisms that would sit in that gap:
- **retry**: attempt the failing operation again, possibly with backoff, 
- **isolation**: route around the failing component, as in circuit breaker or bulkhead patterns, 
- **checkpoint and recover**: persist execution state and resume from the last checkpoint, as [Apache Flink](https://nightlies.apache.org/flink/flink-docs-stable/docs/dev/datastream/fault-tolerance/checkpointing/) does.

Two criteria guided the decision: whether a current requirement in [`requirements.md`](../architecture/requirements.md) demands the mechanism, and whether it is implementable with the engine as decided in [ADR-005](ADR-005-Execution-Model-Granularity.md) (one record at a time), [ADR-006](ADR-006-Execution-Model-Flow-Direction.md) (pull-based) and [ADR-007](ADR-007-Execution-Model-Synchrony.md) (blocking).

## Decision

The engine stops execution and returns the `PipelineError` to the caller. The policy is global to the pipeline: since none of the mechanisms below is adopted, there is nothing to configure per component.

### Alternatives rejected

**Retry.** Rejected: no requirement demands it, and it needs a way to tell a transient failure from a permanent one, which the taxonomy of [ADR-008](ADR-008-Error-Model-Taxonomy.md) does not carry.

**Isolation (circuit breaker, bulkhead).** Rejected: no requirement demands it, it needs the same transient vs. permanent distinction as retry, and the current pipeline shape has nowhere to route around to (a failing acquisition has no fallback acquisition, a failing delivery has no fallback delivery).

**Checkpoint and recover.** Rejected: no requirement demands it, and it needs acquisition and delivery to record how far they have read and written and to resume from there, which the current design does not include.

## Consequences

- Every Pipeline error, whichever component produces it and whichever stage it happens in, produces the same outcome: the pipeline stops and the `PipelineError` reaches the caller. The caller decides what to do next.
- Any behavior beyond stopping (retry from the caller's side, alerting, logging to a persistent store) is implemented outside the library.
- The shape of `PipelineError` (which fields it carries, how much context it reports) is not decided here.
