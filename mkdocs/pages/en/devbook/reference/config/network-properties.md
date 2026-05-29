# Network Properties

There is a large number of network-related settings that can be customized in Symbol.
These settings can be directly provided to the <Catapult:> client through `.properties` files.

However, the easiest way to change them is by using <Shoestring:> and an `overrides.ini` file.

The header of each of the tables below indicates which file contains that table's properties.

!!! warning
    Setting any configuration property to a value different from those of the rest of the network will make your node
    <fork:>, effectively disconnecting it from the rest of the network.

    Only [Node Properties](./node-properties.md) can be safely edited, as they only affect your node.

## Network Configuration

<div class="md-typeset__scrollwrap">
<div class="md-typeset__table">
--8<-- 'devbook/reference/config/config_network.properties.html'
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
