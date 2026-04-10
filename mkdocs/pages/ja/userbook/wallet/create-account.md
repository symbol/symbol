---
title: アカウントの作成
---

# 新しいアカウントの作成

このページでは、Symbol Desktop Wallet の既存の [プロファイル](default:プロファイルとは) に新しい [アカウント](default:アカウント) を追加する方法を説明します。
同じプロファイル内で複数のアカウントを管理でき、それらはすべて単一のパスワードで保護されます。

これは、資産を整理したり、異なるユースケースを分離したりする場合などに役立ちます。

アカウントの作成に通貨を消費する必要はありません。また、新しいアカウントは、トランザクションに参加するまでブロックチェーン上には反映されません。

## 前提条件

* Symbol Desktop Wallet がインストールされていることを確認してください。
まだインストールしていない場合は、[ウォレットのインストール](./install.md) ガイドを参照してください。

* すでにプロファイルが設定され、ログインしている必要があります。
必要に応じて、[プロファイルの作成](./create-profile.md) または [プロファイルのインポート](./import-profile.md) を参照してください。

## 新しいアカウントの作成方法

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/export-profile-1.jpg") }}
ウォレットのメイン画面から、 **Accounts** （アカウント）タブに移動します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/create-account-1.jpg") }}
画面下部の **:material-plus-circle: Add an account** （アカウントの追加）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/create-account-2.jpg") }}
フォームに入力して、新しいアカウントを設定します。

**Select the Type of Account** （アカウントの種類を選択）では、デフォルトのオプションである
**"I want to create a seed account for my profile"** （プロファイルのシードアカウントを作成する）を選択したままにします。

**New Account Name** （新しいアカウント名）フィールドに名前を入力します。
このラベルは、後でアカウントを識別するのに役立ちます。

**Password** （パスワード）フィールドにプロファイルのパスワードを入力して、操作を承認します。

その後、 **Confirm** （確認）をクリックしてアカウントを作成します。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

新しいアカウントはプロファイルのニーモニックフレーズから生成され、 **Seed accounts** （シードアカウント）セクションの管理対象アカウントのリストに追加されます。

![作成された新しいアカウント](screenshots/create-account-3.jpg)

これはシードアカウントであるため、ニーモニックフレーズのバックアップからいつでも再生成できます。したがって、その秘密鍵を個別にバックアップする必要はありません。

一方、[インポートされたアカウント](./import-account.md) はニーモニックフレーズから派生したものではないため、アクセスできなくなるのを防ぐために、秘密鍵を個別にバックアップする必要があります。

[プロファイルのエクスポート](./export-profile.md) を行うと、シードアカウントとインポートされたアカウントの両方が1つの <paper wallet:|ペーパーウォレット> にバックアップされます。

## 次のステップ

これで、以下のことが可能になります。

* 新しいアカウントのアドレスを共有して資金を受け取る。
* アカウントを使用して [トランザクション](default:トランザクション) を送信したり、 [モザイク](default:モザイク) やネームスペースなどのSymbolの機能とやり取りする。
