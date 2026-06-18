---
title: トランザクション包含の証明
tutorial_level: intermediate
---

# ブロックへのトランザクション包含の証明 {: #proving-a-Transaction's-inclusion-in-a-block }

Symbolの各 [ブロック](default:ブロック) は、その [トランザクション](default: トランザクション) を [マークルツリー](default:マークルツリー) （Merkle tree）に記録します。そのルート（根）である `transactionsHash` は、ブロックヘッダーに保存されます。ブロックの全トランザクションをダウンロードすることなく、このルートに対してトランザクションを検証することで、そのトランザクションがブロックに含まれていることを証明できます。

このチュートリアルでは、APIからマークル証明（Merkle proof）を取得し、特定のトランザクションがブロックの一部であることを検証する方法を説明します。

## 前提条件 {: #prerequisites }

開始する前に：

* [開発環境をセットアップ](../start/setup.md) してください。
* [ブロックハッシュ](../../textbook/blocks.md#block-hashes) の仕組み、特に `transactionsHash` マークルツリーについて復習しておいてください。

このチュートリアルではネットワークからのデータの読み取りのみを行います。 [アカウント](default:アカウント) や [XYM](default:XYM) の残高は必要ありません。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/chain/prove_transaction', ['py', 'js']) }}

このスニペットは、証明するトランザクションの [ハッシュ](default:ハッシュ) を `TRANSACTION_HASH` 環境変数から読み取ります。設定されていない場合は、Symbolテストネットのブロック `55` にある既知のトランザクションがデフォルトとして使用されます。

## コード解説 {: #code-explanation }

### 承認済みトランザクションの取得 {: #fetching-the-confirmed-transaction }

{{ tutorial.code_snippet_tagged('step-1') }}

コードは、 <get:/transactions/confirmed/{transactionId}> エンドポイントから承認済みのトランザクションを取得します。

`meta.height` フィールドはトランザクションが承認されたブロックの高さであり、次のステップでブロックヘッダーを取得するために必要です。

レスポンスには `merkleComponentHash` も含まれています。これはブロックのマークルツリーで使用されるリーフ（葉）ハッシュです。通常のトランザクションでは、この値はトランザクションハッシュと等しくなります。 [アグリゲートトランザクション](default: アグリゲートトランザクション) の場合、トランザクションハッシュと連署者の公開鍵を連結したものの SHA3-256 ハッシュとして計算されます。

### ブロックヘッダーの取得 {: #fetching-the-block-header }

{{ tutorial.code_snippet_tagged('step-2') }}

<get:/blocks/{height}> エンドポイントは、 `transactionsHash` フィールドを含むブロックのメタデータを返します。このハッシュは、ブロック内の各トランザクションの `merkleComponentHash` から構築されたマークルツリーのルートです。

コードは16進文字列を `Hash256` オブジェクトにラップします。これは <dy:Merkle.proveMerkle> 関数が期待する形式です。

### マークル証明パスの取得 {: #fetching-the-merkle-proof-path }

{{ tutorial.code_snippet_tagged('step-3') }}

<get:/blocks/{height}/transactions/{hash}/merkle> エンドポイントは、**マークル証明パス**（Merkle proof path）を返します。これは、 `merkleComponentHash` から始めて `transactionsHash` を再計算するために必要な最小限の中間ハッシュのセットです（マークルツリーの各レベルに1つずつ）。

パスの各項目には以下が含まれます：

* **hash**: ツリーの次のレベルを再計算するために必要な中間ハッシュ。
* **position**: 前の結果と結合する際に、このハッシュが「左（left）」か「右（right）」のどちらに位置するか。

コードは各項目をハッシュとブール値（ハッシュが左側の場合は `true` ）のペアに変換し、 <dy:Merkle.proveMerkle> 関数が期待する形式に合わせます。

### 証明の検証 {: #verifying-the-proof }

{{ tutorial.code_snippet_tagged('step-4') }}

<dy:Merkle.proveMerkle> は、指定された位置順序に従って `merkleComponentHash` を証明パス内の各中間ハッシュと反復的に結合することで、マークルルートを再計算します。計算されたルートがブロックの `transactionsHash` と一致すれば、そのトランザクションがブロックの一部であることが証明されます。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています：

```text linenums="1" hl_lines="5 7 15 39 40"
--8<-- 'devbook/chain/prove_transaction.log'
```

出力の主なポイント：

* **トランザクションメタデータ**（5行目と7行目）: <get:/transactions/confirmed/{transactionId}> からのJSONレスポンスには、証明に必要なブロックの高さ（ `height` ）と `merkleComponentHash` が含まれています。

* **ブロックトランザクションハッシュ**（15行目）: <get:/blocks/{height}> からのJSONレスポンスには、そのブロックで承認された全トランザクションのマークルルートである `transactionsHash` が含まれています。

* **マークルパスの長さ**（39行目）: <get:/blocks/{height}/transactions/{hash}/merkle> からのJSONレスポンスには `4` つのエントリが含まれています。これはツリーが4レベルあり、ブロックに最大 $2^4 = 16$ 個のトランザクションが含まれていることを意味します。

* **証明結果**（40行目）: 計算されたルートが `transactionsHash` と一致し、トランザクションが正真正銘ブロック `55` の一部であることが確認されました。

エクスプローラーでトランザクションやそのブロックを調査するには、 [Symbol Testnet Explorer](https://testnet.symbol.fyi/) にアクセスし、トランザクションハッシュまたはブロック高を入力してください。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました：

| ステップ                                                        | 関連ドキュメント                                        |
|-------------------------------------------------------------|---------------------------------------------------|
| [承認済みトランザクションの取得](#fetching-the-confirmed-transaction) | <get:/transactions/confirmed/{transactionId}>     |
| [ブロックヘッダーの取得](#fetching-the-block-header)                 | <get:/blocks/{height}>                            |
| [マークル証明パスの取得](#fetching-the-merkle-proof-path)          | <get:/blocks/{height}/transactions/{hash}/merkle> |
| [証明の検証](#verifying-the-proof)                           | <dy:Merkle.proveMerkle>                           |

## 次のステップ {: #next-steps }

同じ手順は、 <get:/blocks/{height}/statements/{hash}/merkle> エンドポイントを使用し、
ブロックの `receiptsHash` に対して検証することで、レシートの証明にも使用できます。
