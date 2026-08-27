# ADR-017: Module Isolation — Library Packaging

## Status

Accepted

**Opened:** August 27th 2026

**Closed:** August 27th 2026

## Context

[`decisions-map.md`](../architecture/decisions-map.md) (Module isolation) states that heavy dependencies must be optional, and names the AWS SDK as the case that motivates the category: a consumer who does not integrate with AWS should not pay for the SDK in compilation time or footprint. This ADR decides how the library is packaged into CMake targets and how heavy dependencies are made optional inside that packaging.

One criterion guided the decision: **weight of dependency**. Adding a separate target in the build system has a cost: more build configuration to write, more names for the consumer to know, more pieces to link together. That cost is worth paying when the separate target lets a consumer skip a heavy dependency they do not need. Separating pieces of code that carry no heavy dependency pays the cost without giving the consumer anything to skip.

Prior art in the same domain: [Apache Arrow C++](https://github.com/apache/arrow/blob/main/cpp/src/arrow/CMakeLists.txt) ships a single library target `arrow` that consumers link against, composed internally from CMake OBJECT libraries (`arrow_array`, `arrow_io`, `arrow_filesystem`, others). Optional integrations that pull heavy dependencies (S3, GCS, Azure among them) are gated by feature flags (`ARROW_S3`, `ARROW_GCS`), which conditionally append the corresponding source file to the object library and link the dependency to it. With `ARROW_S3=OFF`, `filesystem/s3fs.cc` is not compiled and the AWS SDK is not linked; the single target `arrow` still exists and still exposes the filesystem abstraction.

## Decision

The library ships as a single CMake target, `cpe`, that a consumer links against. All contracts, all first-party components and the engine belong to that target. Directory structure organises code by role for readability; it does not translate into separate build targets.

Heavy dependencies are made optional by CMake feature flags, following the pattern shown in the prior art above. A component whose implementation requires a heavy dependency sits behind a flag that gates both its source file and the link. S3 acquisition and delivery, when they exist, are the first such case: both need the AWS SDK.

### Alternatives rejected

**One target per contract.** Eight targets, one per contract of [ADR-014](ADR-014-Extensibility-Contract-Decomposition.md), each carrying its first-party components. Rejected on the weight criterion: seven of the eight contracts carry no heavy dependency, so target boundaries between them buy the consumer nothing they cannot already skip.

**One target per component.** Fragmentation to the finest grain (a target for Buffer, another for File, another for Line, and so on). The weight criterion cuts harder here: Buffer, File, Line and Raw carry no dependency for anyone to opt out of, so splitting each into its own target is build-system surface without a driver.

**A `cpe_contracts` target holding only the abstract contract headers, alongside a target holding the first-party components.** Rejected on the weight criterion: the abstract contracts are header-only, with no `.cpp` and no external dependencies, so a third party that inherits from a contract without linking any first-party component gains nothing measurable by linking `cpe_contracts` alone.

**A dedicated target for the engine.** No weight to avoid: the engine is the entry point of the library and depends only on the contracts. Any consumer who links `cpe` needs the engine, so a target boundary here isolates code that is always linked together.

**A dedicated target for each heavy-dependency component (`cpe_acquisition_s3` and similar).** Rejected as unnecessary for the driver: gating the source file and the link inside the single target satisfies the same requirement (the dependency is not paid when the flag is off) without adding a target the consumer has to know about or link explicitly.

## Consequences

- A consumer links one target, `cpe`, regardless of which components they use.
- Heavy dependencies enter the build only when the consumer opts in through the corresponding flag. The library does not attempt to detect optional dependencies automatically.
- Any future component that pulls a heavy dependency follows the same shape: a flag, sources appended to `cpe` under the flag, dependency linked to `cpe` under the flag.
- Contracts do not include component headers, and the engine depends only on contract headers. These rules are preserved by include hygiene at the source level, not by target boundaries.
- The exact name of each flag, whether the single target is composed internally from OBJECT libraries (as Arrow does) or built as a flat source list, and the CMake mechanism that guards source inclusion and linkage under a flag, are physical-level decisions, left to implementation.
