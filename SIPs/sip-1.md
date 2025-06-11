---
SIP: 1
Title: SIP Process and Guidelines
Author: Team TSS
Discussions-to: N/A
Status: Draft
Category: Informational
Created: 2025-Jun-4
Maintainer: Team TSS
---

# SIP-1: SIP Process and Guidelines

## Abstract

This document establishes the Symbol Improvement Proposal (SIP) process. It defines how proposals are created, reviewed, and accepted, and includes roles, categories, status flows, and templates to guide authors, maintainers, and the broader community.

## Current Situation

Currently, there is no standardized process for proposing changes to Symbol. As a result, community members often find it difficult to contribute ideas or understand how proposals are reviewed and adopted.

## Proposed Changes

- Establish a standardized SIP process and markdown template.
- Define clear roles and responsibilities for authors and maintainers.
- Use GitHub as the central hub for SIP tracking and discussions.

## Rationale

The Symbol community regularly generates valuable ideas, but many contributors are unsure how to formally submit or advocate for changes. Discussions often happen in informal channels like Discord or Twitter and are quickly lost. This proposal provides a clear, transparent, and collaborative pathway for meaningful contributions to Symbol's evolution.


## Implementation

### SIP Workflow

```mermaid
sequenceDiagram
participant Proposer
participant Discord/Twitter
participant Maintainer
participant GitHub

Proposer->>Discord/Twitter: 1. Discuss idea with the community
Proposer->>Maintainer: 2. Request SIP number
Maintainer->>GitHub: 3. Create SIP issue
Proposer->>GitHub: 4. Submit SIP PR
Maintainer->>GitHub: 5. Review PR and provide feedback
Maintainer->>GitHub: 6. Update SIP status
```

1. Proposer comes up with an idea and discusses it in a public channel (Discord, Twitter, etc).

2. When the proposer considers the idea to be mature enough, they requests a SIP number from a relevant maintainer.

3. The maintainer assigns a SIP number and creates the corresponding GitHub issue.

4. The proposer writes the SIP and submits a pull request.

5. The maintainer reviews the submission and provides feedback.

6. The maintainer updates the SIP status as needed.

### SIP Categories
- **Core**: Protocol-level changes requiring forks.
- **Networking**: Node configuration or networking behavior.
- **Interface**: API (e.g., REST) or SDK-level changes.
- **Library**: Updates to client libraries (npm, Python, etc.).
- **Application**: Application related changes (Explorer, wallet, etc.).
- **Informational**: Non-binding proposals or documentation guidelines.

### SIP Statuses Workflow

```mermaid
stateDiagram-v2

[*] --> Draft

Draft --> Review: Proposer marks as ready
Review --> Accepted: Maintainer approves
Review --> Rejected: Maintainer declines
Review --> Withdrawn: Proposer withdraws

Accepted --> Implemented: Maintainer updates status
Implemented --> Staging: Maintainer deploys to testnet and updates status
Staging --> Final: Maintainer deploys to mainnet and updates status

Withdrawn --> [*]
Rejected --> [*]
Final --> [*]
```

### SIP Statuses
- **Draft**: Initial version under development. The proposer is still working on the proposal and nobody else is expected to contribute. The proposer will change the status to Review when ready.
- **Review**: Open for review by maintainers and the community. Anybody can contribute. A maintainer will change the status to Accepted or Rejected when a decision is made. The proposer can also set the status to Withdrawn at any point.
- **Accepted**: Approved by the maintainers and planned for future implementation.
- **Rejected**: Declined by maintainers or community consensus. The proposal will not be pursued further.
- **Withdrawn**: The proposer has withdrawn the proposal. It's no longer under consideration.
- **Implemented**: The proposal has been fully implemented in code and is ready for deployment in testnet environment. The maintainer will update the status to Implemented.
- **Staging**: The proposal has been deployed to testnet for live testing. The maintainer will update the status to Staging.
- **Final**: The proposal has been successfully deployed to mainnet. The maintainer will update the status to Final.


## Citations

- [Bitcoin Improvement Proposals (BIPs)](https://github.com/bitcoin/bips)
- [Ethereum Improvement Proposals (EIPs)](https://github.com/ethereum/EIPs)
- [Symbol Proposal Notes by @ymuichiro](https://gist.github.com/ymuichiro/5037f4231ca753b42d622036047e67eb)