# Class: MosaicResolutionStatement

## Constructors

### new MosaicResolutionStatement()

```ts
new MosaicResolutionStatement(): MosaicResolutionStatement
```

#### Returns

`MosaicResolutionStatement`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_resolutionentries"></a> `_resolutionEntries` | `public` | `any`[] |
| <a id="_unresolved"></a> `_unresolved` | `public` | [`UnresolvedMosaicId`](UnresolvedMosaicId.md) |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.resolutionEntries` | `public` | `string` |
| `TYPE_HINTS.unresolved` | `public` | `string` |

## Accessors

### resolutionEntries

#### Get Signature

```ts
get resolutionEntries(): any[]
```

##### Returns

`any`[]

#### Set Signature

```ts
set resolutionEntries(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any`[] |

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

### unresolved

#### Get Signature

```ts
get unresolved(): UnresolvedMosaicId
```

##### Returns

[`UnresolvedMosaicId`](UnresolvedMosaicId.md)

#### Set Signature

```ts
set unresolved(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | [`UnresolvedMosaicId`](UnresolvedMosaicId.md) |

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
static deserialize(payload): MosaicResolutionStatement
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`MosaicResolutionStatement`
