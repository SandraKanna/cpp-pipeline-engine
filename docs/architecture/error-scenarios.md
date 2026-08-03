# Error scenarios

**Date:** August 3rd 2026

**Status:** Accepted

This document maps everything that can fail as data flows through the pipeline, stage by stage, and states which failures the library treats as errors. It derives from [conceptual-model.md](conceptual-model.md) (Stage 3), applying the "what can go wrong" angle to the responsibilities defined there. It does not decide how failures are categorized, represented or handled: those decisions are tracked in [decisions-map.md](decisions-map.md) (Errors) and resolved in individual ADRs, which use this map as their shared reference.

## Vocabulary

- **Stage:** one of the five responsibilities defined in `conceptual-model.md` Stage 3: acquisition, deserialization, processing, serialization, delivery.
- **Setup:** the moment before any data flows, when the library takes the user's pipeline configuration, validates it and wires the stages together. Errors detected here are separate from errors detected once data is flowing.
- **Outcome:** the result of a stage doing (or attempting) its work. Most outcomes are successful (a record produced, a stream ended normally), some are errors.
- **Error:** an outcome the library models and reports through its error model. Not every non-successful outcome is an error: a stream ending, or a record filtered out on purpose, are part of the pipeline's normal life cycle and travel through separate channels.

## Setup

Failures detected here prevent the pipeline from starting: no bytes are read, no output is produced.

- **Missing required stage.** The configuration does not name a stage that is required for the pipeline to be executable. At minimum, the pipeline needs a stage to acquire bytes from an origin and a stage to deliver bytes to a destination.
- **Unknown stage type.** The configuration names a stage type the library does not know (a format the library was not told about, an unrecognized processing step).
- **Invalid stage parameters.** A named stage exists but its parameters are missing (a required path not provided), malformed (a number where a string is expected), or contradict each other (two mutually exclusive options both set).
- **Unreadable configuration input.** The configuration itself cannot be read: the file does not exist, its content is not valid YAML/JSON, or the code-based setup calls are made in the wrong order.
- **Incompatible pipeline composition.** Individual stages are valid but the way they are composed is not: two consecutive stages exchange incompatible types or schemas, or the overall order of stages does not form a valid pipeline.

## Acquisition

This stage obtains bytes from an origin (a file, a network endpoint, a message queue). Failures here mean the bytes never enter the pipeline: no records can be built downstream because there is nothing to build them from.

- **Origin not found.** The origin the pipeline was told to read from does not exist (a path that leads nowhere, a hostname that does not resolve).
- **Access denied.** The origin exists but the pipeline is not allowed to read from it (file permissions, missing credentials, revoked authorization).
- **Origin unreachable.** The origin exists and is allowed but cannot be contacted right now (network unreachable, connection refused, timeout on establishing the connection).
- **Read failure mid-stream.** Reading started successfully but broke while bytes were flowing (connection dropped, device disconnected, transient I/O error).
- **Origin exhausted before expected.** The origin ended before delivering what it announced (a content-length header that overpromises, a truncated file).

## Deserialization

This stage interprets the bytes as records. Failures here mean the bytes arrived but cannot be turned into a well-formed record.

- **Malformed input.** The bytes do not conform to the format's grammar (an unclosed quote in CSV, invalid JSON syntax, a log line that does not match the expected shape).
- **Truncated input.** The bytes form a valid beginning but end before a record is complete (a JSON object without its closing brace, a CSV row cut mid-field).
- **Duplicate field name in a record.** A single record carries the same field name more than once, which the record contract forbids (see [ADR-003](ADR-003-Data-Model-Record-Contract.md)).
- **Unrepresentable value.** The bytes encode a value the data model cannot hold: a category outside the closed set defined in [ADR-001](ADR-001-Data-Model-Value-Types.md). Numeric values beyond the representation's precision are not errors but documented limitations; see [ADR-004](ADR-004-Data-Model-Numeric-Value-Representation.md).

## Processing

This stage validates, filters and transforms records. Failures here mean a record arrived well-formed but cannot be processed as expected.

- **Missing required field.** The record does not carry a field the processing step expects to find.
- **Wrong value type.** A field carries a value of a category the processing step does not accept (a text where a date is expected).
- **Value out of range.** A value falls outside the range the processing step accepts (a negative age, a timestamp before an allowed lower bound).
- **Failed validation rule.** A record fails a domain rule declared by the processing step (an email that does not match the expected pattern, two fields that must agree but do not).
- **User processing step signals a domain-specific failure.** A user-written processing step reports a failure the library cannot describe with the categories above (a rejection tied to the user's own business rules).

## Serialization

This stage writes records back as bytes. Failures here mean a record is well-formed inside the pipeline but cannot be expressed in the target format.

- **Value not expressible in the target format.** A value uses a category the target format cannot represent (a nested object written to CSV, an array in a format that only accepts scalars).
- **Missing field required by the target format.** A record lacks a field the target format or its declared schema requires (a CSV column defined in the header but absent from the record).
- **Encoding incompatibility.** The value's byte-level representation is incompatible with the target format's expectations (a character outside the target encoding).

## Delivery

This stage leaves the produced bytes at a destination. Failures here mean the records were serialized correctly but cannot reach or persist at the destination.

- **Destination not found.** The destination the pipeline was told to write to does not exist (a path with no parent directory, a hostname that does not resolve, a bucket that was not created).
- **Access denied.** The destination exists but the pipeline is not allowed to write to it (file permissions, missing credentials, revoked authorization).
- **Destination unreachable.** The destination exists and is allowed but cannot be contacted right now (network unreachable, connection refused, timeout on establishing the connection).
- **Write failure mid-stream.** Writing started successfully but broke while bytes were flowing (connection dropped, device disconnected, transient I/O error).
- **Destination capacity exceeded.** The destination cannot accept more bytes (disk full, quota exceeded, remote store rejects further writes).
- **Destination rejects the payload.** The destination is reachable and has capacity but refuses what was written (a remote API returns an error, a store that checks the shape of what it receives refuses the record).


## Transversal failures

Some failures are not bound to a single stage: they can arise wherever the pipeline consumes a resource or runs code. Grouping them separately avoids repeating the same category in every stage above.

- **Resource exhaustion.** A resource the pipeline depends on is depleted while the pipeline is running (out of memory, disk full while buffering, file descriptors or sockets exhausted).
- **Broken invariant inside a component.** A stage's own code reaches a state its author believed impossible (an assertion violated, a data structure left inconsistent). This is a bug, not malformed input.
- **Unexpected failure escaping from a component.** A stage signals a failure through a channel outside the pipeline's contract (a C++ exception thrown from a component that was not expected to throw, an error code the pipeline does not know how to interpret).