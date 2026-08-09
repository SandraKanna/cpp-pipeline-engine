# ADR-016: Extensibility — Contract Implementation Mechanism

## Status

Accepted

**Opened:** August 9th 2026

**Closed:** August 9th 2026

## Context

[`decisions-map.md`](../architecture/decisions-map.md) (Extensibility) opens three questions about how a user adds a new component to the pipeline. [ADR-014](ADR-014-Extensibility-Contract-Decomposition.md) fixed how many contracts the extension surface exposes: eight. [ADR-015](ADR-015-Extensibility-Contracts-Shape.md) fixed the logical shape of each contract. This ADR decides which C++ mechanism realises each contract.

Two criteria guided the decision:

1. **Runtime resolution.** `decisions-map.md` states that the concrete type of a component is not known at compile time, because the pipeline is built from configuration. Any mechanism the ADR adopts has to let the engine hold and call a component whose type is chosen after compilation.

2. **Natural fit to what the contract holds.** Some of the eight contracts naturally hold resources whose life spans the pipeline run (an open file, a socket, a buffer carried across calls); others express a rule over one record and hold at most small parameters. The mechanism a contract adopts has to accommodate what the contract holds without asking the user to write scaffolding around it.

Prior art in the same domain adopts two mechanisms according to this split. [Apache Arrow C++](https://github.com/apache/arrow/blob/main/cpp/src/arrow/io/interfaces.h) exposes its I/O layer as abstract classes with pure virtual methods, and its [compute layer](https://arrow.apache.org/docs/cpp/api/compute.html) as kernels registered by function pointer. [Apache Beam](https://beam.apache.org/documentation/programming-guide/#pardo) exposes `DoFn` as a class for stateful components and `ProcessFunction` as a functional interface for pure operations.

## Decision

The five contracts on the byte side (`BytesAcquisition`, `RecordDelimitation`, `RecordParsing`, `RecordSerialization`, `BytesDelivery`) hold resources; they are each an abstract class with one pure virtual method: the operation of ADR-015. The engine holds each component through a base-class pointer and calls the operation through it. The user extends the library by writing a class that derives from the contract and overrides the operation.

The three contracts on the record side (`RecordValidation`, `RecordFiltering`, `RecordTransformation`) express a rule; they are each a distinct nominal type wrapping `std::function<Signature>`. The engine holds each component as its wrapper type. The user extends the library by passing a lambda, a function pointer, or a functor.

Each of the three record-side contracts is a nominal type of its own, not a shared alias. `RecordValidation` and `RecordFiltering` share the same underlying signature with different meaning, and a shared alias would let the compiler accept a validator where a filter is expected.

### Alternatives rejected

**All eight contracts as abstract classes.** Rejected on the natural fit criterion: the record-side contracts hold no resources, so the class the user has to write around a rule is scaffolding without content.

**All eight contracts as callables.** Rejected on the natural fit criterion: the byte-side contracts hold resources whose lifetime is already covered by constructor and destructor, and a callable has no constructor or destructor of its own to manage them.

**Compile-time mechanism.** Contracts realised through concepts, or through templates parameterised by the concrete component type. Rejected on the runtime resolution criterion: neither concepts nor templates keep the concrete type unknown until runtime.

**Hand-written type erasure for the byte-side contracts.** A wrapper type that hides an internal virtual hierarchy, so the user writes a class without inheriting from anything. Rejected on cost: the wrapper is code the project owns and maintains, over a mechanism (abstract class with virtual methods) that already satisfies the runtime resolution criterion.

## Consequences

- The extension surface has two shapes. Extending a byte-side contract means writing a class deriving from an abstract base; extending a record-side contract means writing a lambda or a functor. Which shape a user meets depends on which contract they extend.
- The engine holds byte-side and record-side components through different mechanisms and calls them through different paths. Any code the engine shares across contracts accounts for both.
- The exact signature of each operation, how the engine holds and owns each component, and the concrete technique that makes the record-side wrapper nominal are physical-level decisions, left to implementation.
