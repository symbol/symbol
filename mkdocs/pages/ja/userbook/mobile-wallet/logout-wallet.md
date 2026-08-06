---
title: ログアウト
---

# ウォレットからのログアウト

このページでは、Symbol モバイルウォレットアプリで有効なウォレットからログアウトする方法を説明します。

ログアウトすると、このデバイスからウォレットデータが削除されます。
アカウントと資産はブロックチェーン上に残りますが、ウォレットの [ニーモニックフレーズ](default:ニーモニックフレーズ) と、インポートした
[外部アカウント](default:外部アカウント)の [秘密鍵](default:秘密鍵) のコピーがない限り、**アクセスできなくなります**。

!!! warning "ログアウト前にウォレットをバックアップしてください"

    ログアウトする前に、[ウォレットのニーモニックフレーズをバックアップ](./export-wallet.md)していることを確認してください。
    ウォレットに外部アカウントが含まれる場合は、それらの[秘密鍵もバックアップ](./export-account.md)していることを確認してください。

## 前提条件

* Symbol モバイルウォレットがインストールされていることを確認してください。
    まだインストールしていない場合は、[アプリのインストール](./install.md) ガイドを参照してください。

* すでにウォレットが設定され、アプリでロック解除されている必要があります。
    必要に応じて、[ウォレットの作成](./create-wallet.md) または [ウォレットのインポート](./import-wallet.md) を参照してください。

* ウォレットの PIN コードが必要です。

## ウォレットからログアウトする方法

有効なウォレットからログアウトするには、次の手順に従ってください。

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
ウォレットの **:material-home: HOME** 画面で、右上隅の **:material-cog: Settings** ボタンをタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-1.webp") }}
**:material-minus: Logout** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("logout-wallet-2.webp") }}
確認メッセージを読みます。

ニーモニックフレーズと外部アカウントの秘密鍵がバックアップされている場合は、**Confirm** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-3.webp") }}
PIN コードを入力してウォレットのロックを解除し、ログアウトを確認します。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

ようこそ画面に戻ります。

![ようこそ画面](create-wallet-0.webp){ .tutorial-result }

## 次のステップ

このデバイス上のウォレットからログアウトしました。

* 後で同じウォレットを復元するには、[ウォレットのインポート](./import-wallet.md) に従ってください。
* 後で外部アカウントを復元するには、[アカウントのインポート](./import-account.md) に従ってください。
* 最初から始めるには、[ウォレットの作成](./create-wallet.md) に従ってください。
