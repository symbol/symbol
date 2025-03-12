# Class: ProofScalar

Represents a fixed size byte array.

## Extends

- [`ByteArray`](../../../../core/classes/ByteArray.md)

## Constructors

### new ProofScalar()

```ts
new ProofScalar(proofScalar?): ProofScalar
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `proofScalar`? | `Uint8Array`&lt;`ArrayBuffer`&gt; |

#### Returns

[`ProofScalar`](ProofScalar.md)

#### Overrides

[`ByteArray`](../../../../core/classes/ByteArray.md).[`constructor`](../../../../core/classes/ByteArray.md#constructors)

## Properties

| Property | Modifier | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="bytes"></a> `bytes` | `public` | `Uint8Array` | Underlying bytes. | [`ByteArray`](../../../../core/classes/ByteArray.md).[`bytes`](../../../../core/classes/ByteArray.md#bytes) |
| <a id="name"></a> `NAME` | `static` | `string` | Byte array name (required because `constructor.name` is dropped during minification). | [`ByteArray`](../../../../core/classes/ByteArray.md).[`NAME`](../../../../core/classes/ByteArray.md#name) |
| <a id="size"></a> `SIZE` | `static` | `number` | - | - |

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
toJson(): string
```

Returns representation of this object that can be stored in JSON.

#### Returns

`string`

JSON-safe representation of this object.

#### Inherited from

[`ByteArray`](../../../../core/classes/ByteArray.md).[`toJson`](../../../../core/classes/ByteArray.md#tojson)

***

### toString()

```ts
toString(): string
```

Returns string representation of this object.

#### Returns

`string`

String representation of this object

#### Inherited from

[`ByteArray`](../../../../core/classes/ByteArray.md).[`toString`](../../../../core/classes/ByteArray.md#tostring)

***

### deserialize()

```ts
static deserialize(payload): ProofScalar
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`ProofScalar`](ProofScalar.md)
