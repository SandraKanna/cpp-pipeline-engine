# Architectural Drivers
**Date:** July 18th 2026

**Status:** Accepted

This document analyses how the requirements from [requirements.md](requirements.md) shape the architecture, and states the quality attributes the design prioritizes together with what each one costs. It does not decide any concrete design: those decisions are tracked in [decisions-backlog.md](decisions-backlog.md) and resolved in individual ADRs.

## Functional drivers

Each functional requirement below forces a structural property the design must have.

1. **Users write their own components.** For a user to add a component without knowing how the tool works inside, the boundary between their code and the library must be a public interface they program against, a **stable contract**, independent of the internal implementation (decoupling).

2. **Different formats through the same pipeline.** For data of different formats to flow through one pipeline, every component must exchange it in a **common data model** they all share, rather than each component speaking its own format.

3. **Pipelines built at runtime.** Building a pipeline from configuration, without recompiling, means the wiring between components is resolved **at runtime**, not fixed at compile time. This is how established tools work: Kafka Connect, for instance, creates and modifies connectors at runtime through configuration submitted to a running service ([Kafka Connect user guide](https://kafka.apache.org/41/kafka-connect/user-guide/)).

4. **Users don't pay for what they don't use.** For heavy optional capabilities not to burden users who don't need them, those capabilities must live in **optional modules** with their own boundaries, separable from the core.

5. **Unbounded or large input.** For input that cannot be assumed to fit in memory, the design cannot load the whole dataset before working on it; it must be able to process data **as it flows**, keeping only part of it in memory at a time.

## Quality attributes (prioritized)
When two desirable properties compete, these priorities decide which one wins. Each is stated with the sacrifice accepted to obtain it.

1. **Extensibility.** The main goal of this project is to showcase clear architectural design, not to compete with existing data pipeline tools on speed. The functional requirements point in the same direction: stable contracts, decoupled components, and runtime pipeline configuration are all flexibility features.
   **Accepted sacrifice:** performance. Prioritizing flexibility and runtime configuration over a rigid compile-time design costs some efficiency and moves some checks from compile time to run time. The exact cost depends on decisions not yet made and will be measured once they are.

2. **Testability.** Required for quality, and made cheap by the decoupling that extensibility already demands: small, well-separated interfaces are easy to test in isolation.
   **Accepted sacrifice:** more design work and discipline up front, which slows down the coding process.

3. **API usability.** The public API is part of the deliverable itself, so it must be clean, readable and hard to misuse.
   **Accepted sacrifice:** internal implementation complexity (a comfortable API requires more elaborate code underneath) and configurability (sensible defaults instead of exposing every possible option).