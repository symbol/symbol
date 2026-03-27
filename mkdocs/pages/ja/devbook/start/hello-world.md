---
title: Hello World
---

# Hello World

このチュートリアルでは、以下を行う最小限のプログラムを記述することで、Symbol SDK のインストールが正しく機能しているかを確認する方法を説明します。 [cite: 30, 31, 32]

* SDK を使用してネットワーク名と開始日を取得します。 [cite: 33, 34, 35]
* [ノード] (default: ノード) に接続し、現在のブロックチェーン高を表示します。 [cite: 79]

[アカウント] (default: アカウント) 、鍵、 [トランザクション] (default: トランザクション) は必要ありません。基本的な SDK 呼び出しと REST リクエストのみを使用します。 [cite: 70, 78]

## 前提条件

まだ完了していない場合は、[開発環境のセットアップ](../start/setup.md) から始めてください。 [cite: 14]

## 完全なコード

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/start/hello-world', ['py', 'js']) }}

### SDK の呼び出し

{{ tutorial.code_snippet(['py:7:11', 'js:6:10']) }}

<dy:SymbolFacade> クラスは、Symbol SDK への主要なエントリポイントです。 [cite: 16]
[トランザクション] (default: トランザクション) の構築や署名から、ネットワーク関連情報の取得まで、Symbol を操作する際に必要となるほとんどのメソッドを提供します。 [cite: 78]

ファサードを作成するには、操作したいネットワーク名（ `mainnet` または `testnet` ）を指定するだけです。 [cite: 33, 34, 35]

この例では、ネットワークの開始日を取得する方法を実演しています。 [cite: 35]
<dy:NetworkTimestampDatetimeConverter.toDatetime> メソッドは、ネットワークタイムスタンプを UTC の日時に変換します。 [cite: 35]
`0` （ジェネシスタイムスタンプ）を渡すことで、ジェネシス [ブロック] (default: ブロック) が生成された瞬間、つまりネットワークの開始日を取得できます。 [cite: 4]

### ノードからの情報取得

{{ tutorial.code_snippet(['py:13:27', 'js:12:28']) }}

Symbol [ブロックチェーン] (default: ブロックチェーン) との対話は、ネットワーク状態の照会や [トランザクション] (default: トランザクション) の送信のための REST インターフェースを公開している [API ノード] (default: API ノード) を通じて行われます。 [cite: 4, 7, 78, 79]
すべての [ノード] (default: ノード) がこのインターフェースを提供しているわけではないため、 **API ノード** とラベル付けされたものに接続することが重要です。 [cite: 7, 79]

この例では、 [API ノード] (default: API ノード) に接続し、 <get:/chain/info> エンドポイントから現在のブロックチェーン高を取得します。 [cite: 4, 7]

このリクエストには [秘密鍵] (default: 秘密鍵) や認証は必要ありません。環境が正しくセットアップされ、ネットワークに到達できることを確認するためのシンプルで効果的なテストとなります。 [cite: 71]

## 出力

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text
--8<-- 'devbook/start/hello-world.log'
```

## 結論

上記の出力が得られれば、準備は完了です。 [cite: 14]
Symbol SDK にアクセスでき、Symbol [API ノード] (default: API ノード) への到達に成功しました。 [cite: 7]

Symbol のアドベンチャーを始めるために必要なものはこれだけです。

次は [アカウントを作成](../accounts/create-from-private-key.md) してみませんか？ [cite: 70]