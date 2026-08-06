---
title: ハーベスティングの有効化
---

# ハーベスティングの有効化

このページでは、Symbol モバイルウォレットアプリから [ハーベスティング](default:ハーベスティング) を有効にする方法を説明します。

ハーベスティングは、[ノード](default:ノード) が新しいブロックを作成し、参加アカウントに報酬を配分する仕組みです。
[委任ハーベスティング](default:委任ハーベスティング) では、自分が所有していないノードにハーベスティング作業を委任することで、アカウントが参加できます。
資金は自分のアカウントに残り、ノードがあなたの代わりにハーベスティングを行い、報酬の一部を受け取ります。

Symbol モバイルウォレットがサポートしているのは委任ハーベスティングのみです。
他の種類のハーベスティングを行うには、自分でノードを運用する必要があります。
詳しい説明は [ハーベスティング](../../textbook/harvesting.md) ページを参照してください。
自分でノードを運用したい場合は、[Shoestring を使用したノード運用](../shoestring/overview.md) ガイドを参照してください。

!!! info "ノードの選び方"
    開始する前に、ハーベスティングを委任するノードを探します。
    [symbol.fyi/nodes](https://symbol.fyi/nodes) を開き、ノードの **Public Key** をクリックして詳細を開き、
    **API ENDPOINT** をコピーします。

    ノードが正常に動作している限り、どのノードを選ぶかはアカウントには関係ありません。
    ノードエクスプローラーの詳細画面で、すべての **API NODE STATUS** チェックを通過しているノードを選んでください。

## 前提条件

* Symbol モバイルウォレットがインストールされていることを確認してください。
    まだインストールしていない場合は、[アプリのインストール](./install.md) ガイドを参照してください。

* すでにウォレットが設定され、アプリでロック解除されている必要があります。
    必要に応じて、[ウォレットの作成](./create-wallet.md) または [ウォレットのインポート](./import-wallet.md) を参照してください。

* アカウントには少なくとも 10,000 <XYM:> が必要です。

* ハーベスティングを有効にするためのトランザクション手数料を支払うのに十分な <XYM:> が必要です。

* 選択したノードの API ENDPOINT が必要です。

## ハーベスティングの有効化方法

委任ハーベスティングを有効にするには、次の手順に従ってください。

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("harvesting-0.webp") }}
ウォレットの **:material-home: HOME** 画面で、選択中のアカウントに少なくとも 10,000 <XYM:> があることを確認します。

右下隅の **ACTIONS** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-1.webp") }}
**Harvesting** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-4.webp") }}
Harvesting 画面には、選択中のアカウントが委任ハーベスティングを開始できるかどうかが表示されます。

**NODE URL** ボックスと **START** ボタンが表示されている場合、ハーベスティングを有効にできます。

ノードの **API ENDPOINT** を **NODE URL** ボックスに貼り付け、トランザクション手数料を選択して、
**START** をタップします。

この手数料は、この有効化トランザクションがどれだけ早く処理されるかにだけ影響します。
{{ tutorial.step_end() }}

<div markdown="block" class="tutorial-alt-grid">

!!! info "ハーベスティングが無効な場合"

    ハーベスティングはさまざまな理由で無効になることがあります。

    * アカウント残高が 10,000 <XYM:> 未満の場合は、続行する前に資金を追加してください。

        ![残高不足](harvesting-2.webp){ .off-glb }

    * アカウントに十分な残高があるにもかかわらず [インポータンス](default:インポータンス) がまだ低すぎる場合は、インポータンス計算が更新されるまで待ってください。
        インポータンスは残高変更より遅れて反映されます。
        再計算の遅延については、[インポータンス](../../textbook/accounts.md#importance) を参照してください。

        ![インポータンスがまだ低すぎる](harvesting-3.webp){ .off-glb }

</div>

{{ tutorial.step_begin("harvesting-5.webp") }}
確認メッセージを確認します。

問題がなければ、**CONFIRM** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-3.webp") }}
PIN コードを入力してウォレットのロックを解除し、トランザクションを承認します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-6.webp") }}
アプリがトランザクションを作成、署名、アナウンス、承認するまで待ちます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-7.webp") }}
トランザクションが承認されると、アプリに成功メッセージが表示されます。

**OK** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-8.webp") }}
Harvesting 画面に戻ります。

ノードが委任ハーベスティング要求を受け入れるまで待ちます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-9.webp") }}
要求が受け入れられると、アカウントの状態が **:material-checkbox-marked-circle-outline: Active** に変わります。

これで、アカウントはブロックをハーベストし、インポータンスに比例した報酬を得られるようになります。
インポータンスはおおむね残高に比例します。
詳細は [ハーベスティング](../../textbook/harvesting.md) を参照してください。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}
