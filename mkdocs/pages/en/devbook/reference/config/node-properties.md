# Node Properties

There is a large number of node-related settings that can be customized in Symbol.

These settings can be directly provided to the <Catapult:> client through `.properties` files.

However, the easiest way to change them is by using <Shoestring:> and an `overrides.ini` file.

The header of each of the tables below indicates which file contains that table's properties.

!!! note
    For **network**-related properties, see the [Network Properties](./network-properties.md) page.

## User configuration

<div class="md-typeset__scrollwrap">
<div class="md-typeset__table">
--8<-- 'devbook/reference/config/config-user.properties.html'
</div>
</div>

## Node configuration

<div class="md-typeset__scrollwrap">
<div class="md-typeset__table">
--8<-- 'devbook/reference/config/config-node.properties.html'
</div>
</div>

## Harvesting Configuration

<div class="md-typeset__scrollwrap">
<div class="md-typeset__table">
--8<-- 'devbook/reference/config/config-harvesting.properties.html'
</div>
</div>

<style>
    /* Hide irrelevant column */
    .md-typeset table th:nth-child(3),
    .md-typeset table td:nth-child(3){
        display: none;
    }

    .md-typeset .md-typeset__table table td {
        border: none;
        padding-bottom: 0.25rem;
        font-size: 0.8rem;
    }

    .md-typeset td:nth-child(4) {
        word-break:break-word;
    }
</style>
