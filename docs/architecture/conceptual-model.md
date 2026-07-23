# Shared conceptual model

This document describes what the tool does, in four stages from the most general to the most detailed. It derives from [architectural-drivers.md](architectural-drivers.md) and provides the shared vocabulary used in [decisions-backlog.md](decisions-backlog.md).

## Stage 0 — Context

<p align="center"><img src="img/stage-0_context.png" alt="Context diagram" width="700"></p>

Source data arrives in different formats; the library filters and transforms it and produces a standardized output in a single format. Binary data (images, audio) is not accepted. Analysis (aggregating, joining sources, computing statistics) is deliberately left out: the tool produces homogeneous output *so that* other tools can analyze it.

## Stage 1 — Characterizing the input

<p align="center"><img src="img/stage-1_input.png" alt="Acquisition channel and data nature" width="700"></p>

Two independent axes: the acquisition channel (local or network) and the nature of the data (bounded or unbounded). A bounded dataset has a known, fixed size and does not grow over time; an unbounded one grows while it is being read. All four combinations exist, so the channel does not determine boundedness. Format (CSV, JSON, log lines) is a third axis, independent of the other two.

## Stage 2 — Inside the library

<p align="center"><img src="img/stage-2_internal-flow.png" alt="Internal flow and library boundary" width="700"></p>

Configuration is provided by the user once, before any data flows, and defines every stage. Errors and logs are transversal: they occur at every stage and are observable from outside the library. Acquisition sits at the boundary: the library ships its own implementations and the user can provide theirs.

## Stage 3 — Responsibilities

<p align="center"><img src="img/stage-3_responsibilities.png" alt="Responsibilities and data states" width="700"></p>

From here on, `extraction` is named `deserialization`, and `output` splits into `serialization` and `delivery`.

| Responsibility | What it does |
|---|---|
| Acquisition | obtains bytes from an origin |
| Deserialization | interprets those bytes as records |
| Processing | validates, filters and transforms records |
| Serialization | writes records back as bytes |
| Delivery | leaves those bytes at a destination |

Two consequences:

- A record only exists between deserialization and serialization. Everywhere else the data is bytes. This bounds the scope of the data model decision.
- Acquisition and delivery are symmetrical and are the only responsibilities that touch the outside world; the three in between know nothing about files, sockets or buckets. Heavy dependencies therefore live at the two ends.

## Architectural pattern

Independent stages, each with a single function, chained by the data passing from one to the next: this shape is the [Pipe and Filter](https://www.geeksforgeeks.org/system-design/pipe-and-filter-architecture-system-design/) pattern. ETL is one of its use cases, alongside Unix pipelines and compilers.