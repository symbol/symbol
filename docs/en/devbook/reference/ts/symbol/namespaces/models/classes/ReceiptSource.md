# Class: ReceiptSource

## Constructors

### new ReceiptSource()

```ts
new ReceiptSource(): ReceiptSource
```

#### Returns

`ReceiptSource`

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_primaryid"></a> `_primaryId` | `public` | `number` |
| <a id="_secondaryid"></a> `_secondaryId` | `public` | `number` |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |

## Accessors

### primaryId

#### Get Signature

```ts
get primaryId(): number
```

##### Returns

`number`

#### Set Signature

```ts
set primaryId(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `number` |

##### Returns

`void`

***

### secondaryId

#### Get Signature

```ts
get secondaryId(): number
```

##### Returns

`number`

#### Set Signature

```ts
set secondaryId(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `number` |

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
static deserialize(payload): ReceiptSource
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

`ReceiptSource`
