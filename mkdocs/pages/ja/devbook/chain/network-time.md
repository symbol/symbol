---
title: ネットワーク時間
tutorial_level: beginner
---

# ネットワーク時間 {: #network-time }

Symbol ブロックチェーン における時間は「ネットワーク時間」で測定されます。これは、チェーンの最初の [ブロック](default:ブロック) から経過したミリ秒数として定義されます。

この簡単なチュートリアルでは、API 呼び出しによって返されるネットワーク時間を [UTC](https://ja.wikipedia.org/wiki/%E5%8D%94%E5%AE%9A%E4%B8%96%E7%95%8C%E6%99%82) のような標準的なタイムスタンプに変換する方法を説明します。

## 前提条件 {: #prerequisites }

このチュートリアルでは、SDK を必要とせずに [Symbol REST API](../reference/rest/symbol.md) を使用します。
HTTP リクエストを行う方法さえあれば実行可能です。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/chain/network_time', ['py', 'js']) }}

## コード解説 {: #code-explanation }

コードはネメシスブロックのタイムスタンプと現在のネットワーク時間を取得します。
その後、それらを足し合わせて標準的な UTC での現在時刻を求めます。

### ネメシスタイムスタンプの取得 {: #fetch-nemesis-timestamp }

{{ tutorial.code_snippet_tagged('step-1') }}

ネメシスブロックの作成時間は固定のネットワークプロパティであり、 <get:/network/properties> エンドポイントを使用して取得できます。
返される値（ `epochAdjustment` ）は、 `s` サフィックスを削除して整数に変換すると、 [UNIX タイムスタンプ](https://ja.wikipedia.org/wiki/UNIX%E6%99%82%E9%96%93) になります。
つまり、UNIX エポック（1970年1月1日 00:00:00 UTC）から経過したうるう秒を除いた秒数です。

このチュートリアルでは説明の目的でネットワークから取得していますが、これは固定値であるため、必要に応じて定数として扱い、ハードコードすることも可能です。
Symbolの [メインネット](default: メインネット) の場合、値は `1615853185` であり、これは `2021-03-16T00:06:25Z` （2021年3月16日 午前0時06分25秒 UTC）に相当します。

### 現在のネットワーク時間の取得 {: #fetch-current-network-time }

{{ tutorial.code_snippet_tagged('step-2') }}

照会先の [ノード](default: ノード) が認識している現在のネットワーク時間は、 <get:/node/time> エンドポイントを使用して取得されます。
ネットワーク内のノードは通常同期されているため、ほぼ同じ時間を返します。

実際に照会されるプロパティは `communicationTimestamps.receiveTimestamp` であり、これはリクエストがノードによって受信された時間を表します。

この値は [ネットワーク時間](default: ネットワーク時間) 、つまりネメシスブロックが作成されてから経過したミリ秒数で表されます。

### 変換 {: #conversion }

現在のネットワーク時間から UTC への変換は、単位（秒またはミリ秒）を揃えることに注意しながら、2つの数値を足し合わせるだけで済みます。

{{ tutorial.code_snippet_tagged('step-3') }}

このチュートリアルでは、プロセスを示すために手動で加算を行っています。
Symbol SDK を使用している場合は、 <dy:NetworkTimestampDatetimeConverter> がより便利な抽象化を提供します。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="5 7"
--8<-- 'devbook/chain/network_time.log'
```

5行目はネメシスブロックのタイムスタンプを示しており、常に同じ値を表示します。

7行目は、UTC に変換された現在のネットワーク時間を示しています。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                | 関連ドキュメント                                     |
|-----------------------------------------------------|------------------------------------------------|
| [ネメシスタイムスタンプの取得](#fetch-nemesis-timestamp)        | <get:/network/properties>                      |
| [現在のネットワーク時間の取得](#fetch-current-network-time) | <get:/node/time>                               |
| [両方の結合](#conversion)                            | <dy:NetworkTimestampDatetimeConverter> (オプション) |
