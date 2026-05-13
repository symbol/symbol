---
title: マルチシグでの署名
tutorial_level: intermediate
---

# マルチシグアカウントからのトランザクション署名 {: #signing-a-transaction-from-a -multisignature-account }

このチュートリアルでは、[転送トランザクションの作成](../transactions/transfer.md) チュートリアルと同様に、[アカウント](default: アカウント) から自身へ 1 [XYM](default: XYM) を転送します。

ただし、このケースでは送信元が [マルチシグアカウント](default: マルチシグアカウント)（「マルチシグ」とも呼ばれます）であるため、アカウント単体でトランザクションを開始したり署名したりすることはできません。代わりに、連署者アカウントのいずれかがトランザクションを作成し、代理で署名を行います。

このチュートリアルでは、[マルチシグアカウントの設定](./configure-multisig.md) チュートリアルで作成したマルチシグ構成を使用し、連署者 0 がトランザクションの開始と署名を行います。

```dot
digraph "Multisignature Tree" {
    rankdir="BT";
    node [fontsize=12];
    "マルチシグアカウント";
    "連署者 0";
    "連署者 1";

    "連署者 0" -> "マルチシグアカウント";
    "連署者 1" -> "マルチシグアカウント";
}
```

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

* 開発環境をセットアップしていること。
    [開発環境のセットアップ](../start/setup.md) を参照してください。
* [マルチシグアカウントの設定](./configure-multisig.md) チュートリアルを完了していること。

さらに、トランザクションがどのようにアナウンスされ承認されるかを理解するために [転送トランザクション](../transactions/transfer.md) チュートリアルを、[アグリゲートトランザクション](default: アグリゲートトランザクション) の仕組みを理解するために [アグリゲートコンプリートトランザクション](../transactions/complete-aggregate.md) のチュートリアルを復習しておいてください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/accounts/sign_multisig', ['py', 'js']) }}

## コード解説 {: #code-explanation }

一般的に、マルチシグアカウントの代理でトランザクションに署名するには、必要な連署を提供する [アグリゲートトランザクション](default:アグリゲートトランザクション) でラップするだけで済みます。

このチュートリアルでは、転送の起点となるマルチシグアカウントを署名者として、転送内容を含む [埋め込みトランザクション](default:埋め込みトランザクション) を構築します。その後、トランザクションを承認できるアカウントである連署者が署名した [アグリゲートコンプリートトランザクション](default:アグリゲートコンプリートトランザクション) で、その転送トランザクションをラップします。

### アカウントの設定 {: #setting-up-the-accounts }

{{ tutorial.code_snippet_tagged('step-1') }}

このチュートリアルでは2つの個別のアカウントが必要です。
それらの [秘密鍵](default:秘密鍵) は環境変数を通じて提供できます。設定されていない場合は、デフォルト値が使用されます。

| 環境変数                   | デフォルト値      | 用途        |
|----------------------------|--------------|-------------|
| `MULTISIG_PRIVATE_KEY`     | `0000..0001` | マルチシグアカウント  |
| `COSIGNATORY0_PRIVATE_KEY` | `0000..0002` | 連署者アカウント |

各秘密鍵は64文字の16進数文字列です。

連署者アカウントは、トランザクション手数料を支払うのに十分な資金を保有している必要があります。デフォルト値を使用する場合、これらのアカウントにはすでに資金が供給されている可能性があります。

上記のスニペットは、後で使用するために各アカウントの [キーペア](default:キーペア) を派生させて保存します。

### ネットワーク時間と手数料の取得 {: #fetching-network-time-and-fees }

{{ tutorial.code_snippet_tagged('step-2') }}

[転送トランザクション](../transactions/transfer.md) チュートリアルで説明されているプロセスに従い、ネットワーク時間と推奨手数料をそれぞれ <get:/node/time> および <get:/network/fees/transaction> から取得します。

### トランザクションの構築 {: #building-the-transaction }

{{ tutorial.code_snippet_tagged('step-3') }}

埋め込まれた [転送トランザクション](default:トランザクション) には以下のフィールドが含まれます。

* `signer_public_key`: 資金の転送元となるアカウント、つまりマルチシグアカウントの [公開鍵](default: 公開鍵)。

* `recipient_address`: この例では、資金は送信元に戻されるため、受信者もマルチシグアカウントになります。

* `mosaics`: [転送トランザクション](../transactions/transfer.md) チュートリアルで説明されているように、1 [XYM](default:XYM) に相当する `symbol.xym` モザイクの 1,000,000 絶対量。

埋め込みトランザクションは、内部トランザクションが1つだけの場合でも、アグリゲートトランザクションにラップされます。

{{ tutorial.code_snippet_tagged('step-4') }}

主なフィールドは以下の通りです。

* `signer_public_key`: 今回は、トランザクションを承認し、その手数料を支払う連署者の [公開鍵](default: 公開鍵) です。

* `transactions`: 埋め込みトランザクションのリスト。この例では1つだけですが、いくつでも含めることができます。

簡単にするため、このチュートリアルでは [アグリゲートコンプリートトランザクション](default:アグリゲートコンプリートトランザクション) を使用します。
詳細については、[コンプリート](../transactions/complete-aggregate.md) および [ボンデッド](../transactions/bonded-aggregate.md) アグリゲートトランザクションのチュートリアルを参照してください。

最後に、連署者によってアグリゲートトランザクションに署名が行われます。

{{ tutorial.code_snippet_tagged('step-5') }}

!!! note "複数の連署者"

    他のマルチシグ構成では、より多くの署名が必要になる場合があります。その場合、 <dy:SymbolFacade.signTransaction> ではなく <dy:SymbolFacade.cosignTransaction> を使用して署名を付加します。

    例については [マルチシグアカウントの設定](./configure-multisig.md) チュートリアルを参照してください。

### アグリゲートトランザクションの送信 {: #submitting-the-aggregate-transaction }

最後のステップは、[転送トランザクション](../transactions/transfer.md) チュートリアルで説明されているように、トランザクションをアナウンスして承認を待つことです。

{{ tutorial.code_snippet_tagged('step-6') }}

プロトコル制約に違反した場合、トランザクションは拒否されます。以下の表は、最も一般的なエラーの原因をまとめたものです。

| エラーメッセージ                                 | 考えられる原因                                                   |
|------------------------------------------|--------------------------------------------------------------|
| Multisig Operation Prohibited By Account | マルチシグアカウント自体がアグリゲートトランザクションに署名しようとした。                    |
| Aggregate Ineligible Cosignatories       | 署名者が連署者リストに含まれていない。                                   |
| Consumer Batch Signature Not Verifiable  | アグリゲートトランザクションに付加された署名が、その `signer_public_key` と一致しない。 |

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="2-3 11 20 24"
--8<-- 'devbook/accounts/sign_multisig.log'
```

出力の主なポイント:

* **2-3行目**: 関与するすべてのアカウントの公開鍵。
* **11行目** (`signer_public_key`): アグリゲートトランザクションの署名者。連署者アカウントと一致していることに注目してください。
* **20行目** (`signer_public_key`): 埋め込まれた転送トランザクションの署名者。マルチシグアカウントと一致していることに注目してください。
* **24行目** (`recipient_address`): エンコードされたマルチシグアカウントの [アドレス](default:アドレス)。

出力に示されているトランザクションハッシュを使用して、[Symbol Testnet Explorer](https://testnet.symbol.fyi/) でトランザクションを検索できます。

## 結論 {: #conclusion }

このチュートリアルは機能的には [転送トランザクション](../transactions/transfer.md) チュートリアルと同じですが、送信元アカウントとして [マルチシグアカウント] (default:マルチシグアカウント) を使用しています。

特に、このチュートリアルでは以下の方法を説明しました。

| ステップ                                                           | 関連ドキュメント                                   |
|----------------------------------------------------------------|----------------------------------------------|
| [転送トランザクションを埋め込みトランザクションにラップする](#building-the-transaction) | <dy:SymbolTransactionFactory.createEmbedded> |
| [適切な場所に署名を付加する](#building-the-transaction)             | <dy:SymbolFacade.signTransaction>            |
