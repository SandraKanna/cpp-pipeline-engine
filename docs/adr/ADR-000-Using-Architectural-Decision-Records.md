# ADR-000 - Using Architectural Decision Records (ADRs)

## Status

accepted

**Opened:** July 22nd 2026

**Closed:** July 22nd 2026

## Context

After coming up with the vision and main constraints/goals for this project, I wanted to start designing some parts of the tool after only a single look at the architectural drivers. Not taking the time to weigh them and analyze them correctly was misleading me into taking the wrong decision (e.g. going with dynamic plugins instead of the simpler, and sufficient for this portfolio project, polymorphic inheritance). Taking the time at the beginning to get a global view of the project is important before starting anything. ADRs help with this, as they force me to weigh the pros and cons of each decision before acting on it.

`architectural-drivers.md` also states that documentation is part of this project's deliverable itself, meant to be read, not just used. Finally, I intend to reuse this project's architectural methodology in future repos, so the process itself needs to be documented clearly enough to be repeatable.

## Decision

Use ADRs to document every significant architectural decision (the ones that are costly to reverse) taken while developing this tool, derived from the original architectural drivers. Implementation-level decisions that are cheap to change are not required to go through this process. The official flow is:

`drivers → backlog → ADR`

## Consequences

This decision means taking more time at the beginning, before touching the core app or even designing the tool in detail, in order to sweep all the drivers and build the backlog first. It also means committing to repeat this full flow for every future significant decision, even when in the moment a shortcut might feel justified.

In exchange, it saves time later, gives the project a single clear source of truth, and leaves a clean log of technical decisions. The alternatives considered were: ad-hoc decisions without a backlog, and inline documentation (comments or a single large design document instead of separate ADRs). Both allow a faster start, but fail to deliver the clear, auditable documentation that is itself part of this project's deliverable.