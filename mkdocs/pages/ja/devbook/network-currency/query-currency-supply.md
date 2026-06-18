---
title: 通貨供給量の照会
tutorial_level: beginner
---

# 通貨供給量の照会 {: #querying-currency-supply }

取引所や市場データアグリゲーターは、時価総額やトークン指標を表示するために正確な供給量の数値を必要とします。

Symbolネットワークは、専用のRESTエンドポイントを通じて、ネイティブ通貨である[XYM](default: XYM)の最大供給量、総供給量、および循環供給量を公開しています。

このチュートリアルでは、それぞれの値を照会し、それらから追加の指標を導き出す方法を説明します。

## 前提条件 {: #prerequisites }

このチュートリアルでは、SDKを必要とせずに[Symbol REST API](../reference/rest/symbol.md)を使用します。
HTTPリクエストを行う方法さえあれば実行可能です。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/network-currency/query_currency_supply', ['py', 'js']) }}

このスニペットでは、 `NODE_URL` 環境変数を使用してSymbol API[ノード](default: ノード)を設定します。
値が指定されない場合は、デフォルトの[テストネット](default: テストネット)ノードが使用されます。

!!! warning "デフォルトノードはテストネットです"
    デフォルトのノードはテストネットを指しています。
    本番環境の供給量データについては、 `NODE_URL` を[メインネット](default:メインネット)ノードに設定してください。
    利用可能なメインネットノードのリストについては、[symbol.fyi/nodes](https://symbol.fyi/nodes)を参照してください。

## コード解説 {: #code-explanation }

### 供給量の値の取得 {: #fetching-supply-values }

{{ tutorial.code_snippet_tagged('step-1') }}

各供給量の値は、専用のエンドポイントを通じて利用できます。

* <get:/network/currency/supply/max>: ネットワークプロパティで設定されている、XYMのハードキャップ（最大発行上限）。
* <get:/network/currency/supply/total>: 現在までに発行（ミント）されたXYMの総量。
    新しいXYMは、新しい[ブロック](default: ブロック)ごとの[インフレーション](default:インフレーション)報酬を通じて徐々に発行されます。
* <get:/network/currency/supply/circulating>: 総供給量から、ネメシスアカウント、トレジャリー（財務）アカウント、および[ハーベスト](../../textbook/harvesting.md#sink)、[モザイクレンタル](../../textbook/mosaics.md#lease-fee)、[ネームスペースレンタル](../../textbook/namespaces.md#lease-fee)の手数料を収集するシンクアカウントが保持する残高を差し引いた値。

これら3つのエンドポイントはすべて、（JSONではなく）プレーンテキストの数値を返します。これらは[絶対（アトミック）](default: 可分性)単位ではなく、すでに小数点以下の桁数を含む全体単位（例： `8999999999.000000` ）で表されています。

!!! warning "注意: 循環供給量はノードに依存します"
    非循環アカウントのリストは各ノードオペレーターによって（ノードの `rest.json` ファイル内で）設定されるため、異なるノードが異なる循環供給量の値を報告する可能性があります。
    供給量データを統合する場合は、[デフォルト設定](https://github.com/symbol/symbol/blob/dev/client/rest/resources/rest.json)を持つ信頼できるノードを照会するようにしてください。

### 追加の指標の導出 {: #deriving-additional-metrics }

{{ tutorial.code_snippet_tagged('step-2') }}

3つの値をすべて取得した後、コードは2つの追加の指標を導き出します。

* **Non-circulating (非循環):** 総供給量と循環供給量の差。
* **Unminted (未発行):** 最大供給量と総供給量の差。今後発行される予定の残りのXYMを表します。

## 出力 {: #output }

以下の出力は、通貨供給量を照会する典型的な実行例を示しています。

```text linenums="1" hl_lines="2 3 4 5 6"
--8<-- 'devbook/network-currency/query_currency_supply.log'
```

これらの値はテストネットノードからのものであり、メインネットの供給量の数値を反映していません。

出力は、XYMの供給量の完全な内訳を示しています。

* **最大供給量 (maximum supply)** (2行目): XYMのハードキャップです。
* **総供給量 (total supply)** (3行目): すべてのXYMがまだ発行されているわけではないため、最大供給量より少なくなります。
* **循環供給量 (circulating supply)** (4行目): 一部の発行済みXYMは非循環アカウントによって保持されているため、さらに少なくなります。
* **非循環供給量 (non-circulating supply)** (5行目): 総供給量と循環供給量の差を表します。
* **未発行供給量 (unminted supply)** (6行目): インフレーション報酬を通じて徐々に発行される残りのXYMを示しています。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                           | 関連ドキュメント                    |
|------------------------------------------------|--------------------------------------------|
| [最大供給量の取得](#fetching-supply-values)     | <get:/network/currency/supply/max>         |
| [総供給量の取得](#fetching-supply-values)       | <get:/network/currency/supply/total>       |
| [循環供給量の取得](#fetching-supply-values)     | <get:/network/currency/supply/circulating> |
| [追加の指標の導出](#deriving-additional-metrics) | -                                          |

## 次のステップ {: #next-steps }

特定のアカウントのXYM残高を確認するには、[アカウント残高の照会](../accounts/query-balance.md) チュートリアルを参照してください。
