---
title: ボンデッドトランザクションフロー
tutorial_level: advanced
---

# ボンデッドトランザクションフローのリスニング {: #listening-to-bonded-transaction-flow }

[アグリゲートボンデッドトランザクション](default: アグリゲートボンデッドトランザクション) は、通常の [トランザクション](default: トランザクション) よりも複雑なライフサイクルをたどります。
アナウンス後、ネットワークが必要なすべての参加者から [連署](default: 連署) を受け取る `partial`状態に入ります。
すべての連署が届いて初めて、トランザクションは標準の `unconfirmed`および `confirmed`状態へと進みます。

このチュートリアルでは、[アグリゲートボンデッドトランザクション](../transactions/bonded-aggregate.md) チュートリアルのアセットスワップを行いますが、ポーリングの代わりに [WebSocket](../reference/websockets/index.md) チャネルを使用してボンデッドのライフサイクル全体を監視します。

アカウント A がアグリゲートを構築してアナウンスする一方、アカウント B は WebSocket チャネルを購読し、連署を行い、承認を待ちます。

## 前提条件 {: #prerequisites }

開始する前に、開発環境がセットアップされていることを確認してください。
[開発環境のセットアップ](../start/setup.md) を参照してください。

さらに、言語に応じた WebSocket ライブラリをインストールしてください。

=== ":simple-python: Python"

    `websockets` ライブラリをインストールします。

    ```bash
    pip install websockets
    ```

=== ":simple-javascript: JavaScript"

    このチュートリアルでは、Node.js 22 以降で利用可能なネイティブの `WebSocket` API を使用します。
    追加のパッケージは必要ありません。

また、スワップを完了させるために、 [XYM](default: XYM) を持つ2つの [アカウント] (default: アカウント) と1つのカスタム [モザイク](default:モザイク) が必要です。便宜上、事前に資金供給されたアカウントが提供されていますが、これらはメンテナンスされておらず資金が不足している可能性があります。

自身のアカウントを使用する場合は、以下の手順を完了してください。

* アグリゲートトランザクションを開始するためのアカウント（アカウント A）を、 [コード](../accounts/create-from-private-key.md) または [ウォレット](../../userbook/wallet/create-account.md) を使用して作成します。
* スワップに参加するための2つ目のアカウント（アカウント B）を作成します。
* トランザクション手数料、転送量、および [ハッシュロック] (default: ハッシュ) のデポジットを支払うための XYM をアカウント A で入手します。
  [蛇口 (Faucet) からテストネットの通貨を入手する](../accounts/testnet-faucet.md) を参照してください。
* スワップのためにアカウント B が所有するモザイクを作成します。
  [モザイクの作成](../mosaics/create-mosaic.md) を参照してください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/websockets/listen_bonded_transaction_flow', ['py', 'js']) }}

アグリゲートボンデッドトランザクションには、2つの異なる役割が含まれます。アグリゲートを構築、署名、アナウンスする **開始者** （アカウント A）と、WebSocket チャネルを監視し、トランザクションを検証した後に連署する1人以上の **連署者** （アカウント B）です。

実際には、それぞれの役割は別々のマシンの別々のプログラムとして実行され、すべての連署者は開始者がアグリゲートボンデッドを送信する前にすでにリスニング状態（待ち受け状態）になっている必要があります。
このチュートリアルでは、簡略化のため両方の役割を1つのスクリプトにまとめています。

## コード解説 {: #code-explanation }

### アカウント A: アカウントの設定 {: #account-a-setting-up-accounts }

{{ tutorial.code_snippet_tagged('step-1') }}

この例では、簡略化のため1つのスクリプトに両方の [秘密鍵](default: 秘密鍵) を含めています。実際には、各当事者が自身のマシンで [署名](default: 署名) します。
アカウント A は、[埋め込みトランザクション](default: 埋め込みトランザクション) の署名者としてアカウント B を設定し、B の [アドレス](default: アドレス) を派生させるために、アカウント B の [公開鍵](default: 公開鍵) のみを必要とします。

環境変数 `ACCOUNT_A_PRIVATE_KEY` と `ACCOUNT_B_PRIVATE_KEY` で各アカウントの鍵を設定します。設定されない場合は、デフォルトでテストキーが使用されます。
自身の鍵を使用する場合は、アカウント A が XYM を持ち、アカウント B がスワップ用のカスタムモザイクを保持していることを確認してください。
アドレスは、ファサードのネットワーク設定を使用して公開鍵から派生します。

### アカウント A: アグリゲートの構築とハッシュロックのアナウンス {: #account-a-building-the-aggregate-and-announcing-the-hash-lock }

{{ tutorial.code_snippet_tagged('step-2') }}

アカウント A は、 [アグリゲートボンデッドトランザクション](../transactions/bonded-aggregate.md) チュートリアルで説明されているのと同じパターンに従い、アカウント B の 1 つのカスタムモザイクと 10 XYM を交換するアグリゲートボンデッドを作成して署名し、必要なハッシュロックをアナウンスします。

唯一の違いは、ハッシュロックを確認するために <get:/transactionStatus/{hash}> をポーリングする代わりに、このチュートリアルでは [トランザクションフローのリスニング](./listen-transaction-flow.md) チュートリアルで説明されているものと同じアプローチに従って WebSocket を使用することです。

### アカウント B: 接続とチャネルのサブスクライブ {: #account-b-connecting-and-subscribing-to-channels }

{{ tutorial.code_snippet_tagged('step-3') }}

このスニペットでは、 `NODE_URL` 環境変数を使用して Symbol API [ノード](default: ノード) を設定します。値が指定されない場合は、デフォルト値が使用されます。
WebSocket URL は、HTTP プロトコルを WebSocket プロトコルに置き換え、 `/ws` を追加することで `NODE_URL` から派生します。

アカウント B は WebSocket 接続を開き、ボンデッドトランザクションのライフサイクルを監視するために、自身のアドレスをスコープとするチャネルをサブスクライブします。
アカウント B はアグリゲートの参加者であるため、ノードはトランザクションのすべてのライフサイクルイベントをアカウント B のアドレスに配信します。
[通常のトランザクション](./listen-transaction-flow.md) で使用されるチャネルに加えて、ボンデッドアグリゲートは追加のチャネルを使用します。

* <ws:partialAdded&#47;{address}>: ボンデッドアグリゲートが `partial` 状態になり、連署を待っているときに通知します。
* <ws:partialRemoved&#47;{address}>: ボンデッドアグリゲートが `partial` 状態を抜けたとき（すべての連署が収集されたか、期限が切れたとき）に通知します。
* <ws:cosignature&#47;{address}>: 部分的トランザクションに連署が追加されたときに通知します。

### アカウント A: ボンデッドアグリゲートのアナウンス {: #account-a-announcing-the-bonded-aggregate }

{{ tutorial.code_snippet_tagged('step-4') }}

アカウント B のサブスクライブが完了すると、アカウント A は通常の <put:/transactions> エンドポイントではなく、 <put:/transactions/partial> にボンデッドアグリゲートをアナウンスします。

### アカウント B: WebSocket メッセージの処理と連署 {: #account-b-handling-websocket-messages-and-cosigning }

{{ tutorial.code_snippet_tagged('step-5') }}

アカウント B は受信メッセージをリスニングし、チャネルごとに振り分けます。
メッセージのスキーマは、 `cosignature` メッセージを除き、 [通常のトランザクションフロー](./listen-transaction-flow.md) チュートリアルと同じです。 `cosignature` メッセージは [CosignatureDTO](../reference/rest/symbol.md#model-CosignatureDTO) スキーマに従い、他のチャネルで使用される `meta.hash` フィールドは含まれません。

重要なアクションは `partialAdded` で発生します。ハッシュが期待されるアグリゲートと一致した場合、アカウント B は `detached` パラメータを `true` に設定した <dy:SymbolFacade.cosignTransactionHash> を使用してトランザクションに連署し、 <put:/transactions/cosignature> に連署をアナウンスします。
より深い検証を行うために、アカウント B は <get:/transactions/partial/{transactionId}> から完全なトランザクションを取得し、内容を検査してから連署するかどうかを決定することができます。

成功したボンデッドアグリゲートの期待されるメッセージシーケンスは、テキストブックの [トランザクションのライフサイクル](../../textbook/transactions.md#transaction-lifecycle) セクションで説明されています。

1. `partialAdded`: ボンデッドアグリゲートがパーシャル（部分的）キャッシュに入り、連署を待ちます。
2. `cosignature`: アカウント B からの連署が追加されます。
3. `unconfirmedAdded`: 完全に署名されたトランザクションが [未承認トランザクションプール] (default: 未承認トランザクションプール) に入ります。
4. `partialRemoved`: トランザクションが `partial` 状態を抜けます。
5. `unconfirmedRemoved`: トランザクションが未承認プールを抜けます。
6. `confirmedAdded`: トランザクションがブロック内で承認されます。

### アカウント B: チャネルのサブスクライブ解除 {: #account-b-unsubscribing-from-channels }

{{ tutorial.code_snippet_tagged('step-6') }}

承認後、アカウント B は接続を閉じる前に、すべてのチャネルのサブスクライブ解除メッセージを送信します。

## 出力 {: #output }

```text linenums="1" hl_lines="2-3 6 7 8-14 15 16-22"
--8<-- 'devbook/websockets/listen_bonded_transaction_flow.log'
```

出力のポイント:

* **アカウント** (2-3行目): アカウント A（開始者）とアカウント B（連署者）のアドレス。
* **ハッシュロック** (6行目): ボンデッドアグリゲートハッシュが計算され、ハッシュロックがアナウンスされ、その承認が WebSocket 経由で受信されます。
* **接続** (7行目): WebSocket 接続が確立され、サーバーは一意の `uid` を返します。
* **サブスクリプション** (8-14行目): （ `status` を含む）7つすべてのボンデッドトランザクションチャネルがサブスクライブされます。
* **アナウンス** (15行目): ボンデッドアグリゲートが `/transactions/partial` にアナウンスされます。
* **連署** (16-18行目): アグリゲートが `partialAdded` に入り、アカウント B が連署を送信し、 `cosignature` チャネルがそれを受信したことを確認します。
* **承認** (19-22行目): 完全に署名されたトランザクションが未承認プールに入り（ `unconfirmedAdded` ）、 `partial` 状態を抜け（ `partialRemoved` ）、 `unconfirmedRemoved` を経て、最後に `confirmedAdded` になります。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                                              | 関連ドキュメント                                                                  |
|-----------------------------------------------------------------------------------|-----------------------------------------------------------------------------|
| [partialAdded をサブスクライブする](#account-b-connecting-and-subscribing-to-channels)      | <ws:partialAdded&#47;{address}>                                             |
| [partialRemoved をサブスクライブする](#account-b-connecting-and-subscribing-to-channels)    | <ws:partialRemoved&#47;{address}>                                           |
| [cosignature をサブスクライブする](#account-b-connecting-and-subscribing-to-channels)       | <ws:cosignature&#47;{address}>                                              |
| [トランザクションメッセージを処理する](#account-b-handling-websocket-messages-and-cosigning)      | [TransactionInfoDTO](../reference/rest/symbol.md#model-TransactionInfoDTO)  |
| [連署メッセージを処理する](#account-b-handling-websocket-messages-and-cosigning)          | [CosignatureDTO](../reference/rest/symbol.md#model-CosignatureDTO)          |
| [partialAdded で連署を送信する](#account-b-handling-websocket-messages-and-cosigning) | <dy:SymbolFacade.cosignTransactionHash><br/><put:/transactions/cosignature> |
