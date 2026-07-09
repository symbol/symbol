# Class: Hash256

Represents a fixed size byte array.

## Extends

- [`ByteArray`](../../index/ByteArray.md)

## Constructors

### Constructor

```ts
new Hash256(hash256?): Hash256;
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `hash256?` | `Uint8Array`&lt;`ArrayBuffer`&gt; |

#### Returns

`Hash256`

#### Overrides

[`ByteArray`](../../index/ByteArray.md).[`constructor`](../../index/ByteArray.md#constructor)

## Properties

| Property | Modifier | Type | Description | Inherited from |
| ------ | ------ | ------ | ------ | ------ |
| <a id="bytes"></a> `bytes` | `public` | `Uint8Array`&lt;`ArrayBuffer`&gt; | Underlying bytes. | [`ByteArray`](../../index/ByteArray.md).[`bytes`](../../index/ByteArray.md#bytes) |
| <a id="name"></a> `NAME` | `static` | `string` | Byte array name (required because `constructor.name` is dropped during minification). | [`ByteArray`](../../index/ByteArray.md).[`NAME`](../../index/ByteArray.md#name) |
| <a id="size"></a> `SIZE` | `static` | `number` | - | - |

## Accessors

### size

#### Get Signature

```ts
get size(): number;
```

##### Returns

`number`

## Methods

### serialize()

```ts
serialize(): Uint8Array<ArrayBuffer>;
```

#### Returns

`Uint8Array`&lt;`ArrayBuffer`&gt;

***

### toJson()

```ts
toJson(): string;
```

Returns representation of this object that can be stored in JSON.

#### Returns

`string`

JSON-safe representation of this object.

#### Inherited from

[`ByteArray`](../../index/ByteArray.md).[`toJson`](../../index/ByteArray.md#tojson)

***

### toString()

```ts
toString(): string;
```

Returns string representation of this object.

#### Returns

`string`

String representation of this object

#### Inherited from

[`ByteArray`](../../index/ByteArray.md).[`toString`](../../index/ByteArray.md#tostring)

***

### deserialize()

```ts
static deserialize(payload): Hash256;
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`Hash256`
