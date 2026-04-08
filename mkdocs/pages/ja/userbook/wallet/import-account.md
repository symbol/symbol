---
title: アカウントのインポート
---

# 既存のアカウントのインポート

このページでは、秘密鍵をインポートして既存の <account:|アカウント> をプロファイルに追加する方法を説明します。  
インポートされたアカウントはプロファイルの <mnemonic phrase:|ニーモニックフレーズ> から派生したものではないため、個別にバックアップする必要があります。

以下の場合にアカウントのインポートが必要になることがあります。

* 別のウォレットまたは別のデバイスで作成した場合。
* プロファイルのシードから生成されていないアカウントを復元する場合。

## 前提条件

* Symbol Desktop Wallet がインストールされていることを確認してください。  
まだインストールしていない場合は、[ウォレットのインストール](./install.md) ガイドを参照してください。

* すでにプロファイルが設定され、ログインしている必要があります。  
必要に応じて、[プロファイルの作成](./create-profile.md) または [プロファイルのインポート](./import-profile.md) を参照してください。

## アカウントのインポート方法

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/export-profile-1.jpg") }}
ウォレットのメイン画面から、 **Accounts** （アカウント）タブに移動します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/create-account-1.jpg") }}
画面下部の **:material-plus-circle: Add an account** （アカウントの追加）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-account-2.jpg") }}
**Select the Type of Account** （アカウントの種類を選択）ドロップダウンで、 **"I want to import an existing account private key"** （既存のアカウントの秘密鍵をインポートする）を選択します。

後でアカウントを識別できるように、 **New Account Name** （新しいアカウント名）フィールドに名前を入力します。

**Enter Your Private Key** （秘密鍵を入力）フィールドに秘密鍵を貼り付けます。

インポートを承認するために、 **Password** （パスワード）フィールドにプロファイルのパスワードを入力します。

その後、 **Confirm** （確認）をクリックしてアカウントをインポートします。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

インポートされたアカウントは、 **Private key accounts** （秘密鍵アカウント）セクションの管理対象アカウントのリストに追加されます。

![インポートされた新しいアカウント](screenshots/import-account-3.jpg)

!!! warning "警告"
    インポートされたアカウントは、 <mnemonic phrase:|ニーモニックフレーズ> から復元することはできません。

    これらの秘密鍵が <paper wallet:|ペーパーウォレット> に含まれるように、必ずすぐにプロファイルをバックアップしてください。

## 次のステップ

これで、以下のことが可能になります。

* [プロファイルのエクスポート](./export-profile.md) を行い、単一のペーパーウォレットにシードアカウントとインポートされたアカウントの両方をバックアップする。
* インポートされたアカウントを使用して、トランザクションの送受信を行う。
