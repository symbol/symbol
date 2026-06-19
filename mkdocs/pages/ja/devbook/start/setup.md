---
title: セットアップ
tutorial_level: beginner
---

# 開発環境のセットアップ

このページでは、本ドキュメントのチュートリアルを実行するために必要な依存関係と、その実行方法について説明します。

希望する言語を選択してください。

=== ":simple-python: Python"

    <table markdown class="setup">
    <tr markdown><td>前提条件</td><td markdown>[Python](https://www.python.org/downloads/) 3.9.2 以降</td></tr>
    <tr markdown><td>インストール</td><td markdown>
    以下のコマンドで Symbol SDK バージョン 3.3.0 をインストールします。
    ```bash
    pip install symbol-sdk-python --upgrade
    ```
    </td></tr>
    <tr markdown><td>サンプルコードの実行</td><td markdown>
    サンプルをダウンロードし、以下のコマンドで実行します。
    ```bash
    python hello-world.py
    ```
    </td></tr></table>

=== ":simple-javascript: JavaScript"

    <table markdown class="setup">
    <tr markdown><td>前提条件</td><td markdown>現在サポートされている任意のバージョンの [Node.js](https://nodejs.org/)</td></tr>
    <tr markdown><td>インストール</td><td markdown>
    プロジェクトフォルダを作成し、Symbol SDK バージョン 3.3.0 を依存関係としてインストールします。
    ```bash
    mkdir symbol-dev && cd symbol-dev
    npm init -y
    npm install symbol-sdk
    ```
    </td></tr>
    <tr markdown><td>サンプルコードの実行</td><td markdown>
    サンプルをダウンロードし、以下のコマンドで実行します。
    ```bash
    node hello-world.mjs
    ```
    </td></tr></table>

## 次のステップ {: #next-steps }

* [Hello World アプリケーションの作成](./hello-world.md) へ進む

<style>
.md-typeset .tabbed-labels a {
    font-size: large;
}
table.setup {
    border-collapse:collapse;
}
table.setup td {
    border: 1px solid var(--md-default-bg-color--light);
    padding: 0.5rem;
}
.md-typeset table.setup td:first-child {
    white-space:nowrap;
}
.md-typeset table.setup td:last-child {
    width: 100%;
}
.md-typeset table.setup pre {
    margin-bottom: 0;
}
</style>
