# Class: VrfProof

## Constructors

### new VrfProof()

```ts
new VrfProof(): VrfProof
```

#### Returns

`VrfProof`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_gamma"></a> `_gamma` | `public` | [`ProofGamma`](ProofGamma.md) |
| <a id="_scalar"></a> `_scalar` | `public` | [`ProofScalar`](ProofScalar.md) |
| <a id="_verificationhash"></a> `_verificationHash` | `public` | [`ProofVerificationHash`](ProofVerificationHash.md) |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.gamma` | `public` | `string` |
| `TYPE_HINTS.scalar` | `public` | `string` |
| `TYPE_HINTS.verificationHash` | `public` | `string` |

## Accessors

### gamma

#### Get Signature

```ts
get gamma(): ProofGamma
```

##### Returns

[`ProofGamma`](ProofGamma.md)

#### Set Signature

```ts
set gamma(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`ProofGamma`](ProofGamma.md) |

##### Returns

`void`

***

### scalar

#### Get Signature

```ts
get scalar(): ProofScalar
```

##### Returns

[`ProofScalar`](ProofScalar.md)

#### Set Signature

```ts
set scalar(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`ProofScalar`](ProofScalar.md) |

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

### verificationHash

#### Get Signature

```ts
get verificationHash(): ProofVerificationHash
```

##### Returns

[`ProofVerificationHash`](ProofVerificationHash.md)

#### Set Signature

```ts
set verificationHash(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`ProofVerificationHash`](ProofVerificationHash.md) |

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
static deserialize(payload): VrfProof
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`VrfProof`
