---
title: ネームスペースメタデータの追加
tutorial_level: advanced
---

# ネームスペースへのメタデータの追加 {: #adding-metadata-to-a-namespace }

[ネームスペース](default: ネームスペース) は、[アカウント](default:アカウント) や [モザイク](default:モザイク) と同様に、キーと値のペアとして [メタデータ](default:メタデータ) を保存できます。

このチュートリアルでは、ネームスペースにメタデータを追加し、ネットワークから取得し、既存の値を更新する方法を説明します。

この例では、ペア `description = My first namespace` をネームスペースに関連付け、その後 `Updated namespace` に変更します。

```dot
digraph {
    layout="neato";
    Namespace [label="ネームスペース\ntestnamespace" tooltip="ネームスペース" pos="0,0!"];
    Metadata [
        style=filled
        class=metadata
        label=<<table border="0"><tr><td><b>キー</b></td><td><b>値</b></td></tr><tr><td>description</td><td>My first namespace</td></tr></table>>
        tooltip="メタデータエントリ"
        shape=note
        pos="2.5,0.5!"];
}
```

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

* 開発環境をセットアップしていること。
    [開発環境のセットアップ](../start/setup.md) を参照してください。
* ネームスペースを所有するための [アカウント](default:アカウント) を、[コード](../accounts/create-from-private-key.md) または [ウォレット](../../userbook/wallet/create-account.md) を使用して作成していること。
* 署名者アカウントが所有する [ネームスペースを登録](./register-root-namespace.md) していること。
* [トランザクション](default:トランザクション) 手数料を支払うための [XYM](default:XYM) を入手していること。
    [蛇口 (Faucet) からテストネットの資金を入手する](../accounts/testnet-faucet.md) を参照してください。

さらに、トランザクションがどのようにアナウンスされ承認されるかを理解するために [転送トランザクション](../transactions/transfer.md) チュートリアルを、[アグリゲートトランザクション](default: アグリゲートトランザクション) の仕組みを理解するために [アグリゲートコンプリートトランザクション](../transactions/complete-aggregate.md) チュートリアルを復習しておいてください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/namespaces/namespace_metadata', ['py', 'js']) }}

## コード解説 {: #code-explanation }

このチュートリアルでは、ネームスペースに新しいメタデータを追加し、その後そのメタデータを更新する方法を実演します。

### アカウントとネームスペースのセットアップ {: #setting-up-the-account-and-namespace }

{{ tutorial.code_snippet_tagged('step-1') }}

このスニペットは、署名者の [秘密鍵](default: 秘密鍵) を `SIGNER_PRIVATE_KEY` 環境変数から読み取ります。設定されていない場合はデフォルトのテストキーが使用されます。
署名者の [アドレス](default: アドレス) は [公開鍵](default: 公開鍵) から派生します。

ネームスペース名は `NAMESPACE_NAME` 環境変数から読み取られ、設定されていない場合は `testnamespace` がデフォルトとなります。
ネームスペース ID は、名前から <dy:IdGenerator.generateNamespaceId> を使用して計算されます。

!!! note "メモ"
    ネームスペースはあらかじめ登録され、署名者アカウントによって所有されている必要があります。そうでない場合、メタデータを追加するトランザクションは拒否されます。

    作成方法については [ルートネームスペースの登録](./register-root-namespace.md) を参照してください。

### ネットワーク時間と手数料の取得 {: #fetching-network-time-and-fees }

{{ tutorial.code_snippet_tagged('step-2') }}

[転送トランザクション](../transactions/transfer.md) チュートリアルで説明されているプロセスに従い、ネットワーク時間と推奨手数料をそれぞれ <get:/node/time> および <get:/network/fees/transaction> から取得します。

### メタデータの定義 {: #defining-the-metadata }

各ネームスペースメタデータエントリは、以下の4つによって一意に識別されます。

* **署名者のアドレス**: メタデータを追加するアカウント。
* **ターゲットアカウントのアドレス**: ネームスペース所有者（このアカウントの署名が必要）。
* **ターゲットネームスペース ID**
* **スコープ指定されたメタデータキー**: メタデータ作成者によって選択される64ビットの値。

    SDKは、人間が読み取り可能な文字列からSHA3-256ハッシュを使用してこのキーを生成する <dy:Metadata.metadataGenerateKey> ヘルパー関数を提供しています。
    この方法により、キーがより意味のあるものになり、衝突の可能性が低減します。

{{ tutorial.code_snippet_tagged('step-3') }}

この例では、キーは文字列 `description` から派生します。
デモンストレーションのためにキー文字列にはタイムスタンプが付加されているため、コードを実行するたびに新しいエントリがネームスペースに追加されます。
実際には、作成または更新したい特定のメタデータエントリを識別する固定キーを使用します。

メタデータの値は、最大1024バイトまでの任意のシーケンスにすることができます。
この例では、値はUTF-8でエンコードされた文字列 `My first namespace` です。

!!! note "メモ"
    キーはメタデータエントリを識別する4つの要素のうちの1つにすぎないため、いずれかの要素が変更されると別のエントリになります。

    例えば、署名者のアドレスが異なれば、異なるアカウントが同じネームスペースに対して同じスコープ指定メタデータキーを競合なく使用できます。

    各エントリは独立しており、最初に作成したアカウントのみが更新できます。

### 埋め込みネームスペースメタデータトランザクションの作成 {: #creating-the-embedded-namespace-metadata-transaction }

{{ tutorial.code_snippet_tagged('step-4') }}

ネームスペースメタデータトランザクションは、ブロックチェーン上のネームスペースにキーと値のペアを関連付けます。
同じトランザクションタイプで、新しいメタデータエントリの追加と既存の更新の両方を処理します。

Symbolでは、これらのトランザクションを署名者アカウントとネームスペース所有者の両方の署名を含む [アグリゲートトランザクション](default: アグリゲートトランザクション) 内に含める必要があります。
これにより、所有者の許可なく不要なメタデータがネームスペースに関連付けられるのを防ぎます。

このチュートリアルでは、署名者がネームスペース所有者でもあるため、必要な署名は1つだけです。
しかし、トランザクションは依然としてアグリゲート内にある必要があるため、コードではネームスペースメタデータトランザクションを以下のプロパティを持つ [埋め込みトランザクション](default: 埋め込みトランザクション) として定義します。

* **Type:** `namespace_metadata_transaction_v1` を使用します。

* **署名者の公開鍵:** メタデータエントリを作成するアカウント。

* **ターゲットアドレス:** ネームスペース所有者のアドレス。
    署名者がネームスペース所有者と異なる場合、所有者はアグリゲートトランザクションに [連署](default: マルチシグアカウント) する必要があります。

* **ターゲットネームスペース ID:** メタデータを関連付けるネームスペース。

* **スコープ指定されたメタデータキー:** このメタデータエントリを識別するために使用される64ビットのキー。

* **値のサイズの差分 (Value size delta):** 新しいメタデータを作成する場合は、値のバイト長に設定します。
    既存のメタデータを更新する場合は、新しい値と現在の値の長さの差分に設定します。

* **値:** バイト形式のメタデータ内容。
    新しいメタデータを作成する場合は、生の値を指定します。
    更新する場合は、計算された値を指定します（[既存のメタデータの変更](#modifying-existing-metadata) セクションで説明します）。

### アグリゲートトランザクションの構築 {: #building-the-aggregate-transaction }

{{ tutorial.code_snippet_tagged('step-5') }}

[埋め込みトランザクション](default: 埋め込みトランザクション) を [アグリゲートトランザクション](default: アグリゲートトランザクション) に追加します。

署名者がネームスペースの所有者であるため、 [連署](default: 連署) は不要であり、アグリゲートは [コンプリート](default: アグリゲートコンプリートトランザクション) として作成できるため、すぐに署名してアナウンスできます。

!!! note "メモ"
    署名者がネームスペース所有者と異なる場合、メタデータエントリを承認するために所有者がアグリゲートトランザクションに連署する必要があります。

    オンチェーンで連署を収集する詳細については、[アグリゲートボンデッドトランザクション](../transactions/bonded-aggregate.md) チュートリアルを参照してください。

### アグリゲートトランザクションの送信 {: #submitting-the-aggregate-transaction }

{{ tutorial.code_snippet_tagged('step-6') }}

アグリゲートトランザクションは、[アグリゲートコンプリートトランザクションの作成](../transactions/complete-aggregate.md#building-the-aggregate-transaction) と同じプロセスに従って署名され、アナウンスされます。

### メタデータの取得 {: #retrieving-metadata }

{{ tutorial.code_snippet_tagged('step-7') }}

メタデータエントリの現在の値を取得するために、コードは `sourceAddress`、`targetAddress`、`scopedMetadataKey`、`targetId`（ネームスペース ID）、および `metadataType`（ネームスペースメタデータの場合は `2`）のフィルタを指定して <get:/metadata> エンドポイントを使用します。

エンドポイントはフィルタに一致するエントリのリストを返します。この例では単一のアイテムが含まれます。

### 既存のメタデータの変更 {: #modifying-existing-metadata }

{{ tutorial.code_snippet_tagged('step-8') }}

既存のメタデータエントリを更新するには、前述のようにネットワークから取得した現在の値が必要です。

メタデータの更新を実演するために、コードは同じスコープ指定メタデータキーを使用して別の `namespace_metadata_transaction_v1` トランザクションを作成し、説明を `My first namespace` から `Updated namespace` に変更します。

既存のメタデータ値を変更することは、新しい値が現在の値を基準として以下のフィールドを使用して定義される必要があるという点で、新規作成とは異なります。

* `value_size_delta`: 新しい値と現在の値の長さの差。
    この例では、`Updated namespace`（17バイト）は `My first namespace`（18バイト）より1バイト短いため、デルタは `-1` になります。

* `value`: 現在の値と新しい値をバイトごとに比較して計算された XOR されたバイト。

    SDKは XOR 計算を処理する <dy:Metadata.metadataUpdateValue> ヘルパー関数を提供しています。
    XOR 操作は各バイトを比較します。一致するバイトはゼロになり、異なるバイトが変更箇所を捉えます。

`value_size_delta` は、XOR されたバイト自体の長さではなく、最終的な値の長さの差（新 vs 旧）を表すことに注意してください。

!!! tip "ヒント"
    メタデータエントリを削除するには、`value_size_delta` を現在の値の長さの負の値に設定し、現在の値を `value` として提供します。XOR によって空の結果が生成され、ネットワークからエントリが削除されます。

[最初のメタデータ作成](#building-the-aggregate-transaction) と同様に、このメタデータの変更はアグリゲートトランザクションにラップされ、署名してアナウンスされます。

{{ tutorial.code_snippet_tagged('step-9') }}

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="3 4 18 19 20 23 37 47 48 50"
--8<-- 'devbook/namespaces/namespace_metadata.log'
```

出力の主なポイント:

* **3行目** (`Namespace name`): メタデータを受け取るネームスペース。
* **4行目** (`Namespace ID`): 計算されたネームスペース ID（10進数および16進数形式）。
* **18行目** (`"scoped_metadata_key"`): 入力文字列からSHA3-256ハッシュを使用して生成された64ビットのキー。
* **19行目** (`"target_namespace_id"`): メタデータを受け取るネームスペース。
* **20行目** (`"value_size_delta": 18`): 新しいメタデータを作成する場合、これは値のバイト長に等しくなります（`"My first namespace"` = 18バイト）。
* **23行目**: エクスプローラーでメタデータの作成を確認するためのトランザクション [ハッシュ](default: ハッシュ)。
* **37行目** (`Current value: My first namespace`): 更新前にネットワークから取得された値。
* **47行目** (`"value_size_delta": -1`): 新しい値（`"Updated namespace"` = 17バイト）が現在の値（18バイト）より短いため、負の値になります。
* **48行目** (`"value"`): 生の新しい値ではなく、現在の値と新しい値から計算された XOR 値。
* **50行目**: エクスプローラーでメタデータの更新を確認するためのトランザクションハッシュ。

トランザクションハッシュを使用して、[Symbol Testnet Explorer](https://testnet.symbol.fyi/) でトランザクションを検索できます。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                                               | 関連ドキュメント                                   |
|------------------------------------------------------------------------------------|----------------------------------------------|
| [メタデータのキーと値の定義](#defining-the-metadata)                                         | <dy:Metadata.metadataGenerateKey>            |
| [ネームスペースメタデータトランザクションの作成](#creating-the-embedded-namespace-metadata-transaction) | <dy:SymbolTransactionFactory.createEmbedded> |
| [メタデータの取得](#retrieving-metadata)                                                 | <get:/metadata>                              |
| [既存のメタデータの変更](#modifying-existing-metadata)                                    | <dy:Metadata.metadataUpdateValue>            |
