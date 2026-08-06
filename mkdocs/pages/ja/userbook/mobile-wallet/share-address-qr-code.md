---
title: QR コードでアドレスを共有
---

# QR コードでアドレスを共有

このページでは、[QR コード](https://en.wikipedia.org/wiki/QR_code)を使用して、Symbol モバイルウォレットアプリから
[アカウント](default:アカウント) アドレスを共有する方法を説明します。

アカウントの [アドレス](default:アドレス) は QR コードに変換でき、別な端末の Symbol モバイルウォレットで簡単にスキャンできます。
スキャンされたアドレスは、単発のトランザクションの受信者として使用したり、後で使うためにスキャン側ウォレットの
アドレス帳に追加したりできます。

## 前提条件

* Symbol モバイルウォレットがインストールされていることを確認してください。
    まだインストールしていない場合は、[アプリのインストール](./install.md) ガイドを参照してください。

* すでにウォレットが設定され、アプリでロック解除されている必要があります。
    必要に応じて、[ウォレットの作成](./create-wallet.md) または [ウォレットのインポート](./import-wallet.md) を参照してください。

* 別な端末にも Symbol モバイルウォレットがインストールされている必要があります。

## QR コードでアドレスを共有する方法

アカウントアドレスを QR コードとして表示し、別な端末でスキャンするには、次の手順に従ってください。

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("share-address-0.webp") }}
ウォレットの **:material-home: HOME** 画面で、アカウントドロップダウンから共有したいアカウントを選択します。

この例では、**Second account** が選択されています。

アカウントボックス左側の **:fontawesome-regular-user: DETAILS** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("share-address-1.webp") }}
アカウント詳細画面で、**:octicons-download-24: RECEIVE** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("share-address-2.webp") }}
アカウントアドレスの QR コードが表示されます。

そのまま画面に表示しておきます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("create-wallet-5.webp") }}
別な端末で **:material-home: HOME** 画面に移動し、下部の **:material-line-scan: SCAN** をタップして、
カメラを QR コードに向けます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("share-address-3.webp") }}
別な端末にに **SHARED INFORMATION** 画面が表示され、共有されたアドレスと利用可能なアクションの一覧が表示されます。

**:fontawesome-regular-paper-plane: Send Transaction** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("share-address-4.webp") }}
**TRANSFER** フォームが表示され、**RECIPIENT** フィールドには共有されたアドレスがすでに入力されています。

このガイドはここで終了しますが、[資金とメッセージの送信](./send-funds-and-messages.md) に従ってトランザクションを完了できます。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}
