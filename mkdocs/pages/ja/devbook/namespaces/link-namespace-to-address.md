---
title: ネームスペースのアドレスへのリンク
tutorial_level: intermediate
---

# ネームスペースのアドレスへのリンクと解除

[ネームスペース](default:ネームスペース) は [アドレス](default:アドレス) にリンクさせることができます。これにより、[トランザクション](default:トランザクション) において長い16進数のアドレスの代わりに、人間が読み取り可能なエイリアス（別名）を使用できるようになります。

このチュートリアルでは、ネームスペースをアカウントのアドレスにリンクする方法と、不要になった際にリンクを解除する方法を説明します。

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

* 開発環境をセットアップしていること。
    [開発環境のセットアップ](../start/setup.md) を参照してください。
* ネームスペースを所有する [アカウント](default: アカウント) を、[コード](../accounts/create-from-private-key.md) または [ウォレット](../../userbook/wallet/create-account.md) を使用して作成していること。
* アドレスにリンクするための [ルートネームスペースを登録](./register-root-namespace.md) していること。
* トランザクション手数料を支払うための [XYM](default: XYM) を入手していること。
    [蛇口 (Faucet) からテストネットの通貨を入手する](../accounts/testnet-faucet.md) を参照してください。

さらに、トランザクションがどのようにアナウンスされ承認されるかを理解するために、[転送トランザクション](../transactions/transfer.md) のチュートリアルを復習しておいてください。

!!! info "ネームスペースの所有権が必要"
    ネームスペースをアドレスにリンクできるのは、そのネームスペースを所有しているアカウントのみです。
    ターゲットとなるアドレスによる連署や承認は必要ありません。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/namespaces/link_namespace_to_address', ['py', 'js']) }}

## コード解説 {: #code-explanation }

### アカウントの設定 {: #setting-up-the-account }

{{ tutorial.code_snippet_tagged('step-1') }}

このスニペットは、署名者の秘密鍵を `SIGNER_PRIVATE_KEY` 環境変数から読み取ります。設定されていない場合はデフォルトのテストキーが使用されます。
署名者のアドレスは公開鍵から派生します。
このアカウントは、リンク対象のネームスペースを所有している必要があります。

### ネームスペースとターゲットアドレスの定義 {: #defining-the-namespace-and-target-address }

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

* **ターゲットアドレス:** ネームスペースが指し示す先のアドレス。 `TARGET_ADDRESS` 環境変数から読み込まれます。
    設定されていない場合は、デフォルトのテストアドレスが使用されます。

### ネットワーク時間と手数料の取得 {: #fetching-network-time-and-fees }

{{ tutorial.code_snippet_tagged('step-3') }}

[転送トランザクション](../transactions/transfer.md) チュートリアルで説明されているプロセスに従い、ネットワーク時間と推奨手数料をそれぞれ <get:/node/time> および <get:/network/fees/transaction> から取得します。

### トランザクションの構築 {: #building-the-transaction }

{{ tutorial.code_snippet_tagged('step-4') }}

アドレスエイリアストランザクションでは以下を指定します。

* **Type:** アドレスエイリアストランザクションにはタイプ <ser:AddressAliasTransactionV1> を使用します。

* **署名者の公開鍵:** ネームスペースを所有し、トランザクション手数料を支払うアカウント。

* **ネームスペース ID:** リンクされるネームスペースの識別子。

* **アドレス:** ネームスペースにリンクするターゲットアドレス。

* **エイリアスアクション:** `link` という値はエイリアスを作成します。後にエイリアスを削除するには、代わりに `unlink` を使用します。

!!! info "エイリアスのリンク解除"
    ネームスペースのアドレスへのリンクを解除するには、同じネームスペース ID とアドレスを指定し、 `alias_action` フィールドを `unlink` に設定した別の <ser:AddressAliasTransactionV1> トランザクションをアナウンスします。

    リンク解除プロセスによってネームスペース自体が削除されるわけではなく、ネームスペースとアドレスの関連付けのみが削除されます。
    リンク解除後、そのネームスペースは別のアドレスや [モザイク](default: モザイク) にリンクさせることができます。

### トランザクションの送信 {: #submitting-the-transaction }

{{ tutorial.code_snippet_tagged('step-5') }}

トランザクションは、[転送トランザクションの作成](../transactions/transfer.md#announcing-the-transaction) と同じプロセスに従って署名され、アナウンスされます。

### エイリアスの検証 {: #verifying-the-alias }

{{ tutorial.code_snippet_tagged('step-6') }}

エイリアスが作成されたことを確認するために、コードは <get:/namespaces/{namespaceId}> エンドポイントを使用してネットワークからネームスペース情報を取得します。

レスポンスにはエイリアスタイプ（ `address` ）とリンクされたアドレスが含まれ、ネームスペースが指定したアドレスを指していることが確認されます。

### エイリアスの使用 {: #using-the-alias }

{{ tutorial.code_snippet_tagged('step-7') }}

ネームスペースがアドレスにリンクされると、トランザクションにおいてアドレスの代わりにネームスペースを使用できるようになります。
コードは、完全な16進数アドレスの代わりにエイリアスを受信者アドレスとして使用する [転送トランザクション](default: 転送トランザクション) の作成を示しています。

簡略化のため、この例ではトランザクションを作成しますが、アナウンスや承認の待機は行いません。

ネームスペースを受信者アドレスとして使用するには、 <dy:Address.fromNamespaceId> を使用してネームスペースIDを24バイトのアドレスに変換します。
[前のセクション](#defining-the-namespace-and-target-address)で説明したように、ネームスペースパスの最後のコンポーネントがネームスペースIDとして使用されます。

転送トランザクションのアナウンス方法の詳細については、[転送トランザクション](../transactions/transfer.md) チュートリアルを参照してください。

!!! note "アドレス解決レシート"
    ネットワークがネームスペースエイリアスを受信者アドレスとして使用するトランザクションを処理すると、**アドレス解決レシート（Address Resolution Receipt）** が生成されます。
    このレシートには、トランザクションが承認された時点でエイリアスが実際に指し示していたアドレスが記録されます。

    これは過去の監査可能性にとって重要です。エイリアスはいつでも変更または削除できるため、たとえエイリアスがその後更新されていたとしても、解決されたアドレスを常に検証できることがレシートによって保証されます。

    解決レシートは <get:/statements/resolutions/address> エンドポイントを使用して照会できます。
    レシートの詳細については、テキストブックの [解決ステートメント](../../textbook/blocks.md#resolution-statements) セクションを参照してください。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="3 5 23 32 33 36"
--8<-- 'devbook/namespaces/link_namespace_to_address.log'
```

出力の主なポイント:

* **ネームスペースとターゲット** (3、5行目): ネームスペース `nsaddr_1770541301` がターゲットアドレス `TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI` にリンクされています。

* **トランザクションハッシュ** (23行目): トランザクションハッシュを使用して、 [Symbol Testnet Explorer](https://testnet.symbol.fyi/) でトランザクションを検索できます。

* **エイリアスの検証** (32-33行目): ネームスペース情報により、エイリアスタイプが `2` (アドレス) であることが確認され、リンクされたアドレスが表示されています。

* **エイリアスの使用** (36行目): エイリアスを受信者として使用して転送トランザクションが作成されており、完全なアドレスの代わりに使用できることが実証されています。

    !!! note "異なる受信者アドレス"
        受信者アドレスがターゲットアドレスと異なるのは、それがターゲットアドレス自体ではなく [エンコードされたネームスペース ID](#using-the-alias) であるためです。
        ネットワークはトランザクションを処理する際に、エイリアスをリンクされたアドレスに解決します。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                             | 関連ドキュメント                             |
|------------------------------------------------------------------|----------------------------------------|
| [ネームスペース ID を生成する](#defining-the-namespace-and-target-address) | <dy:IdGenerator.generateNamespacePath> |
| [アドレスエイリアストランザクションを構築する](#building-the-transaction)            | <dy:SymbolTransactionFactory.create>   |
| [エイリアスを検証する](#verifying-the-alias)                             | <get:/namespaces/{namespaceId}>        |
| [エイリアスを使用する](#using-the-alias)                                 | <dy:Address.fromNamespaceId>           |
| [アドレス解決レシートを照会する](#using-the-alias)                          | <get:/statements/resolutions/address>  |
