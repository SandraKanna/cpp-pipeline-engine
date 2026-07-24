# Architectural Drivers
**Date:** July 18th 2026
**Status:** Accepted

## Vision & Purpose

This project aims to build an open source, modular C++ library to help users build data processing pipelines. The tool will act as an interface for other applications to read data from different sources, easily transform it and export it to different destinations.

It is intended to be accessible via different entry points, such as a CLI and, later, the cloud. The CLI, built on top of the library, will serve both as an example of integration and as a ready-to-use tool.

## Context

This is a non-commercial portfolio project with a production-ready deliverable, intended to showcase my skills with modern tools and good architectural and coding practices. The public API and the project's
documentation are part of the deliverable itself: they are meant to be read, not just used.

## Functional requirements that shape the architecture

1. The user must be able to write their own components (e.g. source/transform/sink) and combine them with the library's. This requires **stable contracts** (public interfaces) so the user never needs to know the internal logic of the tool.
2. Different types of data must flow through the same pipeline. This requires a **common data model** that all components share.
3. The user must be able to build a pipeline **at runtime**, without recompiling, as common data pipeline tools do (cf. [Kafka Connect](https://kafka.apache.org/41/kafka-connect/user-guide/), which builds entire pipelines from JSON/Properties config against a running service).
4. The user shouldn't pay the cost of features they don't use. Heavy dependencies (like future cloud integrations) must be separated from the core, so the architecture needs **optional modules** with their own boundaries.
5. The tool must handle unbounded or arbitrarily large data (logs, sensor streams): input cannot be assumed to fit in memory.

## Quality attributes (prioritized)

1. **Extensibility.** The main goal of this project is to showcase clear architectural design, not to compete with existing data pipeline tools on speed. The functional requirements point in the same direction: stable contracts, decoupled components, and runtime pipeline configuration are all flexibility features.
   **Accepted sacrifice:** performance. Dynamic typing at runtime means more indirections and less compiler optimization than a compile-time design, and type errors will appear at runtime instead of being caught by the compiler. This cost is accepted and will be measured in due time.

2. **Testability.** Required for quality, and made cheap by the decoupling that extensibility already demands: small interfaces are easy to test in isolation.
   **Accepted sacrifice:** more design work and discipline up front, which slows down the coding process.

3. **API usability.** The public API is part of the deliverable itself, so it must be clean, readable and hard to misuse.
   **Accepted sacrifice:** internal implementation complexity (a comfortable API requires more elaborate code underneath) and configurability (sensible defaults instead of exposing every possible option).

## Constraints

- The stack is fixed from day 1: C++20, CMake, GoogleTest, GMock clang-format/clang-tidy, GitHub Actions. Rationale: modern, industry-standard tooling, aligned with the project's showcase purpose.
- AWS integration is part of the vision; it will arrive incrementally, and its pace depends on my learning curve (currently doing the AWS Cloud Practitioner training).
- This is a one-person project: time and resources are limited, which favors small iterations and simplicity.

## Scope

- **In:** structured data (CSV, JSON, sensor messages) that can be represented as records of fields.
- **Out:** binary data (images, audio, medical formats). It would require a different data model and a different architectural design.
- **Out:** data analysis. The tool transforms and normalizes records to output homogeneous formats for other tools to read; it does NOT aggregate, join sources, compute statistics or detect anomalies. That belongs to the user's own workflow, with other tools.