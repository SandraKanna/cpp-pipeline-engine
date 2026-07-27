# ADR-002: Data Model — Value Representation Mechanism

## Status

Accepted

**Opened:** July 23rd 2026

**Closed:** July 27th 2026

## Context

[`decisions-backlog.md`](../architecture/decisions-backlog.md) (Data model, Logical level) establishes that a field's value type cannot be fixed at compile time: it must be chosen at runtime, without recompiling. This ADR decides which C++ mechanism represents that value. 

A spike over modern C++ features and community references ([Stack Overflow — runtime mapping of values to types](https://stackoverflow.com/questions/62759640/runtime-mapping-of-values-to-types), [Nimrod's Coding Lab — elegant ways to map runtime values to types](https://nimrod.blog/posts/cpp-elegant-ways-to-map-runtime-values-to-types/)) surfaced three mechanisms that let the concrete type be chosen at runtime:

- [`std::variant`](https://en.cppreference.com/w/cpp/utility/variant): a type-safe union: a fixed set of types, defined at compile time, holding one of them at a time.
- [`std::any`](https://en.cppreference.com/w/cpp/utility/any): a container that can hold a value of any type, read back by asking for the type you expect.
- Polymorphic inheritance with [RTTI](https://en.cppreference.com/w/cpp/language/typeid): a base class with one subclass per kind of value, reached through a base pointer; `typeid` / `dynamic_cast` recover the concrete type at runtime.

## Decision

`Value` is represented as `std::variant`.

Two conditions constrain the choice. First, the type is chosen at runtime, without recompiling ([`decisions-backlog.md`](../architecture/decisions-backlog.md), Logical level). Second, [ADR-001](ADR-001-Data-Model-Value-Types.md) established that the value categories form a closed set, known at compile time, and that composites make the set recursive. `std::variant` fits both: it holds one type at a time from a set defined at compile time and chosen at runtime, and it admits the recursive composites as alternatives backed by standard containers. It also keeps the compile-time list of what a value can be, which the alternatives below give up or scatter.

### Alternatives rejected

**`std::any`** can hold any type, including ones unknown today, but gives up the compile-time list of what a value can be. That openness is only useful for genuinely new kinds of data, and [`architectural-drivers.md`](../architecture/architectural-drivers.md) (Scope) places those (such as binary) in a different data model. It is not needed here.

**Polymorphic inheritance.** Each kind of value becomes a subclass of a base class, reached through a base pointer. This forces a heap allocation and an indirection on every value, scalars included, when a scalar needs neither. It also commits the public type to one storage strategy, which [`decisions-backlog.md`](../architecture/decisions-backlog.md) (Data model, Physical level) leaves open for implementation to decide.

## Consequences
- Adding a new kind of value later requires editing `Value` and the code that reads it: a compiler-visible change, not a silent gap.
- `std::variant` lets the compiler check that code handles every type a value can hold (a check `std::any` and a base-pointer hierarchy cannot offer). Whether the project enforces it (rather than allowing a generic catch-all that skips it) is a code-style matter, out of scope here.
- The internal storage of a `Value` stays free to change without affecting the public contract, including which container backs the recursive composites: a physical-level choice, left open.