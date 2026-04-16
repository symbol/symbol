# WebSockets {: #websockets }

ブロックチェーン上でイベントが発生したときに**リアルタイムに更新**を取得するために、Symbol では [WebSockets](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API) を公開しています。

クライアントアプリケーションは WebSocket 接続を開き、一意の識別子を取得できます。
この識別子を使用することで、アプリケーションは更新のために [REST API](../rest/symbol.md) を継続的にポーリングする代わりに、利用可能な任意のチャネルをサブスクライブできます。

チャネルでイベントが発生すると、 [REST Gateway](../../../textbook/nodes.md#rest-gateway) はサブスクライブしているすべてのクライアントにリアルタイムで通知を送信します。

WebSocket URI は HTTP リクエスト URI と同じホストとポートを共有しますが、 `ws://` プロトコルを使用します。
エンドポイントは `/ws` です。例: `ws://localhost:3001/ws`

送信するサブスクリプションメッセージと受信する更新メッセージはどちらも JSON 形式を使用し、これらについて次に説明します。

!!! warning "警告"

    アイドル状態が長すぎると、WebSocket 接続は警告なしに切断されます。

    再接続時にチャネルは自動的に再サブスクライブされません。

## レスポンス形式 {: #response-format }

すべてのチャネルは同じレスポンス形式を使用します。

```json title="レスポンスボディ"
{
    "topic": "{subscribed-channel}",
    "data": { ... }
}
```

* ``topic`` にはサブスクライブしたチャネルの名前が含まれるため、同じWebSocketを使用して複数のチャネルを監視できます（ ``topic`` は、サブスクライブ時にリクエストボディで提供される ``subscribe`` フィールドと一致します）。
* ``data`` はチャネル固有のオブジェクトです。以下のリストに記載されている各チャネルで、返されるデータオブジェクトについて説明しています。

## チャネル {: #channels }

### `block` {: #block }

ws:block
:   **新しいブロックが作成される**たびに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、1つのブロックに関する情報が含まれます。

=== "リクエストボディ"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "block"
    }
    ```

=== "レスポンスボディ"

    [BlockInfoDTO](../rest/symbol.md#model/BlockInfoDTO)

### `finalizedBlock` {: #finalizedblock }

ws:finalizedBlock
:   ブロックのセットが <finalization|ファイナライズ> されるたびに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、ファイナライゼーションラウンドで**最も高いブロック（最新のブロック）**に関する情報が含まれます。
    それより低いブロック高を持つすべてのブロックは、ファイナライズされたと見なされます。

=== "リクエストボディ"

    ```json
        {
            "uid": "{uid}",
            "subscribe": "finalizedBlock"
        }
    ```

=== "レスポンスボディ"

    [FinalizedBlockDTO](../rest/symbol.md#model/FinalizedBlockDTO)

### `confirmedAdded` {: #confirmedadded }

ws:confirmedAdded&#47;{address}
:   指定されたアドレスに関連するトランザクションがブロックに含まれたときに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、1つの承認済みトランザクションに関する情報が含まれます。

=== "リクエストボディ"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "confirmedAdded/{address}"
    }
    ```

=== "レスポンスボディ"

    [TransactionInfoDTO](../rest/symbol.md#model/TransactionInfoDTO)

### `unconfirmedAdded` {: #unconfirmedadded }

ws:unconfirmedAdded&#47;{address}
:   指定されたアドレスに関連するトランザクションが、ブロックに含まれるのを待つ未承認（unconfirmed）状態になったときに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、1つの未承認トランザクションに関する情報が含まれます。

このメッセージを受信する考えられるシナリオは次のとおりです。
トランザクションが <put:/transactions> HTTP エンドポイントを介してネットワークにアナウンスされた場合、または <bonded aggregate transaction:|アグリゲートボンデッドトランザクション> が必要なすべての連署者を集め、状態が `partial` から `unconfirmed` に変更された場合です。

=== "リクエストボディ"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "unconfirmedAdded/{address}"
    }
    ```

=== "レスポンスボディ"

    [TransactionInfoDTO](../rest/symbol.md#model/TransactionInfoDTO)

### `unconfirmedRemoved` {: #unconfirmedremoved }

ws:unconfirmedRemoved&#47;{address}
:   指定されたアドレスに関連するトランザクションが `unconfirmed` 状態を抜けたときに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、未承認ではなくなったトランザクションのハッシュが含まれます。

このメッセージを受信する考えられるシナリオは次のとおりです。
トランザクションが承認された場合、またはデッドラインに達してトランザクションがブロックに含まれなかった場合です。

=== "リクエストボディ"

    ```json
    {
        "uid":"{uid}",
        "subscribe":"unconfirmedRemoved/{address}"
    }
    ```

=== "レスポンスボディ"

    トランザクションのハッシュ。

### `partialAdded` {: #partialadded }

ws:partialAdded&#47;{address}
:   指定されたアドレスに関連する <bonded aggregate transaction:|アグリゲートボンデッドトランザクション> が、必要なすべての連署が完了するのを待つ `partial` 状態になったときに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、追加された1つのパーシャルトランザクションに関する情報が含まれます。

=== "リクエストボディ"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "partialAdded/{address}"
    }
    ```

=== "レスポンスボディ"

    [TransactionInfoDTO](../rest/symbol.md#model/TransactionInfoDTO)

### `partialRemoved` {: #partialremoved }

ws:partialRemoved&#47;{address}
:   指定されたアドレスに関連する <bonded aggregate transaction:|アグリゲートボンデッドトランザクション> が `partial` 状態を抜けたときに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、削除された1つのパーシャルトランザクションのハッシュが含まれます。

このメッセージが発行される考えられるシナリオは次のとおりです。
必要なすべての <cosignatures:|連署> を受信してトランザクションが `unconfirmed` になった場合、またはデッドラインに達してトランザクションがブロックに含まれなかった場合です。

=== "リクエストボディ"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "partialRemoved/{address}"
    }
    ```

=== "レスポンスボディ"

    トランザクションのハッシュ。

### `cosignature` {: #cosignature }

ws:cosignature&#47;{address}
:   指定されたアドレスに関連する <cosignature:|連署> が、 `partial` 状態の <bonded aggregate transaction:|アグリゲートボンデッドトランザクション> に追加されたときに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、連署付きトランザクションが1つ含まれます。

=== "リクエストボディ"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "cosignature/{address}"
    }
    ```

=== "レスポンスボディ"

    [CosignatureDTO](../rest/symbol.md#model/CosignatureDTO)

### `status` {: #status }

ws:status&#47;{address}
:   指定されたアドレスに関連するトランザクションがエラーを通知したときに、サブスクライブしているクライアントに通知します。
    返される各メッセージには、1つのエラーメッセージとトランザクションのハッシュが含まれます。

=== "リクエストボディ"

    ```json
    {
        "uid": "{uid}",
        "subscribe": "status/{address}"
    }
    ```

=== "レスポンスボディ"

    [TransactionStatusDTO](../rest/symbol.md#model/TransactionStatusDTO)