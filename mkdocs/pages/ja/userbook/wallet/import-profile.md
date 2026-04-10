---
title: プロファイルのインポート
---

# 既存のプロファイルのインポート

このページでは、以前にインストールした Symbol Desktop Wallet、または他の互換性のあるウォレットから、**既存の** [プロファイル](default:プロファイル) を復元する方法を説明します。  
プロファイルとは何かを知りたい場合、またはゼロから新しく作成したい場合は、代わりに [プロファイルの作成](./create-profile.md) チュートリアルを参照してください。

## 前提条件

* Symbol Desktop Wallet がインストールされていることを確認してください。  
まだインストールしていない場合は、[ウォレットのインストール](./install.md) ガイドを参照してください。

* [プロファイル](default:プロファイル) とは何かを理解していることを確認してください。

* 復元したいプロファイルの [ニーモニックフレーズ](default:ニーモニックフレーズ) が必要になります。  
プロファイルを最初に作成した際に使用したウォレットのドキュメントを確認してください。  
プロファイルが Symbol Desktop Wallet で作成されたものである場合は、手順について [プロファイルのエクスポート](./export-profile.md) チュートリアルを参照してください。

## 既存のプロファイルのインポート方法

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("screenshots/create-profile-0.jpg") }}
Symbol Desktop Wallet を開き、 **Create a new profile?** （新しいプロファイルを作成しますか？）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-1.jpg") }}
**Import Profile** （プロファイルのインポート）を選択します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-2.jpg") }}
プロファイルの詳細を入力します。

プロファイルに名前を付けます。  
これは、複数のプロファイルを整理するための単なる参照用です。

ネットワークタイプ（通常は `Mainnet` ）を選択します。

パスワードを入力して確認します。

オプションで、パスワードのヒントを追加します。

**Next** （次へ）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-3.jpg") }}
復元したいプロファイルのニーモニックフレーズを入力します。

これは、プロファイルを最初に作成したときに受け取ったシークレットフレーズです。  
単語が正しい順序であり、スペースで区切られていることを確認してください。  
フレーズが有効になるまで、 **Next** （次へ）ボタンは無効のままです。

**Next** （次へ）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-4.jpg") }}
復元するアカウントを選択します。

ウォレットはニーモニックをスキャンして関連するアカウントを検索し、プロファイルに含めるアカウントを選択できるようにします。

これらのアカウントの中には、一度も使用されたことがないものもあるかもしれません。
どのアカウントを復元するかを決定するのに役立つように、現在のXYM残高が表示されます。

ここで選択しなかったアカウントは、後から追加することができます。  
[アカウントの作成](./create-account.md) チュートリアルを参照してください。

アカウントの選択が完了したら、 **Next** （次へ）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("screenshots/import-profile-5.jpg") }}
安全に関するヒントを読み、利用規約に同意して、 **Finish** （完了）をクリックします。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

ウォレットのメイン画面に移動します。

![プロファイルの作成成功](screenshots/create-profile-8.jpg)

## 次のステップ

これで、インポートしたプロファイルを使用する準備が整いました。

* プロファイルにさらにアカウントを追加する方法については、[アカウントの作成](./create-account.md) を参照してください。
