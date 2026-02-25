# Hello World

This tutorial shows how to verify that your Symbol SDK installation is working correctly by writing a minimal program
that:

* Retrieves the network name and launch date using the SDK.
* Connects to a <node:> and prints the current blockchain height.

No accounts, keys, or transactions are required, just a basic SDK call and a REST request.

## Prerequisites

If you have not done so already, start with [Setting Up a Development Environment](../start/setup.md).

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/start/hello-world', ['py', 'js']) }}

### Making SDK Calls

{{ tutorial.code_snippet(['py:7:11', 'js:6:10']) }}

The <dy:SymbolFacade> class is the main entry point to the Symbol SDK.
It provides most of the methods you will need when working with Symbol:
from building and signing transactions to retrieving network-related information.

To create a facade, simply specify the name of the network you want to work with, either `mainnet` or `testnet`.

This example then demonstrates how to retrieve the network launch date.
The <dy:NetworkTimestampDatetimeConverter.toDatetime> method converts
a network timestamp into a UTC datetime.
By passing `0` (the genesis timestamp) you can obtain the moment the genesis block was produced, that is,
the network's launch date.

### Retrieving Information From a Node

{{ tutorial.code_snippet(['py:13:27', 'js:12:28']) }}

Interaction with the Symbol blockchain happens through <API nodes:>, which expose a REST interface for
querying network state and submitting transactions.
Not all nodes provide this interface, so it is important to connect to one labeled as **API Node**.

This example connects to an API node and retrieves the current blockchain height from the <get:/chain/info> endpoint.

This request does not require any private keys or authorization, making it a simple and effective test to confirm that
the environment is set up correctly and can reach the network.

## Output

The output shown below corresponds to a typical run of the program.

```text
--8<-- 'devbook/start/hello-world.log'
```

## Conclusion

If you got the output shown above, you're all set!
You have access to the Symbol SDK and successfully reached a Symbol API node.

That's all you need to start your Symbol adventure.

Why not try [creating an account next](../accounts/create-from-private-key.md)?
