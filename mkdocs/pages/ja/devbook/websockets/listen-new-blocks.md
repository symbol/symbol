---
title: 新しいブロック
---

# 新しいブロックのリスニング {: #listening-to-new-blocks }

<ws:block> と <ws:finalizedBlock> WebSocket チャネルは、新しい [ブロック](default: ブロック) が生成されたとき、または [ファイナライズ](../../textbook/consensus.md#finalization) されたときにリアルタイムの通知を送信します。
<get:/chain/info> エンドポイントをポーリングする場合と比較して、WebSocket は API 呼び出しを繰り返すオーバーヘッドなしに、更新が発生した瞬間にプッシュします。

このチュートリアルでは、両方のチャネルをサブスクライブし、到着した各更新を表示する方法を説明します。

!!! note "メモ"

    ポーリングベースのアプローチについては、[チェーンとファイナライズの最新高の照会](../chain/chain-heights.md) チュートリアルを参照してください。

## 前提条件 {: #prerequisites }

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

{{ tutorial.code_full('devbook/websockets/listen-new-blocks', ['py', 'js']) }}

このスニペットでは、 `NODE_URL` 環境変数を使用して Symbol API [ノード](default: ノード) を設定します。
値が指定されない場合は、デフォルト値が使用されます。
WebSocket URL は、HTTP プロトコルを WebSocket プロトコルに置き換え、 `/ws` を追加することで `NODE_URL` から派生します。

プログラムは `Ctrl+C` で中断されるまで実行され、接続を閉じる前にサブスクライブ解除のステップをトリガーします。

## コード解説 {: #code-explanation }

### WebSocket への接続 {: #connecting-to-the-websocket }

{{ tutorial.code_snippet(['py:13:17', 'js:7:16']) }}

最初のステップは、ノードの `/ws` エンドポイントへの WebSocket 接続を開くことです。
接続すると、サーバーは以降のすべてのサブスクリプションリクエストに含める必要がある一意の識別子（ `uid` ）を含むメッセージを送信します。

接続プロトコルの詳細については、[WebSocket リファレンス](../reference/websockets/index.md) を参照してください。

### チャネルのサブスクライブ {: #subscribing-to-channels }



{{ tutorial.code_snippet(['py:19:27', 'js:18:24']) }}

コードは2つのチャネルをサブスクライブします。

* <ws:block>: 新しいブロックが生成されるたびに（約30秒ごとに）通知します。
* <ws:finalizedBlock>: ファイナライズラウンドが完了するたびに（約10〜20分ごとに）通知します。

各サブスクリプションメッセージには、接続ステップで受信した `uid` とチャネル名が含まれます。

### メッセージの処理 {: #handling-messages }

{{ tutorial.code_snippet(['py:29:48', 'js:26:50']) }}

コードは、プログラムが中断されるまで受信メッセージをリスニングします。
各メッセージには、チャネルを識別する `topic` フィールドと、イベントペイロードを含む `data` オブジェクトが含まれます。

`block` メッセージの場合、ペイロードは [BlockInfoDTO](../reference/rest/symbol.md#model-BlockInfoDTO) スキーマに従います。
このチュートリアルでは、各ブロックを識別するためにそのうちの2つを使用します。

* `data.block.height`: 新しいブロックの高さ。
* `data.meta.hash`: 新しいブロックのハッシュ。

`finalizedBlock` メッセージの場合、ペイロードは [FinalizedBlockDTO](../reference/rest/symbol.md#model-FinalizedBlockDTO) スキーマに従います。
このチュートリアルでは以下を使用します。

* `data.height`: ファイナライズされたブロックの高さ。
* `data.hash`: ファイナライズされたブロックのハッシュ。

チェーンの高さは、新しいブロックが生成されるたびに増加します。
ファイナライズは通常、ブロック生成の10〜20分後に行われるため、ファイナライズされた高さはチェーンの先端（最新高）より遅れます。

[投票ノード](default: 投票ノード) がこのプロセスをどのように推進するかについての詳細は、テキストブックの [コンセンサス](../../textbook/consensus.md) セクションを参照してください。

### 終了時のサブスクライブ解除 {: #unsubscribing-on-exit }

{{ tutorial.code_snippet(['py:50:57', 'js:52:60']) }}

プログラムが中断されたとき（ `Ctrl+C` ）、コードは接続を閉じる前に両方のチャネルのサブスクライブ解除メッセージを送信します。
これにより、ノードからのクリーンな切断が保証されます。

## 出力 {: #output }

以下の出力は、新しいブロックとファイナライズイベントをリスニングする典型的な実行例を示しています。

```text linenums="1" hl_lines="2 3 4 5 8 11"
--8<-- 'devbook/websockets/listen-new-blocks.log'
```

出力の主なポイント:

* **接続** (2行目): `wss://` URL への WebSocket 接続が確立され、サーバーは一意の `uid` を返します。
* **サブスクリプション** (3-4行目): `block` チャネルと `finalizedBlock` チャネルの両方がサブスクライブされます。
* **新しいブロック** (5-7行目、9-10行目): 新しいブロックの通知が約30秒ごとに届きます。
* **ファイナライズ** (8行目): ファイナライズラウンドが完了すると、一度に複数のブロックをカバーするファイナライズ通知が届きます。
* **サブスクライブ解除** (11行目): `Ctrl+C` を押すと、コードは両方のチャネルのサブスクライブを解除します。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ | 関連ドキュメント |
| ---------------------------------------------------------- | ------------------------------------------------------------------------------ |
| [block チャネルのサブスクライブ](#subscribing-to-channels) | <ws:block> |
| [finalized チャネルのサブスクライブ](#subscribing-to-channels) | <ws:finalizedBlock> |
| [ブロックメッセージの処理](#handling-messages) | [BlockInfoDTO](../reference/rest/symbol.md#model-BlockInfoDTO) |
| [ファイナライズメッセージの処理](#handling-messages) | [FinalizedBlockDTO](../reference/rest/symbol.md#model-FinalizedBlockDTO) |
