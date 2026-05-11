---
title: ネームスペースのモザイクへのリンク
tutorial_level: intermediate
---

# ネームスペースのモザイクへのリンクと解除 {: #linking-and-unlinking-namespaces-to-mosaics }

[ネームスペース](default:ネームスペース) は [モザイク](default:モザイク) にリンクさせることができます。これにより、トランザクションにおいて長い16進数のモザイクIDの代わりに、人間が読み取り可能なエイリアス（別名）を使用できるようになります。

このチュートリアルでは、ネームスペースをモザイク識別子にリンクする方法と、不要になった際にリンクを解除する方法を説明します。

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

* 開発環境をセットアップしていること。
    [開発環境のセットアップ](../start/setup.md) を参照してください。
* ネームスペースを所有する [アカウント](default: アカウント) を、[コード](../accounts/create-from-private-key.md) または [ウォレット](../../userbook/wallet/create-account.md) を使用して作成していること。
* モザイクにリンクするための [ルートネームスペースを登録](./register-root-namespace.md) していること。
* リンク対象のモザイクIDを保有していること。新しく作成するか、既存のモザイクを使用できます。
* トランザクション手数料を支払うための [XYM](default: XYM) を入手していること。
    [蛇口 (Faucet) からテストネットの資金を入手する](../accounts/testnet-faucet.md) を参照してください。

さらに、トランザクションがどのようにアナウンスされ承認されるかを理解するために、[転送トランザクション](../transactions/transfer.md) のチュートリアルを復習しておいてください。

!!! info "ネームスペースとモザイクの両方の所有権が必要"
    ネームスペースとモザイクの両方を所有しているアカウントのみが、それらをリンクさせることができます。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/namespaces/link_namespace_to_mosaic', ['py', 'js']) }}

## コード解説 {: #code-explanation }

### アカウントの設定 {: #setting-up-the-account }

{{ tutorial.code_snippet_tagged('step-1') }}

このスニペットは、署名者の秘密鍵を `SIGNER_PRIVATE_KEY` 環境変数から読み取ります。設定されていない場合はデフォルトのテストキーが使用されます。
署名者のアドレスは公開鍵から派生します。
このアカウントは、リンクされるネームスペースとモザイクの両方を所有している必要があります。

### ネームスペースとターゲットモザイクの定義 {: #defining-the-namespace-and-target-mosaic }

{{ tutorial.code_snippet_tagged('step-2') }}

コードでは以下を定義しています。

* **ネームスペース名:** リンクするネームスペース。 `NAMESPACE_NAME` 環境変数から読み込まれ、設定されていない場合は `my_namespace` がデフォルトとなります。
    この名前は、自身のアカウントがすでに所有しているネームスペースと一致している必要があります。
* **ネームスペース ID:** ID は、 <dy:IdGenerator.generateNamespacePath> を使用してネームスペース名から生成されます。
    これは階層内の各レベルの ID 配列を返します。
    最終的なネームスペース ID を取得するために、最後の要素が選択されます。
    最後の要素を取得する方法は、ルートネームスペースとサブネームスペースの両方で機能します。

    `foo` のようなルートネームスペースの場合、配列には要素が１つ含まれます。
    `symbol.xym` のようなサブネームスペースの場合、要素は２つ含まれ、最後の要素が `symbol` 配下の `xym` の ID となります。

    !!! info "サブネームスペース ID は一意"
        サブネームスペース ID は階層的に派生するため、末尾の名前が同じでも親が異なる2つのサブネームスペースは、異なる ID を生成します。
        例えば、 `foo.xym` と `bar.xym` のパスの最後の要素は異なります。

* **モザイク ID:** ネームスペースが指し示す先のモザイクの16進数識別子。 `MOSAIC_ID` 環境変数から読み込まれます。設定されていない場合は、デフォルトのテスト用モザイクIDが使用されます。

### ネットワーク時間と手数料の取得 {: #fetching-network-time-and-fees }

{{ tutorial.code_snippet_tagged('step-3') }}

[転送トランザクション](../transactions/transfer.md) チュートリアルで説明されているプロセスに従い、ネットワーク時間と推奨手数料をそれぞれ <get:/node/time> および <get:/network/fees/transaction> から取得します。

### トランザクションの構築 {: #building-the-transaction }

{{ tutorial.code_snippet_tagged('step-4') }}

モザイクエイリアストランザクションでは以下を指定します。

* **Type:** モザイクエイリアストランザクションにはタイプ `mosaic_alias_transaction_v1` を使用します。

* **署名者の公開鍵:** ネームスペースとモザイクを所有し、トランザクション手数料を支払うアカウント。

* **ネームスペース ID:** リンクされるネームスペースの識別子。

* **モザイク ID:** ネームスペースにリンクするモザイクの識別子。

* **エイリアスアクション:** `link` という値はエイリアスを作成します。後にエイリアスを削除するには、代わりに `unlink` を使用します。

!!! info "エイリアスのリンク解除"
    ネームスペースのモザイクへのリンクを解除するには、同じネームスペース ID とモザイク ID を指定し、 `alias_action` フィールドを `unlink` に設定した別の `mosaic_alias_transaction_v1` トランザクションをアナウンスします。

    リンク解除プロセスによってネームスペースやモザイク自体が削除されるわけではなく、それらの関連付けのみが削除されます。
    リンク解除後、そのネームスペースは別のモザイクやアドレスにリンクさせることができます。

### トランザクションの送信 {: #submitting-the-transaction }

{{ tutorial.code_snippet_tagged('step-5') }}

トランザクションは、[転送トランザクションの作成](../transactions/transfer.md#announcing-the-transaction) と同じプロセスに従って署名され、アナウンスされます。

{{ tutorial.code_snippet_tagged('step-6') }}

コードはその後、ステータスが `confirmed` に変わるまで <get:/transactionStatus/{hash}> エンドポイントをポーリングして、トランザクションが承認されるのを待ちます。

### エイリアスの検証 {: #verifying-the-alias }

{{ tutorial.code_snippet_tagged('step-7') }}

エイリアスが作成されたことを確認するために、コードは <get:/namespaces/{namespaceId}> エンドポイントを使用してネットワークからネームスペース情報を取得します。

レスポンスにはエイリアスタイプ（ `mosaic` ）とリンクされたモザイクIDが含まれ、ネームスペースが指定したモザイクを指していることが確認されます。

### エイリアスの使用 {: #using-the-alias }

{{ tutorial.code_snippet_tagged('step-8') }}

ネームスペースがモザイクにリンクされると、トランザクション内でモザイクIDの代わりにネームスペースを使用できるようになります。
コードでは、16進数のモザイクIDではなく、モザイク配列内でエイリアスを使用した [転送トランザクション](default: 転送トランザクション) の作成を実演しています。

簡単にするため、この例ではモザイクを送信者自身のアドレスに送り返しており、トランザクションのアナウンスや承認の待機は行いません。

ネームスペースをモザイクIDとして使用するには、ネームスペース名を <dy:IdGenerator.generateMosaicAliasId> を使用してモザイクエイリアスIDに変換します。
[前のセクション](#defining-the-namespace-and-target-mosaic)で説明したように、ネームスペースパスの最後のコンポーネントがネームスペースIDとして使用されます。

転送トランザクションのアナウンス方法の詳細については、[転送トランザクション](../transactions/transfer.md) チュートリアルを参照してください。

!!! note "モザイク解決レシート"
    ネットワークがネームスペースエイリアスをモザイクIDとして使用するトランザクションを処理すると、**モザイク解決レシート（Mosaic Resolution Receipt）** が生成されます。
    このレシートには、トランザクションが承認された時点でエイリアスが実際に指し示していたモザイクIDが記録されます。

    これは過去の監査可能性にとって重要です。エイリアスはいつでも変更または削除できるため、たとえエイリアスがその後更新されていたとしても、解決されたモザイクIDを常に検証できることがレシートによって保証されます。

    解決レシートは <get:/statements/resolutions/mosaic> エンドポイントを使用して照会できます。
    レシートの詳細については、テキストブックの [解決ステートメント](../../textbook/blocks.md#resolution-statements) セクションを参照してください。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="3 5 23 32 33 36"
--8<-- 'devbook/namespaces/link_namespace_to_mosaic.log'
```

出力の主なポイント:

* **ネームスペースとターゲット** (3、5行目): ネームスペース `nsmos_1770541301` がターゲットのモザイクIDにリンクされています。

* **トランザクションハッシュ** (23行目): トランザクションハッシュを使用して、 [Symbol Testnet Explorer](https://testnet.symbol.fyi/) でトランザクションを検索できます。

* **エイリアスの検証** (32-33行目): ネームスペース情報により、エイリアスタイプが `1` (モザイク) であることが確認され、リンクされたモザイクIDが表示されています。

* **エイリアスの使用** (36行目): モザイク配列内でエイリアスを使用して転送トランザクションが作成されており、完全なモザイクIDの代わりに使用できることが実証されています。

    !!! note "異なるモザイク ID"
        転送で使用されているモザイクIDが元のモザイクIDと異なるのは、それがモザイクID自体ではなく [エンコードされたネームスペース ID](#using-the-alias) であるためです。
        ネットワークはトランザクションを処理する際に、エイリアスをリンクされたモザイクに解決します。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                            | 関連ドキュメント                             |
|-----------------------------------------------------------------|----------------------------------------|
| [ネームスペース ID を生成する](#defining-the-namespace-and-target-mosaic) | <dy:IdGenerator.generateNamespacePath> |
| [モザイクエイリアストランザクションを構築する](#building-the-transaction)           | <dy:SymbolTransactionFactory.create>   |
| [エイリアスを検証する](#verifying-the-alias)                            | <get:/namespaces/{namespaceId}>        |
| [転送内でエイリアスを使用する](#using-the-alias)                         | <dy:IdGenerator.generateMosaicAliasId> |
| [モザイク解決レシートを照会する](#using-the-alias)                         | <get:/statements/resolutions/mosaic>   |
