# cpp-pipeline-engine
[![CI](https://github.com/SandraKanna/cpp-pipeline-engine/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/SandraKanna/cpp-pipeline-engine/actions/workflows/ci.yml)


> 🚧 Project under construction

**Description:** a modular C++ library used to build data processing pipelines. It acts as an interface for other applications to read data from different sources, easily transform it and export it to different destinations.



## Stack

| Layer         |        Technology                          
|---------------|-----------------------------------------
| Language      | C++20
| Build         | CMake
| Tests         | GoogleTest
| Quality       | Clang-format + Clang-tidy + sanitizers
| CI            | GitHub Actions

## Documentation

- [Requirements](docs/architecture/requirements.md): what the product must do
- [Architectural drivers](docs/architecture/architectural-drivers.md): what shapes this design
- [Conceptual model](docs/architecture/conceptual-model.md): what the tool does
- [Decisions map](docs/architecture/decisions-map.md): architecturally significant decisions + dependency sequence with reversal cost
- Architecture Decision Records ([`docs/adr/`](docs/adr/)): ADR-000 (ADR process), ADR-001 through ADR-004 (data model), ADR-005 through ADR-007 (execution model), ADR-008 through ADR-013 (error model)

## Data model

Implemented in [`include/cpe/data_model.hpp`](include/cpe/data_model.hpp) / [`src/data_model.cpp`](src/data_model.cpp), covering ADR-001 through ADR-004. Tests in [`tests/data_model_test.cpp`](tests/data_model_test.cpp).