# cpp-pipeline-engine
[![CI](https://github.com/SandraKanna/cpp-pipeline-engine/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/SandraKanna/cpp-pipeline-engine/actions/workflows/ci.yml)


> 🚧 Project under construction

**Description:** a modular C++ library used to build data processing pipelines. It acts as an interface for other applications to read data from different sources, easily transform it and export it to different destinations.


## Stack

| Layer   | Technology                             |
|---------|----------------------------------------|
| Language| C++20                                  |
| Build   | CMake                                  |
| Tests   | GoogleTest                             |
| Quality | Clang-format + Clang-tidy + sanitizers |
| CI      | GitHub Actions                         |

## Documentation

- [Requirements](docs/architecture/requirements.md): what the product must do
- [Architectural drivers](docs/architecture/architectural-drivers.md): what shapes this design
- [Conceptual model](docs/architecture/conceptual-model.md): what the tool does
- [Decisions map](docs/architecture/decisions-map.md): architecturally significant decisions + dependency sequence with reversal cost
- [Error scenarios](docs/architecture/error-scenarios.md): everything that can fail as data flows through the pipeline
- Architecture Decision Records ([`docs/adr/`](docs/adr/)): ADR-000 (ADR process), ADR-001 through ADR-004 (data model), ADR-005 through ADR-007 (execution model), ADR-008 through ADR-013 (error model), ADR-014 through ADR-016 (extensibility)

## Status

Implemented so far: the data model, and the acquisition stage with in-memory and file components. The rest is designed but not yet built.
