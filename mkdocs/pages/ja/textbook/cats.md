# CATS DSL {: #cats-dsl }

CATS
:   **CATS DSL**（ユーモラスな再帰頭字語として **Compact Affinitized Transfer Schema**、
    すなわち **ドメイン特化言語（Domain-Specific Language）** の略）は、
    構造化データのバイナリ形式を定義するための、簡潔で記述的な言語です。

元々は Symbol と NEM のために開発され、両プロトコルにおけるすべての
[ブロック](default:ブロック) と [トランザクション](default:トランザクション) を記述するために使用されているが、
任意のバイナリ形式を表現できる汎用的な設計になっています。

CATS はサイズ効率、パフォーマンス、厳密な型付けを重視しており、
可能であればゼロコピーのデシリアライズ（復元）を目指している。
特徴として、固定サイズバッファ、厳密な型エイリアス、インライン構造体、条件付きフィールドなどがあります。

CATS の定義は「ジェネレータ」と呼ばれるツールによって処理されます。
ジェネレータは、CATS で定義されたバイナリ構造をシリアライズ（書き込み）および
デシリアライズ（読み込み）するコードを、特定のプログラミング言語向けに生成します。

現在、Python および JavaScript／TypeScript 向けのジェネレータが存在し、
Java 向けのジェネレータも開発中です（２０２５年６月時点）。
これらは Symbol の SDK により使用され、プラットフォーム間で一貫した効率的なバイナリエンコードを保証しています。

このページでは CATS DSL の構文と機能を説明します。
正確な定義は、Symbol のソースリポジトリに含まれています。
[厳密な文法](https://github.com/symbol/symbol/blob/dev/catbuffer/parser/catparser/grammar/catbuffer.lark) を参照してください。
この文法は [Lark parsing language](https://lark-parser.readthedocs.io) を用いて記述されています。

!!! note "空白"

    すべての CATS 文は改行で終わります（セミコロンは使用しない）が、その他の空白文字は本質的ではないです。

    インデントはパーサー上は必須ではありませんが、可読性のために慣例的に用いられています。

１つの CATS ファイルは４つのトップレベルキーワードから構成されています：`import`、`using`、`enum`、`struct`。
それぞれについて以下で説明します。

## `import` {: #import }

CATS ファイルは `import` 文を使って他の CATS ファイルを読み込むことができます。
これによりスキーマ定義をモジュール化し、再利用しやすくできます。

他の CATS ファイルを読み込むには、そのファイル名を引用符で指定します。

```cats
import "other.cats"
```

インポートされるファイル名は、パーサーに渡されたインクルードパスを基準に解決されます。

## `using` {: #using }

`using` 文は組み込みプリミティブ型に対する**型エイリアス**を定義します。
これらのエイリアスはパーサーおよびジェネレータにおいて独立した型として扱われるため、
同じ下位表現を共有していても厳密な型付けが可能になります。

```cats
using <TypeAlias> = <Built-in type>
```

CATS では、次の２系統の組み込み型に対してエイリアスを定義できます。

* **整数型**
    * 符号なし: `uint8`, `uint16`, `uint32`, `uint64`
    * 符号あり: `int8`, `int16`, `int32`, `int64`

* **固定サイズのバイナリバッファ**
    * `binary_fixed(N)` は N バイト長の固定バッファを表します。

例えば、８バイトの符号なし整数として `Height` 型を定義するには次のように書きます。

```cats
using Height = uint64
```

３２バイトのバイナリバッファとして `PublicKey` 型を定義するには次のように書きます。

```cats
using PublicKey = binary_fixed(32)
```

以下の例では `Height` と `Weight` はどちらも `uint64` を基にしていますが、
これらは**別個の型**として扱われ、互換的に使用することはできません。

```cats
using Height = uint64
using Weight = uint64
```

## `enum` {: #enum }

`enum` 文は**列挙型**を定義します。
列挙型は、整数型を基盤とする名前付き定数の集合です。

各列挙型では、基盤となる整数型を明示的に指定しなければなりません。
いずれの組み込み整数型も使用できます。

```cats
enum <TypeName> : <Backing type>
    <ConstantName> = <Value>
    ...
```

列挙のメンバーは `enum` 宣言の行の次から定義します。
各メンバーには定数の整数値を割り当てる必要があります。

例えば、３２ビット符号なし整数を基盤型とする `TransportMode` 列挙型は次のように定義できます。

```cats
enum TransportMode : uint32
    ROAD = 0x0001
    SEA = 0x0002
    SKY = 0x0004
```

### 列挙型の属性 {: #enum-attributes }

列挙型には振る舞いを変更する属性を付与できます。
各属性は `@` で始まり、列挙型宣言の直前の行に記述します。
現時点でサポートされている属性は次のとおりです。

* `@is_bitwise`
    列挙型がビットフィールド（フラグの集合）であることを示し、
    生成コードにビット演算サポートを追加します。

    例：

    ```cats
    @is_bitwise
    enum TransportMode : uint32
        ROAD = 0x0001
        SEA = 0x0002
        SKY = 0x0004
    ```

    この属性は、列挙値同士をビット OR で組み合わせたり、
    個々のフラグをビット AND で検査したりすることができることを
    ジェネレータに伝えます。

## `struct` {: #struct }

`struct` 文は、名前付きフィールドの集合として**構造化されたバイナリレイアウト**を定義します。

構造体は CATS の最も重要な構成要素であり、
[トランザクション](default:トランザクション)、[ブロック](default:ブロック)、
その他すべての複合オブジェクトを記述するために使用されます。

構造体の宣言は `struct` キーワードから始まり、必要に応じてその前に「修飾子」を付けることができます。
その後の行で、各フィールドに名前と型を与えて列挙します。

```cats
[Optional modifier] struct <StructName>
    <FieldName> = <FieldType>
    ...
```

例：

```cats
struct Vehicle
    weight = uint32
    wheel_count = uint8
```

### 修飾子 {: #modifiers }

CATS は次の修飾子をサポートします。

* `abstract`
    継承のための基本構造体を定義します。
    ジェネレータは適切な派生型をインスタンス化するファクトリを生成します。

* `inline`
    この構造体は単なる構成用途としてのみ使用され、独立した型として出力しません。

いずれの修飾子も指定されない場合、その構造体はそのまま生成結果に含まれます。

### 特別なフィールドコンストラクタ {: #special-field-constructors }

フィールドは型の代わりに、特別なコンストラクタを使って宣言することもできます。

* `make_const(type, value)`
    定数を定義します。
    このフィールド自体はレイアウトには現れず、生成コード内で
    `<StructName>.<FieldName>` として参照できる定数となります。

    次の例では、`TRANSPORT_MODE` はシリアライズされないが、型 `TransportMode`、値 `ROAD` の
    `Car.TRANSPORT_MODE` という定数として扱われます。

    ```cats
    struct Car
        TRANSPORT_MODE = make_const(TransportMode, ROAD)
    ```

* `make_reserved(type, value)`
    固定値を持つ予約フィールドを定義します。
    このフィールドはレイアウト上に保持され、常に指定した値になります。

    以下の例では、`wheel_count` は常に値 `4` を持つ `uint8` として格納されます。

    ```cats
    struct Car
        wheel_count = make_reserved(uint8, 4)
    ```

* `sizeof(type, reference)`
    別のフィールドのサイズ（バイト数）を自動的に格納するフィールドを定義します。
    これにより、参照先の型を変更してもサイズ用のフィールドを手動で更新する必要がなくなります。

    次の例では、`car_size` は常にフィールド `car` のバイトサイズを保持します `uint16` となります。
    `car` の型は `Car` です。

    ```cats
    struct SingleCarGarage
        car_size = sizeof(uint16, car)
        car = Car
    ```

### 条件付きフィールド {: #conditional-fields }

フィールドは、別のフィールドの値に基づいて**条件付き**で存在させることができます。
これは、他言語における共用体（union）に類似した、排他的なレイアウトを表現する際に有用です。

条件付きフィールドは次の構文を使用します。

```cats
    <FieldName> = <FieldType> if <ConstantValue> <Operator> <SelectorField>
```

CATS は次の条件演算子をサポートします。

* `equals`
    セレクタフィールドが定数値と等しい場合にフィールドを含めます。

* `not equals`
    セレクタフィールドが定数値と等しくない場合にフィールドを含めます。

* `has`
    セレクタフィールドが、定数値のすべてのビットを持っている場合（ビットフラグ用）にフィールドを含めます。

* `not has`
    セレクタフィールドに、定数値のビットの一部でも欠けている場合にフィールドを含めます。

例えば、`buoyancy` フィールドは `transport_mode` が `SEA` と等しい場合にのみ含まれます。

```cats
struct Vehicle
    transport_mode = TransportMode

    buoyancy = uint32 if SEA equals transport_mode
```

### 配列フィールド {: #array-fields }

CATS は、すべての要素が同じ型である固定長配列および可変長配列の両方をサポートします。

構文は次のとおりです。

```cats
    <FieldName> = array(<ElementType>, <NumberOfElements>)
```

`<NumberOfElements>` には次のいずれかを指定できます。

* 定数
    固定長配列を生成します。

    ```cats
    struct SmallGarage
        vehicles = array(Vehicle, 4)
    ```

* 別フィールドへの参照
    可変長配列を生成します。

    例えば、次の構造体では、`vehicles` フィールドは `vehicles_count` 個の `Vehicle` を含みます。

    ```cats
    struct Garage
        vehicles_count = uint32
        vehicles = array(Vehicle, vehicles_count)
    ```

* 特殊キーワード `__FILL__`
    構造体の末尾まで配列を伸ばすことを示します。

    この場合、構造体には `@size` 属性（[後述](#struct-attributes)）が必要であり、
    構造体全体のバイトサイズを保持するフィールドを参照します。

    ```cats
    @size(garage_byte_size) struct Garage
        garage_byte_size = uint32
        vehicles = array(Vehicle, __FILL__)
    ```

!!! note "注意"

    `ElementType` は次のいずれかでなければなりません。

    * 固定サイズの構造体
    * 自身の `@size` 属性でサイズが注釈されている可変サイズの構造体

    そうでない場合、パーサーはバイト列から要素数を正しく判断できません。

#### 配列フィールドの属性 {: #array-field-attributes }

配列フィールドには、サイズ管理やアラインメント、並べ替えを制御する属性を付けることができます。

サポートされる属性には次があります。

* `@is_byte_constrained`
    配列サイズを要素数ではなくバイト数として解釈します。

* `@alignment(x [, [not] pad_last])`
    各要素を `x` バイト境界に揃える。オプションで末尾要素のパディングを制御できます。

    デフォルトでは、アラインメントが指定された場合、最後の要素にもパディングが入ります。
    `not pad_last` 修飾子を付けることで、末尾要素へのパディングを無効化できます。

* `@sort_key(x)`
    配列を指定したプロパティでソートします。

    例えば、次の配列は `Vehicle` 構造体を weight 項目でソートし、
    ８バイト境界に揃え、かつ末尾要素はパディングしません。

    ```cats
    struct Garage
        @sort_key(weight)
        @alignment(8, not pad_last)
        vehicles = array(Vehicle, __FILL__)
    ```

### インライン {: #inlines }

構造体は `inline` 修飾子を使って、別の構造体の中に**インライン展開**できます。
これにより、ある構造体のフィールドをネストせずに直接別の構造体へ埋め込むことができます。

例えば、次の定義では `Vehicle` の内容が `Car` の中にインライン展開されます。

```cats
struct Vehicle
    weight = uint32

struct Car
    inline Vehicle
    max_clearance = Height
    has_left_steering_wheel = uint8
```

フィールドはその場に展開されるため、最終的な `Car` のレイアウトは次と同等になります。

```cats
struct Car
    weight = uint32
    max_clearance = Height
    has_left_steering_wheel = uint8
```

!!! note "名前付きインライン"

    構造体は**名前付き**でインライン展開することもでき、その場合はフィールド名にプレフィックスが付与されます。

    ```cats
    <FieldName> = inline <StructName>
    ```

    次の例では、`SizePrefixedString` が `Vehicle` 内で `friendly_name` としてインライン展開されます。

    ```cats
    struct SizePrefixedString
        size = uint32
        __value__ = array(int8, size)

    struct Vehicle
        weight = uint32
        friendly_name = inline SizePrefixedString
        year = uint16
    ```

    これは次のように展開されます。

    ```cats
    struct Vehicle
        weight = uint32
        friendly_name_size = uint32
        friendly_name = array(int8, friendly_name_size)
        year = uint16
    ```

    特別なフィールド `__value__` は、指定したインライン名（この場合は `friendly_name`）に置き換えられます。
    その他のフィールドは、その名前にプレフィックスとアンダースコアが付いた名前に変更されます。
    上の例では、`size` は `friendly_name_size` になります。

### 構造体の属性 {: #struct-attributes }

構造体には、コードジェネレータへのヒントやレイアウト上の動作に影響を与える属性を付与できます。
属性は `@` で始まり、`struct` 宣言の直前に記述します。

CATS がサポートする構造体レベルの属性は次のとおりです。

* `@is_aligned`
    すべてのフィールドをその自然な境界に揃えます。

* `@is_size_implicit`
    構造体を `sizeof(x)` 式で参照できるようにします。

* `@size(x)`
    フィールド `x` が、その構造体全体のサイズ（バイト数）を保持していることを宣言します。

* `@initializes(x, Y)`
    構造体のフィールド `x` を、別の場所で定義された定数 `Y` で初期化します。

* `@discriminator(x [, y...])`
    `abstract` 構造体と組み合わせて使用し、
    デコード時にどの派生型をインスタンス化するかを、指定したプロパティに基づいて選択します。

* `@comparer(x [!transform] [, y...])`
    インスタンスの並べ替えや比較に使用するプロパティを定義します。
    オプションで、比較前に適用する変換を指定できます。
    現在サポートされている変換は、NEM との後方互換性のための `ripemd_keccak_256` のみです。

例えば、次の例では `Vehicle` 内の `transport_mode` フィールドを、派生構造体側で定義される定数に関連付けています。

```cats
@initializes(transport_mode, TRANSPORT_MODE)
abstract struct Vehicle
    transport_mode = TransportMode

struct Car
    TRANSPORT_MODE = make_const(TransportMode, ROAD)
    inline Vehicle
```

定数 `TRANSPORT_MODE` は `Vehicle` を拡張する任意の構造体で定義できます。

### 整数フィールドの属性 {: #integer-field-attributes }

整数フィールドは次の属性をサポートします。

* `@sizeref(x [, y])`
    フィールドの値を `x` のサイズに設定し、必要に応じてオフセット `y` を加えます。

    例えば、`vehicle_size` および `vehicle` の合計サイズを格納するには次のようにします。

    ```cats
    struct Garage
        @sizeref(vehicle, 2)
        vehicle_size = uint16
        vehicle = Vehicle
    ```

## コメント {: #comments }

`#` で始まる行はコメントとして扱われます。

宣言の直前に無いコメントはパーサーによって無視されます。
一方、宣言またはフィールドの直前に置かれたコメントは**ドキュメント**として扱われ、
生成されたコードに含められる場合があります。

例：

```cats
# This comment is ignored {: #this-comment-is-ignored }

# This comment is included as documentation {: #this-comment-is-included-as-documentation }
# and will be associated with the `Height` alias. {: #and-will-be-associated-with-the-height-alias }
using Height = uint64
```

この仕組みにより、バイナリレイアウトに影響を与えることなく、
スキーマにインラインの説明文を追加することができます。
