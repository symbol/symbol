---
title: テストネットの使用
---

# テストネットの使用

このページでは、Symbol モバイルウォレットアプリを [メインネット](default:メインネット) と [テストネット](default:テストネット) の間で切り替える方法を説明します。

メインネットは実際に稼働している Symbol ネットワークで、トランザクションには実際の価値を持つ資金が使用されます。
テストネットは、テストや学習に使用される別のネットワークです。
メインネットと同じように動作しますが、現実世界の価値を持たないテスト用資金を使用します。

テストネットを使うと、実際の資金を危険にさらさずにブロックチェーンに慣れることができます。
テストネットの <XYM:> は、[フォーセットからテストネット資金を取得する](../../devbook/accounts/testnet-faucet.md) ガイドで入手できます。

!!! warning "メインネットとテストネットは別なネットワークです"

    メインネットとテストネットは独立したネットワークです。
    一方のネットワーク上のアカウント、残高、トランザクション履歴は、もう一方とは同じになりません。

## 前提条件

* Symbol モバイルウォレットがインストールされていることを確認してください。
    まだインストールしていない場合は、[アプリのインストール](./install.md) ガイドを参照してください。

* すでにウォレットが設定され、アプリでロック解除されている必要があります。
    必要に応じて、[ウォレットの作成](./create-wallet.md) または [ウォレットのインポート](./import-wallet.md) を参照してください。

## テストネットワークの使用方法

メインネットとテストネットを切り替えるには、次の手順に従ってください。

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
ウォレットの **:material-home: HOME** 画面で、右上の **:material-cog: Settings** ボタンをタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-1.webp") }}
**:octicons-database-24: Network** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("network-2.webp") }}
**NETWORK TYPE** ドロップダウンをタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("network-4.webp") }}
使用するネットワークを選択します。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

数秒後、**Network** 画面に戻ります。
**CONNECTED NODE INFO** ボックスには、選択したネットワークの更新された情報が表示されます。

同じウォレットから派生した場合でも、メインネットとテストネットのアカウントは異なる場合があります。
それらは別々のネットワークに属しており、残高とトランザクション履歴も独立しています。
