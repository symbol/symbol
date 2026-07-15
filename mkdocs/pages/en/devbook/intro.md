---
title: Welcome
---

# Welcome to the Developer Manual

The Developer Manual is for developers building applications on Symbol.
It provides code examples in Python and JavaScript,
showing how to perform common tasks with the SDK or the HTTP API.

The manual is structured as follows:

<div class="grid cards cols-3" markdown>

* :material-rocket-launch-outline:{ .lg .middle } **Getting Started**

    ---

    Set up your development machine and run a quick `Hello World` sample
    to check that everything is ready.

    [:octicons-arrow-right-24: Set up your environment](start/setup.md)

* :material-school-outline:{ .lg .middle } **Tutorials**

    ---

    Follow task-focused tutorials grouped by area.

    [:octicons-arrow-right-24: Create your first account](accounts/create-from-private-key.md)

* :material-book-open-variant:{ .lg .middle } **Reference Guides**

    ---

    Consult exhaustive information about SDK methods, HTTP and WebSockets
    endpoints, and binary structures.

    [:octicons-arrow-right-24: Browse the reference](reference/py/AccountDescriptorRepository.md)

</div>

## Tutorials

Use the navigation menu, or jump directly into one of the tutorials below.

Tutorials are grouped by level, from beginner to advanced, based on the required familiarity with Symbol concepts.
Each tutorial links to the [textbook](../textbook/intro.md) and the relevant reference guides when background
information is useful.

{% import 'tutorials_table.jinja2' as tutorials_table with context %}

{{ tutorials_table.render() }}
