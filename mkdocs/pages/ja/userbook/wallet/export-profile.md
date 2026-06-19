---
title: プロファイルのエクスポート
---

# プロファイルのエクスポート

このページでは、Symbol Desktop Walletから [プロファイル](default:プロファイル) とそれに含まれるすべてのアカウントをエクスポートする方法を説明します。  
プロファイルをエクスポートすることで、バックアップを作成したり、別のデバイスに[プロファイルをインポート](./import-profile.md)して同じアカウントを使用したりすることができます。

Symbol Desktop Walletは、プロファイルをペーパーウォレットとしてエクスポートします。

ペーパーウォレット
:   ウォレットのプロファイルとそれに含まれるすべてのアカウントを復元するための情報を含む、印刷可能なファイルです。

    このファイルは紙に印刷することを目的としており、QRコード、 [秘密鍵](default:秘密鍵) 、 [ニーモニックフレーズ](default:ニーモニックフレーズ) が含まれています。

    情報は手動で入力する必要があるため、ペーパーウォレットはデジタルバックアップよりも利便性に劣ります。  
    ただし、オンラインで保存されたりアクセスされたりすることがないため、より安全です。

!!! danger "ペーパーウォレットを安全に保管してください"

    ペーパーウォレットにアクセスできる人は誰でも、そこに含まれるすべてのアカウントを完全にコントロールできます。

    安全な場所に保管し、デジタル形式であれ印刷したものであれ、**絶対に誰とも共有しないでください**。

## 前提条件 {: #prerequisites }

* Symbol Desktop Walletがインストールされていることを確認してください。  
まだインストールしていない場合は、[ウォレットのインストール](./install.md)ガイドを参照してください。

* [新しいプロファイルの作成](./create-profile.md) または [既存のプロファイルのインポート](./import-profile.md) によって、Symbol Desktop Walletにすでにプロファイルが設定されている必要があります。

## プロファイルのエクスポート方法 {: #how-to-export-a-profile }

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/export-profile-0.jpg") }}
Symbol Desktop Walletを開き、エクスポートしたいプロファイルにログインします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-1.jpg") }}
**Accounts** （アカウント）タブを選択します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-2.jpg") }}
**:material-download: Backup Profile** （プロファイルのバックアップ）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-3.jpg") }}
プロファイルのパスワードを入力してアクセスを確認します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-4.jpg") }}
**:material-download: Download** （ダウンロード）ボタンをクリックします。

[ペーパーウォレット](default:ペーパーウォレット) を含むPDFドキュメントがダウンロードされます。  
セキュリティのため、このドキュメントは印刷した後にデバイスから削除してください。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-5.jpg") }}
この画像は、エクスポートされた [ニーモニックフレーズ](default:ニーモニックフレーズ) を含むペーパーウォレットの例を示しています。

このフレーズを使用して、そこから派生したすべてのアカウントを復元できます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/export-profile-6.jpg") }}
この画像は、個別のアカウントを含むペーパーウォレットの例を示しています。

このアカウントは、ニーモニックフレーズから生成されたか、 [秘密鍵](default:秘密鍵) から直接インポートされたものです。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

!!! danger "ペーパーウォレットは秘密にしてください"

    上の画像は空のテストアカウントのものであり、説明のみを目的としています。

    ペーパーウォレットにアクセスできる人は誰でも、そこに含まれるすべてのアカウントを完全にコントロールできます。

    デジタル形式であれ印刷したものであれ、**ペーパーウォレットは絶対に誰とも共有しないでください**。

## 次のステップ {: #next-steps }

これで、プロファイル内のすべてのアカウントがペーパーウォレットに安全にバックアップされました。

以下のことが可能です。

* アクセスを復元する必要が生じた場合に備えて、バックアップを安全な場所に保管する。
* 新しいデバイスや新しくインストールしたSymbol Desktop Walletで復元するには、[プロファイルのインポート](./import-profile.md) チュートリアルに従う。
