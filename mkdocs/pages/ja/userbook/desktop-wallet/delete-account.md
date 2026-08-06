---
title: アカウントの削除
---

# プロファイルからのアカウントの削除

このページでは、Symbol Desktop Walletの [プロファイル](default:プロファイル) から既存の [アカウント](default:アカウント) を削除する方法を説明します。

この操作により、アカウントはプロファイルから削除されますが、ブロックチェーンからアカウントが削除されるわけではありません。
アカウントの秘密鍵をバックアップしている場合は、後で再度インポートすることができます。

!!! warning "警告"
    アカウントの秘密鍵がバックアップされていない場合、それに含まれる資金や資産へのアクセスは**永久に失われます**。

    削除する前に、アカウントが空であるか、適切にバックアップされていることを確認してください！

## 前提条件 {: #prerequisites }

* Symbol Desktop Wallet がインストールされていることを確認してください。  
まだインストールしていない場合は、[ウォレットのインストール](./install.md) ガイドを参照してください。

* すでにプロファイルが設定され、ログインしている必要があります。  
必要に応じて、[プロファイルの作成](./create-profile.md) または [プロファイルのインポート](./import-profile.md) を参照してください。

## アカウントの削除方法 {: #how-to-delete-an-account }

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/export-profile-1.jpg") }}
Symbol Desktop Wallet の **Accounts** （アカウント）タブに移動します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-account-1.jpg") }}
左側のリストから、削除したいアカウントをクリックします。

現在選択されているアカウントは、色付きのアイコンで表示されます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-account-2.jpg") }}
**Account Information** （アカウント情報）パネルにある **Delete Account** （アカウントの削除）ボタンをクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-account-3.jpg") }}
確認ダイアログで、アカウントを削除することを確認するためのチェックボックスをオンにします。

その後、 **Confirm** （確認）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/delete-account-4.jpg") }}
プロファイルのパスワードを入力して削除を承認し、再度 **Confirm** （確認）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

削除されたアカウントは、管理対象アカウントのリストに表示されなくなります。

![正常に削除されたアカウント](screenshots/delete-account-5.jpg)

## 次のステップ {: #next-steps }

* バックアップがある場合は、秘密鍵を使用して [アカウントを再度インポート](./import-account.md) します。
* [新しいアカウントの作成](./create-account.md) を行います。
