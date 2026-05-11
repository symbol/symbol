---
title: トランザクションフロー
tutorial_level: beginner
---

# トランザクションフローのリスニング {: #listening-to-transaction-flow }

Symbolは、特定の[アカウント](default: アカウント) に対する[トランザクション](default: トランザクション) が承認プロセスを進む際に、リアルタイムの通知を送信するWebSocketチャネルを提供しています。
<get:/transactionStatus/{hash}> エンドポイントをポーリングする場合と比較して、WebSocketはAPI呼び出しを繰り返すオーバーヘッドなしに、更新が発生した瞬間にプッシュします。

このチュートリアルでは、トランザクションチャネルをサブスクライブし、最小限の[転送トランザクション](../transactions/transfer.md)をアナウンスし、WebSocketを使用してその承認を待つ方法を説明します。

!!! note "メモ"

    ポーリングベースのアプローチについては、[トランザクションステータスの監視](../transactions/monitoring-status.md) チュートリアルを参照してください。

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

* 開発環境をセットアップしていること。
  [開発環境のセットアップ](../start/setup.md) を参照してください。
* 監視するアカウントの[アドレス](default: アドレス) を保有していること。
* トランザクション手数料を支払うのに十分な残高を持つアカウントを保有していること。
  [秘密鍵からのアカウント作成](../accounts/create-from-private-key.md) または
  [ウォレットを使用したアカウントの作成](../../userbook/wallet/create-account.md) を参照してください。

さらに、言語固有のWebSocketライブラリをインストールしてください。

=== ":simple-python: Python"

    `websockets` ライブラリをインストールします。

    ```bash
    pip install websockets
    ```

=== ":simple-javascript: JavaScript"

    このチュートリアルでは、Node.js 22以降で利用可能なネイティブの `WebSocket` APIを使用します。
    追加のパッケージは必要ありません。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/websockets/listen_transaction_flow', ['py', 'js']) }}

このスニペットでは、 `NODE_URL` 環境変数を使用してSymbol API[ノード](default: ノード) を設定します。
値が指定されない場合は、デフォルト値が使用されます。
WebSocket URLは、HTTPプロトコルをWebSocketプロトコルに置き換え、 `/ws` を追加することで `NODE_URL` から派生します。

## コード解説 {: #code-explanation }

### 監視対象アドレスと署名者の設定 {: #setting-up-the-monitored-address-and-signer }

{{ tutorial.code_snippet_tagged('step-1') }}

各トランザクションWebSocketチャネルは、特定のアドレスをスコープとします。
`MONITOR_ADDRESS` 環境変数は、監視するアドレスを設定します。
このチャネルは、送信者、受信者、またはトランザクションの内容から派生したその他の役割（例えば、[アグリゲートトランザクション](default: アグリゲートトランザクション) 内の埋め込みトランザクションの署名者など）を問わず、このアドレスがトランザクションに関与するたびに通知を送信します。

通知をトリガーするために、このチュートリアルでは監視対象アドレスに転送トランザクションを送信します。
送信者の秘密鍵は `SIGNER_PRIVATE_KEY` から読み取られます。

これらの環境変数のいずれかが提供されない場合、チュートリアルは同じアカウントに対応するデフォルト値を提供します。

### WebSocketへの接続 {: #connecting-to-the-websocket }

{{ tutorial.code_snippet_tagged('step-2') }}

コードは、ノードの `/ws` エンドポイントへのWebSocket接続を開きます。
接続すると、サーバーは以降のすべてのサブスクリプションリクエストに含める必要がある一意の識別子（ `uid` ）を含むメッセージを送信します。

接続プロトコルの詳細については、[WebSocket リファレンス](../reference/websockets/index.md) を参照してください。

### チャネルのサブスクライブ {: #subscribing-to-channels }

{{ tutorial.code_snippet_tagged('step-3') }}

コードは、監視対象アドレスを各チャネル名に追加して、アドレスをスコープとする3つのチャネルをサブスクライブします。

* <ws:unconfirmedAdded&#47;{address}>: トランザクションが[未承認トランザクションプール](default: 未承認トランザクションプール) に入り、[ブロック](default: ブロック) に含まれるのを待っているときに通知します。
* <ws:unconfirmedRemoved&#47;{address}>: トランザクションが未承認状態を抜けたとき（承認されたか、期限切れになったか）に通知します。
* <ws:confirmedAdded&#47;{address}>: アドレスが関与するトランザクションがブロックに含まれたときに通知します。

各サブスクリプションメッセージには、接続ステップで受信した `uid` と、監視対象アドレスを含む完全なチャネル名が含まれます。

### 転送トランザクションの構築と署名 {: #building-and-signing-a-transfer-transaction }

{{ tutorial.code_snippet_tagged('step-4') }}

このチュートリアルでは、モザイクもメッセージも含まない、監視対象アドレスへの最小限の[転送トランザクション](../transactions/transfer.md) を構築します。
簡略化のために転送が使用されていますが、どのトランザクションタイプでも同じWebSocket通知がトリガーされます。

トランザクションは通常通り構築されます。ネットワーク時間と手数料乗数を取得し、トランザクション記述子を作成して[署名](default: 署名) します。
[ハッシュ](default: ハッシュ) はローカルで計算されるため、後で受信するWebSocketメッセージと照合することができます。

### アナウンスと承認の待機 {: #announcing-and-waiting-for-confirmation }

{{ tutorial.code_snippet_tagged('step-5') }}

コードはトランザクションをアナウンスし、受信メッセージをリスニングして各メッセージを表示します。

!!! warning "注意: チャネルのサブスクライブ後にアナウンスする"

    リスナーの準備ができていることを確認するために、必ずWebSocketチャネルをサブスクライブした **後** にトランザクションをアナウンスしてください。
    そうしないと、WebSocketがリスニング状態になる前に通知が到着する可能性があります。

各メッセージには、チャネルを識別する `topic` フィールドと、イベントペイロードを含む `data` オブジェクトが含まれます。

`confirmedAdded` および `unconfirmedAdded` メッセージの場合、ペイロードは [TransactionInfoDTO](../reference/rest/symbol.md#model-TransactionInfoDTO) スキーマに従います。
`unconfirmedRemoved` メッセージの場合、ペイロードにはトランザクションハッシュ（ `meta.hash` ）のみが含まれます。

ハッシュがアナウンスされたトランザクションと一致する `confirmedAdded` メッセージが到着すると、プログラムは承認メッセージを出力して終了します。

成功したトランザクションの期待されるシーケンスは、テキストブックの [トランザクションのライフサイクル](../../textbook/transactions.md#transaction-lifecycle) セクションで説明されています。

1. `unconfirmedAdded`: トランザクションが未承認プールに入ります。
2. `unconfirmedRemoved`: トランザクションが未承認プールを抜けます。
3. `confirmedAdded`: トランザクションがブロック内で承認されます。

### チャネルのサブスクライブ解除 {: #unsubscribing-from-channels }

{{ tutorial.code_snippet_tagged('step-6') }}

承認後、コードは接続を閉じる前に3つすべてのチャネルのサブスクライブ解除メッセージを送信します。

## 出力 {: #output }

```text linenums="1" hl_lines="2 3 4-6 7 8-10 11 12"
--8<-- 'devbook/websockets/listen_transaction_flow.log'
```

出力の主なポイント:

* **アドレス** (2行目): 監視対象アドレス。
* **接続** (3行目): WebSocket 接続が確立され、サーバーは一意の `uid` を返します。
* **サブスクリプション** (4-6行目): 3つすべてのトランザクションチャネルがサブスクライブされます。
* **アナウンス** (7行目): トランザクションがアナウンスされ、そのハッシュが出力されます。
* **トランザクションフロー** (8-10行目): トランザクションは `unconfirmedAdded` から `unconfirmedRemoved` 、そして `confirmedAdded` へと移行し、承認のライフサイクル全体を示しています。
* **承認** (11行目): `confirmedAdded` からのハッシュがアナウンスされたトランザクションと一致し、成功が確認されます。
* **サブスクライブ解除** (12行目): コードはすべてのチャネルのサブスクライブを解除します。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                           | 関連ドキュメント                                                                 |
|----------------------------------------------------------------|----------------------------------------------------------------------------|
| [unconfirmedAdded のサブスクライブ](#subscribing-to-channels)          | <ws:unconfirmedAdded&#47;{address}>                                        |
| [unconfirmedRemoved のサブスクライブ](#subscribing-to-channels)        | <ws:unconfirmedRemoved&#47;{address}>                                      |
| [confirmedAdded のサブスクライブ](#subscribing-to-channels)            | <ws:confirmedAdded&#47;{address}>                                          |
| [トランザクションメッセージの処理](#announcing-and-waiting-for-confirmation) | [TransactionInfoDTO](../reference/rest/symbol.md#model-TransactionInfoDTO) |

## 次のステップ {: #next-steps }

拒否されたトランザクションとそのエラーコードを検出するには、[トランザクションエラーのリスニング](listen-transaction-error.md) チュートリアルを参照してください。
