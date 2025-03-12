# Class: MosaicFlags

## Constructors

### new MosaicFlags()

```ts
new MosaicFlags(value): MosaicFlags
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

[`MosaicFlags`](MosaicFlags.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value-1"></a> `value` | `public` | `any` |
| <a id="none"></a> `NONE` | `static` | [`MosaicFlags`](MosaicFlags.md) |
| <a id="restrictable"></a> `RESTRICTABLE` | `static` | [`MosaicFlags`](MosaicFlags.md) |
| <a id="revokable"></a> `REVOKABLE` | `static` | [`MosaicFlags`](MosaicFlags.md) |
| <a id="supply_mutable"></a> `SUPPLY_MUTABLE` | `static` | [`MosaicFlags`](MosaicFlags.md) |
| <a id="transferable"></a> `TRANSFERABLE` | `static` | [`MosaicFlags`](MosaicFlags.md) |

## Accessors

### size

#### Get Signature

```ts
get size(): number
```

##### Returns

`number`

## Methods

### has()

```ts
has(flag): boolean
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `flag` | `any` |

#### Returns

`boolean`

***

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
static deserialize(payload): MosaicFlags
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`MosaicFlags`](MosaicFlags.md)

***

### deserializeAligned()

```ts
static deserializeAligned(payload): MosaicFlags
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`MosaicFlags`](MosaicFlags.md)
