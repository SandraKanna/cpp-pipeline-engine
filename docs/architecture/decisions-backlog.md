# Decisions (ADRs) backlog

This backlog maps the architectural drivers (see `architectural-drivers.md`) to the set of significant design decisions the project needs to make. Decisions are grouped by topic rather than by driver, since a single decision can respond to more than one driver, and a single driver can generate more than one decision. Each entry lists its originating driver(s) or section(s), plus a short description of the concrete decision it covers. No decision is resolved here: each one will later become its own ADR, numbered once the order of work is set.

## Data model

### Conceptual level
On a high level, this project defines that different types of data must flow through a pipeline and be transformed into a standardized output that can be reused elsewhere.

- Functional driver 1: the user must be able to write their own components, meaning developers using this library will write the code that defines the object entering the pipeline.
- Functional driver 2: the library must accept different types of data flowing through the same pipeline.

At this level, the shape of that flow is:

`Data Object -> source -> transform -> sink -> standardized output`

(the type of `Data Object` itself is not decided at this level, see logical level below)

### Logical level
Going one level deeper, from functional drivers 2 and 3 we can derive that the `Data Object` cannot be static-generic (no templates): the user must be able to choose among different data types at runtime, without recompiling (driver 3). This requires some form of dynamically-resolved type: `std::variant` (closed set), `std::any` (open type-erasure), or polymorphic inheritance.

### Physical level
How this dynamic type stores its data internally (map-based vs. vector-based field storage, for example) is not decided yet. Deferred to implementation (YAGNI).

## Extensibility
Functional drivers 1 and 3, together with quality attribute 1 (extensibility), define decoupling as the core design principle here. Driver 1 lets the user add their own source/transform/sink implementations for the data types they need in their own workflow. Driver 3 lets them compose those (and the library's own) components into a pipeline at runtime, without recompiling, via configuration.

This is about how a user adds a brand new component, not about isolating heavy dependencies (see Module isolation below).

## Module isolation
Functional driver 4, together with the constraint that AWS integration will arrive in later stages, make this decision straightforward: the library's components must be built as separate, independently buildable modules, and heavy dependencies must be optional. In particular the AWS SDK, which is heavy enough to slow down compilation and increase the footprint for users who don't need it.

## Execution model
Functional driver 5 states that the library must be able to handle unbounded or arbitrarily large data. The use cases include logs and sensor streams that, in principle, never end. This means the incoming data cannot be assumed to fit in memory as a whole. The library needs to decide how many data objects are alive in memory at any given time: load the full dataset before processing (batch) vs. process one record at a time as it arrives (streaming, pull-based).

## Testability
From quality attributes 2 (testability) and 1 (extensibility). Quality attribute 2 states that testability is made cheap by the decoupling that extensibility already demands. The testing tools themselves are already fixed by the project's constraints (GoogleTest/GMock). What this category decides is the shape components must have for those tools to work well against them: internal interfaces must stay minimal, so the surface to mock stays small and every use case (error policies, call order, edge cases) can be tested in isolation without touching disk or network.

## API usability
From quality attribute 3. The design must put the user first: the heaviest part of the work happens backstage, so the user only configures what's strictly necessary. The interface should make common mistakes hard to make. Configuration validation happens in one single place before anything runs. Components ship with sensible defaults that the user can override, which also double as usage examples.

## Error modeling & handling
As the project's context highlights, this is meant to be a production-ready deliverable, meaning it will run against real files, which will very likely contain inconsistencies: misspelled fields, corrupted rows, missing separators, files that don't exist, etc. (Scope also lists CSV, JSON and sensor messages as expected input sources, and driver 5 adds logs to that list, all notoriously imperfect in practice.)

At least two different classes of error need to be distinguished (not yet decided which policy applies to which, deferred to the dedicated ADR):

- **Configuration errors**: e.g. the input file doesn't exist, the pipeline has no sink. These look like programming mistakes, not data problems.
- **Data errors**: e.g. a single row is corrupted or has the wrong type in a field. These are expected in production, and the pipeline needs some policy for them (stop everything? skip the row? route it elsewhere for later inspection?).

### Error modeling
Not decided yet: how each error class is represented in the type system (exceptions vs. some `Result`-like return type). Deferred (YAGNI).

### Error handling
Not decided yet: what runtime behavior each error class triggers (fail fast vs. skip vs. route to a separate destination). Deferred (YAGNI).