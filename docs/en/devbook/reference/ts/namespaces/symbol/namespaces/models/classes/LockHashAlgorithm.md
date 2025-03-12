# Class: LockHashAlgorithm

## Constructors

### new LockHashAlgorithm()

```ts
new LockHashAlgorithm(value): LockHashAlgorithm
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

[`LockHashAlgorithm`](LockHashAlgorithm.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value-1"></a> `value` | `public` | `any` |
| <a id="hash_160"></a> `HASH_160` | `static` | [`LockHashAlgorithm`](LockHashAlgorithm.md) |
| <a id="hash_256"></a> `HASH_256` | `static` | [`LockHashAlgorithm`](LockHashAlgorithm.md) |
| <a id="sha3_256"></a> `SHA3_256` | `static` | [`LockHashAlgorithm`](LockHashAlgorithm.md) |

## Accessors

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

## Methods

### serialize()

```ts
serialize(): Uint8Array<ArrayBufferLike>
```

#### Returns

`Uint8Array`&lt;`ArrayBufferLike`&gt;

***

### toJson()

```ts
toJson(): any
```

#### Returns

`any`

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
static deserialize(payload): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`any`

***

### deserializeAligned()

```ts
static deserializeAligned(payload): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`any`

***

### fromValue()

```ts
static fromValue(value): any
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

`any`

***

### valueToKey()

```ts
static valueToKey(value): string
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

`string`
