# Requirements
**Date:** July 18th 2026

**Status:** Accepted

This document states what the product must do and for whom. It does not decide how the tool is built: the architectural analysis derived from it lives in [architectural-drivers.md](architectural-drivers.md).

## Vocabulary

Terms the reader needs to understand what follows. These name things in the problem domain, not design decisions.

- **Data processing** — reading data from a source, modifying it (for example checking, cleaning or reshaping it), and writing the result somewhere else, possibly in a different format.
- **Pipeline** — a chain of independent steps through which data flows, each step doing one part of the processing and passing its result to the next.
- **Component** — one step of a pipeline. Examples: a step that reads a CSV file, a step that filters out rows missing a field, a step that writes results as JSON.

## Vision & Purpose

This project aims to build an open source, modular C++ library to help users build data processing pipelines. The tool will act as an interface for other applications to read data from different sources, easily transform it and export it to different destinations.

It is intended to be usable through different interfaces, such as a CLI and, later, the cloud. The CLI, built on top of the library, will serve both as an example of integration and as a ready-to-use tool.

## Context

This is a non-commercial portfolio project with a production-ready deliverable, intended to showcase my skills with modern tools and good architectural and coding practices. The public API and the project's documentation are part of the deliverable itself: they are meant to be read, not just used.

## Functional requirements

What the product must let its users do.

1. The user must be able to write their own components and combine them with the library's, without the need to know how the tool works internally.
2. Data arriving in different formats (for example CSV, JSON, log lines) must be able to flow through the same pipeline.
3. The user must be able to build a pipeline at runtime, without recompiling, by describing it in the configuration rather than in the code.
4. The user must not pay the cost of features they don't use: heavy optional capabilities (such as future cloud integrations) must not burden users who only need the basics.
5. The tool must handle unbounded or arbitrarily large data (for example logs or sensor streams), where the input cannot be assumed to fit in memory.
6. Configuring and using the tool must be as friendly as possible: common tasks should be simple, and common mistakes hard to make.

## Constraints

- The stack is fixed from day 1: C++20, CMake, GoogleTest, GMock, clang-format/clang-tidy, GitHub Actions. Rationale: modern, industry-standard tooling, aligned with the project's showcase purpose.
- AWS integration is part of the vision; it will arrive incrementally, and its pace depends on my learning curve (currently doing the AWS Cloud Practitioner training).
- This is a one-person project: time and resources are limited, which favors small iterations and simplicity.

## Scope

- **In:** structured data such as CSV, JSON and sensor messages — data organized as a set of named fields.
- **Out:** binary data (images, audio, medical formats). It would require a different internal design.
- **Out:** data analysis. The tool transforms and normalizes data to output homogeneous formats for other tools to read; it does NOT aggregate, join sources, compute statistics or detect anomalies. That belongs to the user's own workflow, with other tools.