---
title: トランザクションエラー
---

# トランザクションエラーのリスニング {: #listening-to-transaction-errors }

<ws:status&#47;{address}> WebSocket チャネルは、特定の [アカウント](default: アカウント) に関連する [トランザクション](default: トランザクション) がネットワークによって拒否されたときに、リアルタイムの通知を送信します。
<get:/transactionStatus/{hash}> エンドポイントをポーリングする代わりに、 `status` チャネルはネットワークがトランザクションを拒否するとすぐにエラーの詳細をプッシュします。

このチュートリアルでは、 `status` チャネルをサブスクライブし、拒否通知を処理する方法を説明します。
リスナーをテストするために、コードはネットワークが拒否する無効なトランザクションを意図的に送信します。

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

* 開発環境をセットアップしていること。
  [開発環境のセットアップ](../start/setup.md) を参照してください。
* 監視するアカウントのアドレスを保有していること。
  [秘密鍵からのアカウント作成](../accounts/create-from-private-key.md) または
  [ウォレットを使用したアカウントの作成](../../userbook/wallet/create-account.md) を参照してください。

さらに、言語固有の WebSocket ライブラリをインストールしてください。

=== ":simple-python: Python"

    `websockets` ライブラリをインストールします。

    ```bash
    pip install websockets
    ```

=== ":simple-javascript: JavaScript"

    このチュートリアルでは、Node.js 22 以降で利用可能なネイティブの `WebSocket` API を使用します。
    追加のパッケージは必要ありません。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/websockets/listen-transaction-error', ['py', 'js']) }}

このスニペットでは、 `NODE_URL` 環境変数を使用して Symbol API [ノード] (default: ノード) を設定します。
値が指定されない場合は、デフォルト値が使用されます。
WebSocket URL は、HTTP プロトコルを WebSocket プロトコルに置き換え、 `/ws` を追加することで `NODE_URL` から派生します。

## コード解説 {: #code-explanation }

### 監視対象アドレスと署名者の設定 {: #setting-up-the-monitored-address-and-signer }

{{ tutorial.code_snippet(['py:17:27', 'js:14:22']) }}

`status` チャネルは、特定のアドレスをスコープとします。
`MONITOR_ADDRESS` 環境変数は、監視するアドレスを設定します。
このチャネルは、送信者、受信者、またはトランザクションの内容から派生したその他の役割（例えば、 [アグリゲートトランザクション](default: アグリゲートトランザクション) 内の埋め込みトランザクションの署名者など）を問わず、そのアドレスが拒否されたトランザクションに関与するたびに通知を行います。
拒否をトリガーするために、このチュートリアルでは存在しないモザイクを含む転送トランザクションを送信し、 `SIGNER_PRIVATE_KEY` の秘密鍵で署名します。

これらの環境変数のいずれかが提供されない場合、チュートリアルは同じアカウントに対応するデフォルト値を提供します。

### WebSocket への接続 {: #connecting-to-the-websocket }

{{ tutorial.code_snippet(['py:31:35', 'js:25:33']) }}

コードは、ノードの `/ws` エンドポイントへの WebSocket 接続を開きます。
接続すると、サーバーは以降のすべてのサブスクリプションリクエストに含める必要がある一意の識別子（ `uid` ）を含むメッセージを送信します。

接続プロトコルの詳細については、 [WebSocket リファレンス](../reference/websockets/index.md) を参照してください。

### Status チャネルのサブスクライブ {: #subscribing-to-the-status-channel }

{{ tutorial.code_snippet(['py:37:42', 'js:35:38']) }}

コードは、監視対象アドレスをスコープとする <ws:status&#47;{address}> チャネルをサブスクライブします。
このチャネルは、そのアドレスが関与するトランザクションがネットワークによって拒否されるたびに通知を行い、エラーコードとトランザクションハッシュを提供します。

### 無効な転送トランザクションの構築と署名 {: #building-and-signing-an-invalid-transfer-transaction }

{{ tutorial.code_snippet(['py:44:73', 'js:40:67']) }}

このチュートリアルでは、エイリアス `symbol.unknown` を持つモザイクを含め、監視対象アドレスに送信される [転送トランザクション](../transactions/transfer.md) を構築します。
このモザイクはネットワーク上に存在しないため、トランザクションは拒否されます。

トランザクションは通常通り構築されます。ネットワーク時間と手数料乗数を取得し、トランザクション記述子を作成して署名します。
ハッシュはローカルで計算されるため、受信する WebSocket エラーメッセージと照合することができます。

### アナウンスとエラーの待機 {: #announcing-and-waiting-for-the-error }

{{ tutorial.code_snippet(['py:75:96', 'js:69:93']) }}

コードはトランザクションをアナウンスし、受信メッセージをリスニングします。
各メッセージは [TransactionStatusDTO](../reference/rest/symbol.md#model-TransactionStatusDTO) スキーマに従い、以下が含まれます。

* **hash:** 拒否されたトランザクションのハッシュ。
* **code:** トランザクションが拒否された理由を説明するエラーコード。
    すべての可能な値については、 [TransactionStatusEnum](../reference/rest/symbol.md#model-TransactionStatusEnum) スキーマを参照してください。

受信したハッシュがアナウンスされたトランザクションと一致すると、プログラムはエラーコードを出力して終了します。

### チャネルのサブスクライブ解除 {: #unsubscribing-from-the-channel }

{{ tutorial.code_snippet(['py:98:102', 'js:95:98']) }}

エラーを受信した後、コードは接続を閉じる前にサブスクライブ解除メッセージを送信します。

## 出力 {: #output }

```text linenums="1" hl_lines="2 3 4 5 6 7"
--8<-- 'devbook/websockets/listen-transaction-error.log'
```

出力の主なポイント:

* **アドレス** (2行目): 監視対象アドレス。
* **接続** (3行目): WebSocket 接続が確立され、サーバーは一意の `uid` を返します。
* **サブスクリプション** (4行目): `status` チャネルがサブスクライブされます。
* **アナウンス** (5行目): トランザクションがアナウンスされ、そのハッシュが出力されます。
* **エラー** (6行目): 送信者が要求されたモザイクを保持していないため、ネットワークは `Failure_Core_Insufficient_Balance` でトランザクションを拒否します。
* **サブスクライブ解除** (7行目): コードは `status` チャネルのサブスクライブを解除します。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ | 関連ドキュメント |
|------------------------------------------------------------------------------|--------------------------------------------------------------------------------|
| [status チャネルのサブスクライブ](#subscribing-to-the-status-channel) | <ws:status&#47;{address}> |
| [拒否をトリガーする](#building-and-signing-an-invalid-transfer-transaction) | [転送トランザクション](../transactions/transfer.md) |
| [エラーメッセージの処理](#announcing-and-waiting-for-the-error) | [TransactionStatusDTO](../reference/rest/symbol.md#model-TransactionStatusDTO) |
