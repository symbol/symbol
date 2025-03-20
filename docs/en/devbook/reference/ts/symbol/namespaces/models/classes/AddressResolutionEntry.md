# Class: AddressResolutionEntry

## Constructors

### new AddressResolutionEntry()

```ts
new AddressResolutionEntry(): AddressResolutionEntry
```

#### Returns

`AddressResolutionEntry`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_resolvedvalue"></a> `_resolvedValue` | `public` | [`Address`](Address.md) |
| <a id="_source"></a> `_source` | `public` | [`ReceiptSource`](ReceiptSource.md) |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.resolvedValue` | `public` | `string` |
| `TYPE_HINTS.source` | `public` | `string` |

## Accessors

### resolvedValue

#### Get Signature

```ts
get resolvedValue(): Address
```

##### Returns

[`Address`](Address.md)

#### Set Signature

```ts
set resolvedValue(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Address`](Address.md) |

##### Returns

`void`

***

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

***

### source

#### Get Signature

```ts
get source(): ReceiptSource
```

##### Returns

[`ReceiptSource`](ReceiptSource.md)

#### Set Signature

```ts
set source(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`ReceiptSource`](ReceiptSource.md) |

##### Returns

`void`

## Methods

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

***

### sort()

```ts
sort(): void
```

#### Returns

`void`

***

### toJson()

```ts
toJson(): object
```

#### Returns

`object`

JSON-safe representation of this object.

***

### toString()

```ts
toString(): string
```

#### Returns

`string`

***

### deserialize()

```ts
static deserialize(payload): AddressResolutionEntry
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`AddressResolutionEntry`
