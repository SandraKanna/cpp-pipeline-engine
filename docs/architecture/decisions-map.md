# Decisions (ADRs) map

This document maps the architectural drivers (see `architectural-drivers.md`) to the set of significant design decisions the project needs to make, in the order they need to be answered, and tracks their current status. Decisions are grouped by topic rather than by driver, since a single decision can respond to more than one driver, and a single driver can generate more than one decision. Answers live in individual ADRs under `docs/adr/`.

## Data model [resolved]

### Conceptual level

See [conceptual model](conceptual-model.md) for what a *record* is. What it is made of in the type system is decided at the logical level below.

### Logical level
Going one level deeper, from functional drivers 2 and 3 we can derive that the type of a record cannot be fixed at compile time: the user must be able to choose among different data types at runtime, without recompiling (driver 3). This requires some form of dynamically-resolved type.

### Physical level

**Open:** how this dynamic type stores its data internally (map-based vs. vector-based field storage, for example). *Deferred to implementation, not needed to make the data model decision itself.*

## Execution model [resolved]
Functional driver 5 states that the library must be able to handle unbounded or arbitrarily large data. The use cases include logs and sensor streams that, in principle, never end. This means the incoming data cannot be assumed to fit in memory as a whole. Deciding how the pipeline moves data from stage to stage opens three questions, to be answered in sequential order as each one conditions the next:

**Open:** whether the tool processes one record at a time, or a group of records (batch).
**Open:** whether the consumer pulls records from the producer, or the producer pushes records to the consumer.
**Open:** whether a call requesting the next record is blocking or asynchronous.

## Error Model [resolved]
As the project's context highlights, this is meant to be a production-ready deliverable, meaning it will run against real-world input, which will very likely contain inconsistencies: misspelled fields, corrupted rows, missing separators, files that don't exist, etc. All the expected input sources (CSV, JSON and sensor messages, logs) are imperfect in practice. Two situations already come from the current scope: errors raised while the pipeline is being built (an invalid configuration) and errors raised while data flows through it (a corrupted row). Whether they are two classes, one, or more, is part of the taxonomy decision. The category splits into three decisions.

**Open:** which classes of error the model recognizes (taxonomy).

**Open:** how each class is represented in the type system (modeling).

**Open:** what runtime behavior each class triggers (handling), and whether the policy is global to the pipeline or configurable per component.

## Observability [pending]
The conceptual model marks `errors & logs` as a transversal concern observable from outside the library. Whether the library emits its own log stream, and what shape the final execution report takes, are open decisions separate from the error model. Deferred to their own sessions.

## Extensibility [in-progress]
Extensibility here means the ability for a user to add a new component of their own to the pipeline. Functional drivers 1 and 3, together with quality attribute 1 (extensibility), define decoupling as the core design principle. Driver 1 lets the user add their own components. Driver 3 lets them compose those (and the library's own) components into a pipeline at runtime, via configuration.

This block decides the contract any component must satisfy, whether it ships with the library or comes from the user. The facade the user interacts with (builder, configuration schema, resolution from a name to a concrete component) belongs to API usability.

**Open:** how many contracts the extension surface exposes. The five responsibilities in `conceptual-model.md` may map one-to-one, fuse, or split. Three cut points sit inside this question: whether acquisition and deserialization are one contract or two (bytes or records at the extension point?), whether cutting the byte stream into record-sized chunks is part of deserialization or a responsibility of its own that comes before it, and whether serialization and delivery are one contract or two.

**Open:** what shape each contract has: which operations the user implements, how end-of-stream is signalled, whether lifecycle operations are needed.

**Open:** which C++ mechanism realises each contract. Driver 3 constrains this: the concrete type of a component is not known at compile time.

## API usability [pending]
From quality attribute 3. The design must put the user first: the heaviest part of the work happens backstage, so the user only configures what's strictly necessary. The interface should make common mistakes hard to make. Configuration validation happens in one single place before anything runs. Components ship with sensible defaults that the user can override, which also double as usage examples.

**Open:** who decides the input format (CSV, JSON, log lines): the user at configuration time, or the tool by inspecting the data? Not every input is a file with an extension to rely on. *Not resolved here, this document only scopes the question.*

**Open:** when a format allows more than one possible cut (a JSON array read as one record or as many), who decides: the user at configuration time, or the tool at run-time? *Not resolved here, this document only scopes the question.*

## Module isolation [pending]
Functional driver 4, together with the constraint that AWS integration will arrive in later stages, make this decision straightforward: the library's components must be built as separate, independently buildable modules, and heavy dependencies must be optional. In particular the AWS SDK, which is heavy enough to slow down compilation and increase the footprint for users who don't need it.

## Testability [in-progress]
From quality attributes 2 (testability) and 1 (extensibility). Quality attribute 2 states that testability is made cheap by the decoupling that extensibility already demands. The testing tools themselves are already fixed by the project's constraints (GoogleTest/GMock). What this category decides is the shape components must have for those tools to work well against them: internal interfaces must stay minimal, so the surface to mock stays small and every use case (error policies, call order, edge cases) can be tested in isolation without touching disk or network.

## Dependency sequencing

### Reversal cost

| Category | Reversal cost | Why |
|---|---|---|
| Data model | High | It's the type flowing through the entire public API; changing it breaks interfaces, third-party components, and the error model. |
| Execution model | High | The stage contracts encode whether records arrive one at a time or in bulk; changing that later reshapes the contracts, not just the internal implementation. |
| Error | High | Swapping Result<T> for exceptions (or vice versa) touches all code, including third-party components already written against it. |
| Extensibility | High | Defines the contract any component (own or third-party) must satisfy; changing it breaks everything already written against that contract. |
| API usability | High | It's the public-facing surface; changing it directly inconveniences users already relying on it. |
| Module isolation | Medium | Reversible (it's a matter of directory/target organization), but costly if delayed: reorganizing existing modules and CMakeLists takes real work, though it doesn't break usage contracts. |
| Testability | Low | Not a decision with content of its own — it's a consequence of how the other decisions are designed. Can be applied at any point with no real reversal cost. |
| Observability | Low | Adds channels the outside can consume; does not change the contract components speak. Can be introduced later without breaking anything already written against the pipeline. |


### Dependencies graph

<p align="center">
  <img src="img/dependencies-graph.png" alt="Dependencies graph" width="500">
</p>

1. **Data model** — no dependencies. Defines the type that flows through the pipeline; every other decision references it.
2. **Execution model** — depends on (1). You can't decide how data moves without first knowing what that data is.
3. **Error Model** — depends on (2). How an error is modeled and handled depends on whether records are processed one at a time or in bulk, which is exactly what decision (2) settles.
4. **Extensibility** — depends on (1), (2), (3). A user-defined component must fit into the pipeline, which already determines what it receives (1), when it's called (2), and what an error looks like (3).
5. **API usability** — depends on (4). The public-facing builder can only be designed once the underlying components and their contract already exist.
6. **Module isolation** — depends on (1) and (4) specifically, not (2) or (3). You need to know what kinds of components exist (1) and how a user adds their own (4) to decide which compilation module each one belongs to. Execution model and error handling don't affect where a component physically lives.

> **Testability & Observability** - both transversal, not part of the sequence. They start as soon as something testable/observable exists (once 1 and 2 have a first shape) and re-apply to every piece of code added afterward, rather than occupying a fixed position.
