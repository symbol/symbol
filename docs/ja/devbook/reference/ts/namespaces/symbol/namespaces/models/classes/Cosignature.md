# Class: Cosignature

## Constructors

### new Cosignature()

```ts
new Cosignature(): Cosignature
```

#### Returns

[`Cosignature`](Cosignature.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_signature"></a> `_signature` | `public` | [`Signature`](Signature.md) |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) |
| <a id="_version"></a> `_version` | `public` | `bigint` |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.signature` | `public` | `string` |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` |

## Accessors

### signature

#### Get Signature

```ts
get signature(): Signature
```

##### Returns

[`Signature`](Signature.md)

#### Set Signature

```ts
set signature(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Signature`](Signature.md) |

##### Returns

`void`

***

### signerPublicKey

#### Get Signature

```ts
get signerPublicKey(): PublicKey
```

##### Returns

[`PublicKey`](PublicKey.md)

#### Set Signature

```ts
set signerPublicKey(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`PublicKey`](PublicKey.md) |

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

### version

#### Get Signature

```ts
get version(): bigint
```

##### Returns

`bigint`

#### Set Signature

```ts
set version(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `bigint` |

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
static deserialize(payload): Cosignature
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`Cosignature`](Cosignature.md)
