# Class: BlockStatement

## Constructors

### new BlockStatement()

```ts
new BlockStatement(): BlockStatement
```

#### Returns

[`BlockStatement`](BlockStatement.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="_addressresolutionstatements"></a> `_addressResolutionStatements` | `public` | `any`[] |
| <a id="_mosaicresolutionstatements"></a> `_mosaicResolutionStatements` | `public` | `any`[] |
| <a id="_transactionstatements"></a> `_transactionStatements` | `public` | `any`[] |
| <a id="type_hints"></a> `TYPE_HINTS` | `static` | `object` |
| `TYPE_HINTS.addressResolutionStatements` | `public` | `string` |
| `TYPE_HINTS.mosaicResolutionStatements` | `public` | `string` |
| `TYPE_HINTS.transactionStatements` | `public` | `string` |

## Accessors

### addressResolutionStatements

#### Get Signature

```ts
get addressResolutionStatements(): any[]
```

##### Returns

`any`[]

#### Set Signature

```ts
set addressResolutionStatements(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any`[] |

##### Returns

`void`

***

### mosaicResolutionStatements

#### Get Signature

```ts
get mosaicResolutionStatements(): any[]
```

##### Returns

`any`[]

#### Set Signature

```ts
set mosaicResolutionStatements(value): void
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

### transactionStatements

#### Get Signature

```ts
get transactionStatements(): any[]
```

##### Returns

`any`[]

#### Set Signature

```ts
set transactionStatements(value): void
```

##### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any`[] |

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
static deserialize(payload): BlockStatement
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `payload` | `any` |

#### Returns

[`BlockStatement`](BlockStatement.md)
