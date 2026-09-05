---
title: トークンのブリッジ
---

# トークンのブリッジ

このページでは、Symbol モバイルウォレットアプリを使用して Symbol と Ethereum の間でトークンをブリッジする方法を説明します。

ブリッジを使用すると、あるブロックチェーンエコシステムから別のエコシステムへ価値を移動できます。
たとえば、`XYM` を Ethereum 上の `bXYM` としてブリッジし、より広い Ethereum エコシステムで使用できます。
ブリッジでは、<XYM:> を <ETH:> にスワップすることもできます。

ブリッジのワークフロー、手数料、処理時間、リスクの詳しい説明については、
[Symbol から Ethereum へのブリッジ](../../textbook/bridge.md) を参照してください。

!!! warning "ブリッジ要求には時間がかかり、手数料が発生します"

    ブリッジ要求は、送信元ネットワークで承認され、ブリッジに検出され、処理され、送信先ネットワークで支払われる必要があります。
    手数料は送信額から差し引かれ、最終的な受取額は変動する可能性があります。

## 前提条件

* Symbol モバイルウォレットがインストールされていることを確認してください。
    まだインストールしていない場合は、[アプリのインストール](./install.md) ガイドを参照してください。

* すでにウォレットが設定され、アプリでロック解除されている必要があります。
    必要に応じて、[ウォレットの作成](./create-wallet.md) または [ウォレットのインポート](./import-wallet.md) を参照してください。

* 送信元アカウントには、ブリッジする金額と手数料を支払うのに十分な残高が必要です。

## トークンのブリッジ方法

トークンをブリッジするには、次の手順に従ってください。

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin("portrait") }}

{{ tutorial.step_begin("harvesting-0.webp") }}
ウォレットの **:material-home: HOME** 画面で、右下の **ACTIONS** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-1.webp") }}
**Network Bridge** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-2.webp") }}
ウォレットに Ethereum アカウントがまだない場合、作成を促すポップアップが表示されます。
これは、初めてブリッジを使用するときに表示される場合があります。

**CONFIRM** をタップします。

すでに Ethereum アカウントがある場合は、手順 6 に進んでください。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-3.webp") }}
**ACTIVATE ACCOUNT** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-4.webp") }}
Ethereum アカウントが使用できるようになりました。

戻る矢印をタップして **ACTIONS** 画面に戻り、もう一度 **Network Bridge** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-5.webp") }}
**Swap Tokens** 画面では、何をブリッジまたはスワップするかを選択できます。

上のドロップダウンで送信元トークンを選択し、下のドロップダウンで送信先トークンを選択します。
利用可能な選択肢については、[Symbol から Ethereum へのブリッジ](../../textbook/bridge.md) を参照してください。

金額を入力します。
概要ボックスには次の情報が表示されます。

* **You Send**: 送信元アカウントから送信される金額。
* **Transaction Fee**: 送信元ネットワークで支払われる手数料の見積もり。
* **Bridge Fee**: ブリッジによって差し引かれる手数料の見積もり。
* **You Receive**: 送信先ネットワークで受け取る最終金額。

手数料は送信額から差し引かれ、最終的な値はブリッジが要求を処理する時点でのみ確定します。
さらに、ブリッジは [スリッページ](default:スリッページ) 保護を提供しません。

金額が手数料をまかなえないと見込まれる場合、**Amount too low** が表示され、**SEND** ボタンは無効になります。
Ethereum では手数料が大きくなる場合があります。

有効な金額を入力し、**SEND** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-6.webp") }}
確認画面を確認します。

たとえば、`XYM` を `bXYM` にブリッジする場合、**SIGNER ADDRESS** は送信元アカウントで、
**RECIPIENT ADDRESS** は Ethereum アカウントです。

ここに表示される **FEE** はトランザクション手数料のみで、ブリッジ手数料ではありません。

問題がなければ、**CONFIRM** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("export-wallet-3.webp") }}
PIN コードを入力してウォレットのロックを解除し、トランザクションを承認します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-7.webp") }}
アプリが要求を含むトランザクションを作成、署名、アナウンス、承認するまで待ちます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-8.webp") }}
トランザクションが承認されました。

**OK** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-9.webp") }}
要求はブリッジに送信され、保留中になりました。

下部のトランザクションボックスをタップして詳細を表示します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-10.webp") }}
**Swap Details** 画面には、ブリッジ要求の状態が表示されます。

この時点では、要求はブリッジによる処理待ちです。処理には、まずトランザクションがファイナライズされる必要があります。
この処理には約 20 分かかる場合があります。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-11.webp") }}
ブリッジが要求を検出し、処理しています。

これには、送信先ネットワーク上のトランザクション承認を待つ処理が含まれます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-12.webp") }}
トークンがウォレットで利用できるようになりました。
戻る矢印をタップして **ACTIONS** 画面に戻ります。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("harvesting-1.webp") }}
**External Account** をタップします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-4.webp") }}
外部アカウントに保存されている <ETH:> 以外のトークンを確認するには、アカウントボックスをタップして詳細を開きます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("bridge-13.webp") }}
**Bridge Account Details** 画面には、Ethereum アカウントが保有するトークンが表示されます。

この例では、受け取った `bXYM` を確認できます。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}
