# Class: TransactionStatement

## Constructors

### new TransactionStatement()

```ts
new TransactionStatement(): TransactionStatement
```

#### Returns

[`TransactionStatement`](TransactionStatement.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_primaryid"></a> `_primaryId` | `public` | `number` |
| <a id="_receipts"></a> `_receipts` | `public` | `any`[] |
| <a id="_secondaryid"></a> `_secondaryId` | `public` | `number` |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.receipts` | `public` | `string` |

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

### receipts

#### Get Signature

```ts
get receipts(): any[]
```

##### Returns

`any`[]

#### Set Signature

```ts
set receipts(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any`[] |

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
static deserialize(payload): TransactionStatement
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`TransactionStatement`](TransactionStatement.md)
