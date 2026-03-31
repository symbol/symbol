# Network Time

Time in the Symbol blockchain is measured in _network time_, defined as the number of milliseconds elapsed since
the chain's first <block:>.

This quick tutorial shows how to convert the network time returned by API calls to standard timestamps like
[UTC](https://en.wikipedia.org/wiki/Coordinated_Universal_Time).

## Prerequisites

This tutorial uses the [Symbol REST API](../reference/rest/symbol.md) without requiring an SDK.
You only need a way to make HTTP requests.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/chain/network-time', ['py', 'js']) }}

## Code Explanation

The code retrieves the Nemesis block timestamp and the current network time.
Then it adds them together to obtain the current time in standard UTC.

### Fetch Nemesis Timestamp

{{ tutorial.code_snippet(['py:12:19', 'js:7:14']) }}

The Nemesis block creation time is a fixed network property and can be retrieved using the <get:/network/properties>
endpoint.
The returned value (`epochAdjustment`), after removing the `s` suffix and converting it to an integer, is a
[UNIX timestamp](https://en.wikipedia.org/wiki/Unix_time).
That is, the number of non-leap seconds that have elapsed since the Unix epoch (00:00:00 UTC on 1 January 1970).

This tutorial retrieves it from the network for illustration purposes, but this is a fixed value and can be
treated as a constant and hardcoded if desired.
For Symbol's <mainnet:|main network>, the value is `1615853185` which corresponds to `2021-03-16T00:06:25Z`
(March 16, 2021 at 12:06:25 AM UTC).

### Fetch Current Network Time

{{ tutorial.code_snippet(['py:22:27', 'js:17:22']) }}

The current network time, as understood by the queried node, is obtained using the <get:/node/time> endpoint.
Nodes in the network are typically synchronized, so they return similar times.

The actual property queried is `communicationTimestamps.receiveTimestamp`, which represents the time at which the
request was received by the node.

This value is expressed in <network time:>, that is, milliseconds elapsed since the Nemesis block was created.

### Conversion

The conversion from the current network time to UTC only requires adding the two numbers together,
taking care to use consistent units (seconds or milliseconds).

{{ tutorial.code_snippet(['py:29:34', 'js:24:29']) }}

This tutorial makes the addition manually to show the process.
If you are using the Symbol SDK, <dy:NetworkTimestampDatetimeConverter> provides a more convenient abstraction.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="5 7"
--8<-- 'devbook/chain/network-time.log'
```

Line 5 shows the Nemesis block timestamp, and always displays the same value.

Line 7 shows the current network time converted to UTC.

## Conclusion

This tutorial showed how to:

| Step                                                           | Related documentation                             |
|----------------------------------------------------------------|---------------------------------------------------|
| [Obtain the Nemesis timestamp](#fetch-nemesis-timestamp)       | <get:/network/properties>                         |
| [Obtain the current network time](#fetch-current-network-time) | <get:/node/time>                                  |
| [Combine both](#conversion)                                    | <dy:NetworkTimestampDatetimeConverter> (optional) |
