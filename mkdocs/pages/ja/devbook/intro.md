---
title: ようこそ
---

# 開発者マニュアルへようこそ

この開発者マニュアルは、Symbol上でアプリケーションを構築する開発者を対象としています。  
<SDK:>やHTTP APIを使用して一般的なタスクを実行する方法を、PythonおよびJavaScriptのコード例とともに解説します。

本書は以下の構成になっています。

<div class="icon-list" markdown>

* :material-laptop: **はじめに**

    開発用マシンのセットアップ方法と、環境が正しく動作していることを確認するための簡単な`Hello World`サンプルを紹介します。

* :material-school: **チュートリアル**

    各チュートリアルは単一のタスクに焦点を当て、領域ごとに整理されています。  
    必要に応じて、背景知識として[テキストブック](../textbook/intro.md)や関連するリファレンスガイドへリンクしています。

* :material-book-open-page-variant: **リファレンスガイド**

    SDKメソッド、HTTPおよびWebSocketsエンドポイント、バイナリ構造に関する網羅的な情報を提供します。

</div>

ナビゲーションメニューを利用するか、以下のチュートリアルから直接始めることもできます。

以下のチュートリアルは、必要とされる知識レベルに応じて、初級から上級まで分類されています。

{% import 'tutorials_table.jinja2' as tutorials_table with context %}

{{ tutorials_table.render() }}
