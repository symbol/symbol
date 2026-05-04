---
title: トランザクションバッチ処理
tutorial_level: intermediate
---

# トランザクションのバッチ処理 {: #batching-transactions }

[アグリゲートコンプリートトランザクション](default:アグリゲートコンプリートトランザクション) を使用すると、単一の [アカウント](default:アカウント) からの複数のトランザクションを1つのアトミックな操作に結論、1回の手数料と1回の承認で済ませることができます。

これは、例えば報酬の分配、支払いの分割、または複数のアカウントへの同時資金供給などに役立ちます。

このチュートリアルでは、異なる受信者に <XYM:> を送信する2つの [転送トランザクション](default:転送トランザクション) をバッチ処理する方法を説明します。

```dot
digraph {
    rankdir="LR";
    fontsize=12;
    subgraph clusterAggregate {
        label = "アグリゲートコンプリートトランザクション";
        tooltip = "アグリゲートコンプリートトランザクション";
        subgraph clusterT1 {
            label = "埋め込み転送 1";
            tooltip = "埋め込み転送 1";
            style = dashed;
            S1 [label="署名者" tooltip="署名者"];
            R1 [label="受信者 1" tooltip="受信者 1"];
            S1 -> R1 [label="5 XYM"];
        }
        subgraph clusterT2 {
            label = "埋め込み転送 2";
            tooltip = "埋め込み転送 2";
            style = dashed;
            S2 [label="署名者" tooltip="署名者"];
            R2 [label="受信者 2" tooltip="受信者 2"];
            S2 -> R2 [label="3 XYM"];
        }
    }
}
```

すべての埋め込みトランザクションが同じ署名者を共有するため、cosignatures:|連署 は必要ありません。
アグリゲートは単一のアカウントによって署名され、アナウンスされます。
複数のアカウントから署名を収集する必要がある例については、[アグリゲートコンプリート](./complete-aggregate.md) および [アグリゲートボンデッド](./bonded-aggregate.md) のチュートリアルを参照してください。

## 前提条件 {: #prerequisites }

開始する前に、開発環境がセットアップされていることを確認してください。
[開発環境のセットアップ](../start/setup.md) を参照してください。

また、転送とトランザクション手数料をカバーするのに十分な <XYM:> を持つ [アカウント](default:アカウント)も必要です。
便宜上、事前に資金供給されたテストアカウントが提供していますが、これはメンテナンスされておらず、いつでも資金が不足する可能性があります。

自身のアカウントを使用する場合は、以下の手順を完了してください。

* バッチトランザクションを送信するためのアカウントを、[コード](../accounts/create-from-private-key.md)または[ウォレット](../../userbook/wallet/create-account.md)を使用して 作成します。

* トランザクション手数料と転送額を支払うための XYM を入手します。[蛇口 (Faucet) からテストネットの通貨を入手する](../accounts/testnet-faucet.md) を参照してください。

さらに、トランザクションがどのようにアナウンスされ、承認されるかを理解するために、[転送トランザクション](./transfer.md) のチュートリアルを確認してください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/transactions/transaction-batching', ['py', 'js']) }}

## コード解説 {: #code-explanation }

### アカウントの設定 {: #setting-up-the-account }

{{ tutorial.code_snippet(['py:16:35', 'js:14:35']) }}

署名者アカウントは、 `SIGNER_PRIVATE_KEY` 環境変数から読み込まれます。
指定されていない場合は、デフォルトでテストキーが使用されます。

2つの受信者アドレスは、 `RECIPIENT_1` および `RECIPIENT_2` 環境変数から読み込まれます。
指定されていない場合は、デフォルトでテストアドレスが使用されます。

### ネットワーク時間と手数料の取得 {: #fetching-network-time-and-fees }

{{ tutorial.code_snippet(['py:38:56', 'js:38:56']) }}

ネットワーク時間と推奨手数料は、[転送トランザクション](./transfer.md) のチュートリアルで説明されているプロセスに従い、それぞれ <get:/node/time> と <get:/network/fees/transaction> から取得されます。

### 埋め込みトランザクションの作成 {: #creating-embedded-transactions }

{{ tutorial.code_snippet(['py:58:79', 'js:58:79']) }}

各転送は、アグリゲート内にラップされる[埋め込みトランザクション](default:埋め込みトランザクション)として作成されます。
すべての埋め込みトランザクションは同じアカウントから発生するため、同じ `signer_public_key` を使用します。

この例では、2つの [転送トランザクション](default:転送トランザクション)を作成します。

* 最初の転送では、受信者 1 に 5 XYM を送信します。
* 2番目の転送では、受信者 2 に 3 XYM を送信します。

すべてが同じ署名者を共有している場合でも、各埋め込みトランザクションで `signer_public_key` が必要です。

埋め込みトランザクションには、手数料や有効期限のフィールドは**含まれません**。
これらは、それを囲むアグリゲートトランザクションから継承されます。

!!! note "他のトランザクションタイプのバッチ処理"

    この例では転送トランザクションをバッチ処理していますが、（他のアグリゲートを除く）任意のトランザクションタイプをアグリゲート内に埋め込むことができます。
    例えば、モザイクの作成とネームスペースエイリアスの登録を単一のアトミックな操作としてバッチ処理することができます。

### アグリゲートトランザクションの構築 {: #building-the-aggregate-transaction }

{{ tutorial.code_snippet(['py:81:93', 'js:81:94']) }}

* **Type:** <ser:AggregateCompleteTransactionV3|aggregate_complete_transaction_v3> を使用します。

* **Signer public key:** アグリゲートに署名し、トランザクション手数料を支払うアカウント。

* **Deadline:** ネットワーク時間 で指定されるタイムスタンプ。これを過ぎるとトランザクションは期限切れとなり、承認できなくなります。

* **Transactions hash:** <dy:SymbolFacade.hashEmbeddedTransactions> を使用して、すべての埋め込みトランザクションから計算されるハッシュ。
これにより、署名後に埋め込みトランザクションが変更されないことが保証されます。

* **Transactions:** 実行する埋め込みトランザクションの配列。

手数料は、アグリゲートの合計サイズに基づいて計算されます。
連署は必要ないため、連署バイト用に余分なスペースを確保する必要はありません。

### 署名とアナウンス {: #signing-and-announcing }

{{ tutorial.code_snippet(['py:95:110', 'js:96:111']) }}

アグリゲートは <dy:SymbolFacade.signTransaction> で署名され、 <dy:SymbolTransactionFactory.attachSignature> を使用してペイロードにシリアライズされます。
署名されたペイロードはその後、[転送トランザクション](./transfer.md) チュートリアルで説明されている通常のトランザクションと同じプロセスに従って、 <put:/transactions> エンドポイントを使用して [ノード](default:ノード) にアナウンスされます。

### 承認の待機 {: #waiting-for-confirmation }

{{ tutorial.code_snippet(['py:112:133', 'js:113:139']) }}

アナウンス後、 <get:/transactionStatus/{hash}> を使用してトランザクションステータスが監視されます。
ポーリングループは、トランザクションが承認されるか失敗するまで、毎秒ステータスを確認します。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="16 26 29 30 40 43 44 50"
--8<-- 'devbook/transactions/transaction-batching.log'
```

出力の主なポイント:

* **16行目** (`"type": 16705`): これが <ser:AggregateCompleteTransactionV3> であることを識別します。

* **26行目と40行目** (`"recipient_address"`): 2つの埋め込み転送は異なるアカウントをターゲットにしています。
これらは、4〜5行目に出力された Base32 アドレスの16進数エンコード形式です。

* **29-30行目と43-44行目** (`"mosaic_id", "amount"`): 各転送は XYM（モザイクエイリアス ID `16666583871264174062`）を送信します。
このモザイクの [可分性](default:可分性) は 6 であるため、金額 5000000 と 3000000 はそれぞれ 5 および 3 XYM に対応します。

* **50行目** (`"cosignatures": []`): すべての埋め込みトランザクションが同じ署名者を共有しているため、空です。
追加の署名は必要ありません。

アグリゲートトランザクションはアトミックに実行されます。つまり、両方の受信者が XYM の転送を受け取るか、どちらも受け取らないかのいずれかになります。

出力されたトランザクションハッシュ（54行目）を使用して、 [Symbol Testnet Explorer](https://testnet.symbol.fyi/) でトランザクションを検索できます。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                   | 関連ドキュメント                                                                          |
|--------------------------------------------------------|-------------------------------------------------------------------------------------|
| [埋め込みトランザクションの作成](#creating-embedded-transactions) | <dy:SymbolTransactionFactory.createEmbedded>                                        |
| [アグリゲートの構築](#building-the-aggregate-transaction)     | <dy:SymbolTransactionFactory.create><br/><dy:SymbolFacade.hashEmbeddedTransactions> |
| [署名とアナウンス](#signing-and-announcing)                  | <dy:SymbolFacade.signTransaction><br/><dy:SymbolTransactionFactory.attachSignature> |

## 次のステップ {: #next-steps }

* **連署者の追加:** 埋め込みトランザクションに複数の署名者が関与し、アナウンス前にオフチェーンで連署できる場合は、[アグリゲートコンプリート](./complete-aggregate.md)のチュートリアルを参照してください。

* **オンチェーンでの署名収集:** トランザクションがアナウンスされた後に連署者が署名する必要がある場合は、[アグリゲートボンデッド](./bonded-aggregate.md)のチュートリアルを参照してください。

* **手数料のスポンサー:** [他のアカウントの代理での手数料支払い](./fee-sponsorship.md) のチュートリアルを使用して、あるアカウントが別のアカウントの代わりにトランザクション手数料を支払うことができるようにします。
