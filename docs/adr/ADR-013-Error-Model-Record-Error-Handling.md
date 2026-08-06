# ADR-013: Error Model — Record Error Handling

## Status

Accepted

**Opened:** August 6th 2026

**Closed:** August 6th 2026

## Context

[ADR-008](ADR-008-Error-Model-Taxonomy.md) recognizes Record error as an expected error class local to one record: the record is lost or unusable, and the next record is independent. [ADR-009](ADR-009-Error-Model-Representation.md) represents it as `expected<T, RecordError>`. What the engine does with the failing record before continuing is not yet decided.

Since the failure is local, the engine is not obliged to stop, but is not obliged to discard the record either. Three behaviors sit in that space:

- **Skip**: drop the record and continue with the next one.
- **Dead-letter**: route the record to a secondary delivery and continue with the next one. This is the [dead letter channel](https://www.enterpriseintegrationpatterns.com/patterns/messaging/DeadLetterChannel.html) pattern from Enterprise Integration Patterns, also used by [Kafka Connect](https://docs.confluent.io/platform/current/connect/concepts.html#dead-letter-queue) and [AWS SQS](https://docs.aws.amazon.com/AWSSimpleQueueService/latest/SQSDeveloperGuide/sqs-dead-letter-queues.html) under the same name.
- **Fail-fast**: stop the pipeline and return the failure to the caller.

Which one fits depends on the user's domain.

## Decision

The behavior is configurable per pipeline, global across all its components, with three choices: **skip**, **dead-letter** and **fail-fast**. The default is fail-fast.

Fail-fast reuses the mechanism of [ADR-012](ADR-012-Error-Model-Pipeline-Error-Handling.md): the record error is promoted to a `PipelineError` at runtime and returned to the caller. The taxonomy of [ADR-008](ADR-008-Error-Model-Taxonomy.md) does not change; the policy decides the runtime behavior, not the class.

The other two cannot be the default choice because, without explicit configuration, skip would drop records the user did not agree to drop, and dead-letter would need a secondary delivery the user did not configure.

### Alternatives rejected

**A fixed policy (no configuration).** Rejected: which behavior fits depends on the user's domain, so no single choice serves every user.

**Configurable per component.** Rejected: no realistic use case requires different policies for the same failure type in different components of the same pipeline.

**Configurable per record error subtype.** Rejected: [ADR-008](ADR-008-Error-Model-Taxonomy.md) groups every local expected failure under one class, with one representation and one runtime behavior. Splitting the class into subtypes with different policies reopens the taxonomy, which no requirement in [`requirements.md`](../architecture/requirements.md) demands.

**Retry, isolation, checkpoint.** Rejected for the same reasons as in [ADR-012](ADR-012-Error-Model-Pipeline-Error-Handling.md), plus one reason specific to **retry**: for the failures the library itself detects (malformed input, wrong value type, missing field), the same input reprocessed produces the same failure, so retry buys nothing.

## Consequences

- Every record error, whichever component produces it and whichever stage it happens in, is handled by the pipeline's configured policy.
- The dead-letter policy requires the user to configure a secondary delivery. The shape of that configuration is not decided here.
- The fail-fast default protects the user only if they realise the policy is set to it. How the public API makes that explicit is not decided here.
