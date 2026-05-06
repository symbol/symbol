---
title: ルートネームスペースの登録
tutorial_level: intermediate
---

# ルートネームスペースの登録 {: #registering-a-root-namespace }

[ネームスペース](default: ネームスペース) は、[アカウント](default: アカウント) や [モザイク](default:モザイク) に対して人間が読み取り可能なエイリアス（別名）を提供します。これにより、長いアドレスや16進数のモザイクIDの代わりに使用することができます。

このチュートリアルでは、[ルートネームスペース](default:ネームスペース) を登録し、そのレンタル [有効期間](../../textbook/namespaces.md#duration) を設定する方法を説明します。

一度登録した後は、[次のステップ](#next-steps) で説明するように、ネームスペースをモザイクやアカウントにリンクするための追加ステップが必要です。

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

* 開発環境をセットアップしていること。
  [開発環境のセットアップ](../start/setup.md) を参照してください。
* ネームスペースを登録するための [アカウント](default:アカウント) を、[コード](../accounts/create-from-private-key.md) または [ウォレット](../../userbook/wallet/create-account.md) を使用して作成していること。
* トランザクション手数料とレンタル手数料を支払うための [XYM](default: XYM) を入手していること。
  [蛇口 (Faucet) からテストネットの通貨を入手する](../accounts/testnet-faucet.md) を参照してください。

さらに、トランザクションがどのようにアナウンスされ承認されるかを理解するために、[転送トランザクション](../transactions/transfer.md) のチュートリアルを復習しておいてください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/namespaces/register-root-namespace', ['py', 'js']) }}

## コード解説 {: #code-explanation }

### アカウントの設定 {: #setting-up-the-account }

{{ tutorial.code_snippet_tagged('step-1') }}

このスニペットは、署名者の [秘密鍵](default: 秘密鍵) を `SIGNER_PRIVATE_KEY` 環境変数から読み取ります。設定されていない場合は、デフォルトのテストキーが使用されます。
署名者の [アドレス](default:アドレス) は [公開鍵](default:公開鍵) から派生します。
このアカウントが、登録されたネームスペースを所有することになります。

### ネットワーク時間と手数料の取得 {: #fetching-network-time-and-fees }

{{ tutorial.code_snippet_tagged('step-2') }}

[転送トランザクション](../transactions/transfer.md) チュートリアルで説明されているプロセスに従い、ネットワーク時間と推奨手数料をそれぞれ <get:/node/time> および <get:/network/fees/transaction> から取得します。

### トランザクションの構築 {: #building-the-transaction }

{{ tutorial.code_snippet_tagged('step-3') }}

ネームスペース登録トランザクションでは以下を指定します。

* **Type:** ネームスペース登録トランザクションにはタイプ `namespace_registration_transaction_v1` を使用します。

* **登録タイプ:** `root` という値は、ルートネームスペースが作成されることを示します。
    代わりに [サブネームスペースを登録](./register-subnamespace.md) するには `child` を使用してください。

* **有効期間:** ネームスペースがレンタルされるブロック数。
    最小期間は 86,400 ブロック（約30日）、最大期間は 5,256,000 ブロック（約5年）です。

* **名前:** ルートネームスペースの名前。
    名前には、英小文字、数字、ハイフン、アンダースコアのみを使用でき、数字または文字で始まる必要があり、最大64文字までです。

    チュートリアルを複数回実行してもネームスペース名が重複しないように、名前にタイムスタンプが付加されています。実用的なプログラムでは、ネームスペースに固定の名前を使用します。

!!! note "ネームスペースレンタル手数料"

    標準の [トランザクション手数料](#fetching-network-time-and-fees) に加えて、ネームスペースの登録には、要求された期間に比例した [レンタル手数料](../../textbook/namespaces.md#lease-fee) が必要です。

    トランザクション手数料とは異なり、レンタル手数料はトランザクションリクエストには **含まれません**。
    登録トランザクションが承認されると、ネットワークによって **トランザクション署名者のアカウント** から自動的に計算され、差し引かれます。

    レンタル手数料の額は、 <get:/network/fees/rental> エンドポイントを使用して事前に計算できます。

### トランザクションの送信 {: #submitting-the-transaction }

{{ tutorial.code_snippet_tagged('step-4') }}

トランザクションは、[転送トランザクションの作成](../transactions/transfer.md#announcing-the-transaction) と同じプロセスに従って署名され、アナウンスされます。

{{ tutorial.code_snippet_tagged('step-5') }}

コードはその後、ステータスが `confirmed` に変わるまで <get:/transactionStatus/{hash}> エンドポイントをポーリングして、トランザクションが承認されるのを待ちます。

### ネームスペースの取得 {: #retrieving-the-namespace }

{{ tutorial.code_snippet_tagged('step-6') }}

ネームスペースが登録されたことを確認するために、コードは <get:/namespaces/{namespaceId}> エンドポイントを使用してネットワークからネームスペースを取得し、そのプロパティを表示します。

ネームスペース ID は <dy:IdGenerator.generateNamespaceId> を使用して計算されます。
この関数はネームスペース名に決定論的なハッシュアルゴリズムを適用し、ネームスペース情報を照会するために必要な ID を生成します。

レスポンスが成功すれば、ネームスペースが登録され、ネットワーク上でアクティブであることが確認されます。

!!! info "ネームスペースは登録されましたが、まだリンクされていません"

    ネームスペースが [モザイク](default:モザイク) や [アカウント](default:アカウント) のエイリアスとして機能するとき、初めて有用なものとなります。
    [次のステップ](#next-steps) のガイドを使用して、ネームスペースを識別子にリンクしてください。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="7 15 18 20 30 33 34 35 36"
--8<-- 'devbook/namespaces/register-root-namespace.log'
```

出力の主なポイント:

* **ネームスペース名** (7行目): 選択された名前 `ns_1766533079` には、一意性を確保するためにタイムスタンプが含まれています。
    ネームスペースの詳細を表示するには、[Symbol Testnet Explorer](https://testnet.symbol.fyi/) でこの名前を検索してください。

* **手数料** (15行目): 0.0159 XYM のトランザクション手数料は、トランザクションサイズに手数料倍率を乗じて計算されます。 [レンタル手数料](../../textbook/namespaces.md#lease-fee) は、トランザクションが承認された際にネットワークによって別途差し引かれます。

* **ID と名前** (18、20行目): `id` フィールドにはネームスペース ID が10進数で表示され、 `name` には16進数文字列としてエンコードされたネームスペース名が含まれます。例えば、 `6e735f...` は `ns_1...` にデコードされます。

* **ネームスペース ID** (30行目): 18行目の `id` フィールドと一致するように、10進数と16進数の両方の表現が表示されます。

* **登録タイプ** (33行目): 値 `0` はルートネームスペースであることを示します（サブネームスペースの場合は `1` ）。

* **所有者アドレス** (34行目): ネームスペースを登録し、所有しているアカウント。

* **開始高と終了高** (35-36行目): ネームスペースはブロック `2984442` から `3073722` まで有効です。
    終了高には、要求された期間を超えて [猶予期間](../../textbook/namespaces.md#duration) （テストネットでは1日、メインネットでは30日）が含まれており、所有者がネームスペースを他の人に利用可能になる前に更新する時間を与えます。

出力に印刷されたトランザクション [ハッシュ](default: ハッシュ) を使用して、 [Symbol Testnet Explorer](https://testnet.symbol.fyi/) でトランザクションを検索することもできます。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                    | 関連ドキュメント                           |
|---------------------------------------------------------|--------------------------------------|
| [ネームスペース ID を生成する](#building-the-transaction)         | <dy:IdGenerator.generateNamespaceId> |
| [ネームスペース登録トランザクションを構築する](#building-the-transaction) | <dy:SymbolTransactionFactory.create> |
| [ネームスペースを取得する](#retrieving-the-namespace)             | <get:/namespaces/{namespaceId}>      |

## 次のステップ {: #next-steps }

ルートネームスペースを作成したので、以下のことができます。

* [ネームスペースをモザイクにリンクする](./link-namespace-to-mosaic.md) または [アカウントにリンクする](./link-namespace-to-address.md) ことで、エイリアスを作成する。
* [サブネームスペースを登録する](./register-subnamespace.md) ことで、階層構造を作成する。
* 有効期限が切れる前に [ネームスペースを延長する](./extend-root-namespace.md) ことで、アクティブな状態を維持する。
