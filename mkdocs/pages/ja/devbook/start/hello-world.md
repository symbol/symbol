---
title: Hello World
---

# Hello World {: #hello-world }

このチュートリアルでは、以下を行う最小限のプログラムを記述することで、Symbol SDK のインストールが正しく機能しているかを確認する方法を説明します。

* SDK を使用してネットワーク名と開始日を取得します。
* [ノード](default:ノード)に接続し、現在のブロックチェーン高を表示します。

アカウント、鍵、 トランザクションは必要ありません。基本的な SDK 呼び出しと REST リクエストのみを使用します。

## 前提条件 {: #prerequisites }

まだ完了していない場合は、[開発環境のセットアップ](../start/setup.md) から始めてください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/start/hello-world', ['py', 'js']) }}

### SDK の呼び出し {: #making-sdk-calls }

{{ tutorial.code_snippet(['py:7:11', 'js:6:10']) }}

<dy:SymbolFacade> クラスは、Symbol SDK への主要なエントリポイントです。
トランザクションの構築や署名から、ネットワーク関連情報の取得まで、Symbol を操作する際に必要となるほとんどのメソッドを提供します。

ファサードを作成するには、操作したいネットワーク名（ `mainnet` または `testnet` ）を指定するだけです。

この例では、ネットワークの開始日を取得する方法を実演しています。
<dy:NetworkTimestampDatetimeConverter.toDatetime> メソッドは、ネットワークタイムスタンプを UTC の日時に変換します。
`0` （ジェネシスタイムスタンプ）を渡すことで、ジェネシスブロックが生成された瞬間、つまりネットワークの開始日を取得できます。

### ノードからの情報取得 {: #retrieving-information-from-a-node }

{{ tutorial.code_snippet(['py:13:27', 'js:12:28']) }}

Symbolブロックチェーンとの対話は、ネットワーク状態の照会やトランザクションの送信のための REST インターフェースを公開している [API ノード](default:API ノード)を通じて行われます。
すべてのノードがこのインターフェースを提供しているわけではないため、 **API Node** とラベル付けされたものに接続することが重要です。

この例では、APIノードに接続し、 <get:/chain/info> エンドポイントから現在のブロックチェーン高を取得します。

このリクエストには秘密鍵や認証は必要ありません。環境が正しくセットアップされ、ネットワークに到達できることを確認するためのシンプルで効果的なテストとなります。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text
--8<-- 'devbook/start/hello-world.log'
```

## 結論 {: #conclusion }

上記の出力が得られれば、準備は完了です。
Symbol SDK にアクセスでき、Symbol API ノードへの到達に成功しました。

Symbol の冒険を始めるために必要なものはこれだけです。

次は [アカウントを作成](../accounts/create-from-private-key.md) してみませんか？
