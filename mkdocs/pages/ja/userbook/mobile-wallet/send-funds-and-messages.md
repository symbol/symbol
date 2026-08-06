---
title: 資金とメッセージの送信
---

# 資金とメッセージの送信

このページでは、Symbol モバイルウォレットアプリのアカウントから資金とメッセージを送信する方法を説明します。

資金とメッセージは [転送トランザクション](default:転送トランザクション) を使用して送信します。
転送トランザクションでは、1 つ以上の [モザイク](default:モザイク) を別のアカウントへ送信でき、必要に応じて
[メッセージ](../../textbook/transfer_transactions.md#optional-message)を添付できます。
モザイクを送信せずにメッセージだけを送ることもできます。

## 前提条件

* Symbol モバイルウォレットがインストールされていることを確認してください。
    まだインストールしていない場合は、[アプリのインストール](./install.md) ガイドを参照してください。

* すでにウォレットが設定され、アプリでロック解除されている必要があります。
    必要に応じて、[ウォレットの作成](./create-wallet.md) または [ウォレットのインポート](./import-wallet.md) を参照してください。

* 送信元アカウントには、送信額とトランザクション手数料を支払うのに十分な残高が必要です。

## 資金とメッセージの送信方法

転送トランザクションを送信するには、次の手順に従ってください。

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
ウォレットの **:material-home: HOME** 画面で、アカウントボックス中央の **:fontawesome-regular-paper-plane: SEND** ボタンをタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-1.webp") }}
転送フォームに入力します。

* **RECIPIENT** ボックスに受信者のアドレスを貼り付けるか、ボックス内の
    **:fontawesome-regular-address-book: アドレス帳** ボタンを使用して自分のアカウントの 1 つを選択します。

* 送信する **MOSAIC** を選択し、**AMOUNT** を入力します。

* 必要に応じてメッセージを追加し、受信者向けに暗号化する場合は **ENCRYPTED** にチェックを入れます。

必須項目が入力されるまで、**SEND** ボタンは無効のままです。

次のステップでは、入力済みのフォームを確認します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-2.webp") }}
必須項目を入力すると、手数料スライダーが表示されます。

スライダーを動かして、手数料と承認速度を選択します。

手数料が低いほど通常はトランザクションが遅くなりますが、実際の承認時間はネットワーク状況によって異なります。
接続先ノードの最小手数料乗数も、手数料をどこまで低くできるかに影響します。
接続先ノードの確認や変更については、[ノードの変更](./change-node.md) ガイドを参照してください。

**SEND** をタップします。
次の画面で確認を求められます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-3.webp") }}
トランザクションの詳細を確認します。

問題がなければ、**CONFIRM** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-4.webp") }}
アプリがトランザクションを作成、署名、アナウンスするまで待ちます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("transfer-5.webp") }}
転送トランザクションが承認されると、アプリに成功メッセージが表示されます。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

## 次のステップ

転送を送信した後は、[トランザクション履歴の確認](./check-transaction-history.md)を行うと、送信者または受信者の視点から確認できます。
