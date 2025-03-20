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

`MosaicFlags`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value"></a> `value` | `public` | `any` |
| <a id="none"></a> `NONE` | `static` | `MosaicFlags` |
| <a id="restrictable"></a> `RESTRICTABLE` | `static` | `MosaicFlags` |
| <a id="revokable"></a> `REVOKABLE` | `static` | `MosaicFlags` |
| <a id="supply_mutable"></a> `SUPPLY_MUTABLE` | `static` | `MosaicFlags` |
| <a id="transferable"></a> `TRANSFERABLE` | `static` | `MosaicFlags` |

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

`MosaicFlags`

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

`MosaicFlags`
