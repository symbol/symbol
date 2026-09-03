---
title: 手動トランザクション作成
tutorial_level: intermediate
---

# トランザクションを手動で作成する

ほとんどのチュートリアルでは、 <dy:SymbolFacade.createTransactionFromTypedDescriptor> を使用して、ディスクリプタからトランザクションを作成します。
これは推奨される方法です。型付きディスクリプタをサポートする言語では型安全性とより優れたエディタサポートが得られ、さらにデッドラインとトランザクション手数料も計算されます。

完全性のため、このチュートリアルでは下位レベルの代替手段として、 <dy:SymbolTransactionFactory.create> を使用してトランザクションを手動で作成する方法を示します。

この例は [転送トランザクション](./transfer.md) チュートリアルと同じ流れに沿っていますが、ディスクリプタベースのトランザクション作成を、明示的なデッドラインと手数料処理を含む手動のフィールド設定に置き換えています。

残りのステップは簡潔にまとめています。
詳しくは [転送トランザクション](./transfer.md) チュートリアルを参照してください。

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

* 開発環境をセットアップしていること。
    [開発環境のセットアップ](../start/setup.md) を参照してください。
* [コード](../accounts/create-from-private-key.md) または
    [ウォレットを使用して](../../userbook/wallet/create-account.md)、 [アカウント](default:アカウント) を作成していること。
* トランザクション手数料を支払うための <XYM:> を入手していること。
    [蛇口 (Faucet) からの通貨の入手](../accounts/testnet-faucet.md) を参照してください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/transactions/manual_transaction_creation') }}

## コード解説 {: #code-explanation }

### アカウントのセットアップ {: #setting-up-the-account }

{{ tutorial.code_snippet_tagged('step-1') }}

署名者アカウントは、 `SIGNER_PRIVATE_KEY` 環境変数から読み込まれます。
指定されていない場合は、デフォルトでテストキーが使用されます。

### ネットワーク時間の取得 {: #fetching-network-time }

{{ tutorial.code_snippet_tagged('step-2') }}

手動トランザクション作成では、 [ネットワーク時間](default:ネットワーク時間) で表される絶対的なデッドラインが必要です。
ネットワーク時間は、 [ネメシスブロック](default:ネメシスブロック) からの経過ミリ秒数として測定されます。

ディスクリプタベースのトランザクション作成を使用する場合、SDK は代わりに現在からの秒数で表したデッドラインの期間を受け取るため、現在のネットワーク時間を取得する必要はありません。

このスニペットは、 <get:/node/time> から現在のネットワーク時間を取得し、後でトランザクションのデッドラインを設定するために保存します。
アプリケーションはトランザクションごとにネットワーク時間を照会する必要はありません。一度取得すれば、その後はローカルのシステムクロックを使用して調整できます。

!!! info "デッドラインのチェック"

    トランザクションのデッドラインが現在のネットワーク時間より前、または未来に離れすぎている場合、トランザクションは拒否されます。

### 推奨手数料の取得 {: #fetching-recommended-fees }

{{ tutorial.code_snippet_tagged('step-3') }}

Symbol のトランザクションは、ノードにトランザクションをブロックに含めるインセンティブを与えるために手数料を支払う必要があります。
このスニペットは、 <get:/network/fees/transaction> から推奨手数料乗数を取得し、トランザクション作成後に使用できるよう保存します。

### トランザクションの構築 {: #building-the-transaction }

{{ tutorial.code_snippet_tagged('step-4') }}

トランザクションは、プレーンなディスクリプタオブジェクトを受け取る <dy:SymbolTransactionFactory.create> で作成されます。
<dy:SymbolFacade.createTransactionFromTypedDescriptor> とは異なり、手動ファクトリは共通トランザクションフィールドを設定せず、手数料も計算しません。

<dy:SymbolTransactionFactory.create> に渡されるディスクリプタには、以下が含まれます。

* {{ tutorial.var('type') }}: <ser:TransferTransactionV1|transfer_transaction_v1> を使用します。
* {{ tutorial.var('signer_public_key') }}: トランザクションに署名し、手数料を支払うアカウントです。
    転送トランザクションでは、転送されるモザイクの送信元でもあります。
* {{ tutorial.var('deadline') }}: ネットワーク時間で表される絶対的なデッドラインです。
* {{ tutorial.var('recipient_address') }}: この例では、受信者は送信者と同じです。
* {{ tutorial.var('mosaics') }}: 送信するモザイクです。
    この例では 1 XYM を送信します。XYM の [可分性](default:可分性) は 6 であるため、 `1_000_000` 絶対単位として表します。

トランザクションが作成されると、そのサイズが分かります。
最終的な手数料は <dy:FeeCalculator.calculateTransactionFee> を使用して計算されます。
これは、そのサイズに推奨手数料乗数を掛けた値で、トランザクションの `fee` フィールドに設定されます。

### 署名とシリアライズ {: #signing-and-serializing }

{{ tutorial.code_snippet_tagged('step-5') }}

トランザクションは <dy:SymbolFacade.signTransaction> で署名されます。
その後、 <dy:SymbolTransactionFactory.attachSignature> で署名を付加し、ノードにアナウンスできる JSON ペイロードを生成します。

### トランザクションのアナウンス {: #announcing-the-transaction }

{{ tutorial.code_snippet_tagged('step-6') }}

トランザクションは、署名済みペイロードを <put:/transactions> に送信することでアナウンスされます。

### 承認の待機 {: #waiting-for-confirmation }

{{ tutorial.code_snippet_tagged('step-7') }}

アナウンス後、トランザクションが承認されるか失敗するまで、 <get:/transactionStatus/{hash}> を使用してステータスを監視します。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="2 4 8 13 14 19 21"
--8<-- 'devbook/transactions/manual_transaction_creation.log'
```

出力の主なポイントは次のとおりです。

* **2 行目:** コードは現在のネットワーク時間を明示的に取得します。
* **4 行目:** コードは推奨手数料乗数を取得します。
* **8 行目** (`signature`): トランザクションを出力する前に、署名はすでに付加されています。
* **13 行目** (`fee`): 手数料はトランザクション作成後に計算されています。
* **14 行目** (`deadline`): デッドラインは絶対的なネットワーク時間のタイムスタンプです。
* **19 行目:** 署名済みペイロードがネットワークにアナウンスされます。
* **21 行目:** トランザクションハッシュを使って、
    [Symbol Testnet Explorer](https://testnet.symbol.fyi/) でトランザクションを検索できます。

## 結論 {: #conclusion }

このチュートリアルでは、トランザクションを手動で作成する方法を説明しました。

| ステップ                                              | 関連ドキュメント                                                               |
|---------------------------------------------------|--------------------------------------------------------------------------------|
| [ネットワーク時間の取得](#fetching-network-time)        | <get:/node/time>                                                               |
| [推奨手数料の取得](#fetching-recommended-fees)       | <get:/network/fees/transaction>                                                |
| [トランザクションの構築](#building-the-transaction)     | <dy:SymbolTransactionFactory.create>                                           |
| [手数料の計算](#building-the-transaction)             | <dy:FeeCalculator.calculateTransactionFee>                                     |
| [署名とシリアライズ](#signing-and-serializing)         | <dy:SymbolFacade.signTransaction><br/><dy:SymbolTransactionFactory.attachSignature> |
| [アナウンスと承認](#announcing-the-transaction)        | <put:/transactions><br/><get:/transactionStatus/{hash}>                        |
