# Class: DetachedCosignature

## Constructors

### new DetachedCosignature()

```ts
new DetachedCosignature(): DetachedCosignature
```

#### Returns

[`DetachedCosignature`](DetachedCosignature.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_parenthash"></a> `_parentHash` | `public` | [`Hash256`](Hash256.md) |
| <a id="_signature"></a> `_signature` | `public` | [`Signature`](Signature.md) |
| <a id="_signerpublickey"></a> `_signerPublicKey` | `public` | [`PublicKey`](PublicKey.md) |
| <a id="_version"></a> `_version` | `public` | `bigint` |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.parentHash` | `public` | `string` |
| `TYPE_HINTS.signature` | `public` | `string` |
| `TYPE_HINTS.signerPublicKey` | `public` | `string` |

## Accessors

### parentHash

#### Get Signature

```ts
get parentHash(): Hash256
```

##### Returns

[`Hash256`](Hash256.md)

#### Set Signature

```ts
set parentHash(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`Hash256`](Hash256.md) |

##### Returns

`void`

***

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
static deserialize(payload): DetachedCosignature
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`DetachedCosignature`](DetachedCosignature.md)
