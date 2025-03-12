# Class: AliasAction

## Constructors

### new AliasAction()

```ts
new AliasAction(value): AliasAction
```

#### Parameters

| Parameter | Type |
| ------ | ------ |
| `value` | `any` |

#### Returns

[`AliasAction`](AliasAction.md)

## Properties

| Property | Modifier | Type |
| ------ | ------ | ------ |
| <a id="value-1"></a> `value` | `public` | `any` |
| <a id="link"></a> `LINK` | `static` | [`AliasAction`](AliasAction.md) |
| <a id="unlink"></a> `UNLINK` | `static` | [`AliasAction`](AliasAction.md) |

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
