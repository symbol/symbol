---
title: 蛇口 (Faucet) からの通貨の入手
---

# 蛇口 (Faucet) からのテストネット通貨の入手 {: #getting-testnet-funds-from-the-faucet }

Symbol [テストネット](default:テストネット) では、テスト目的で開発者の [アカウント](default:アカウント) に <XYM:> を無料で配布する「蛇口 (Faucet)」を提供しています。このガイドでは、ウェブベースの Faucet を使用してテストネットの通貨を請求する方法を説明します。

!!! note "メモ"
    テストネットの XYM には実質的な価値はありません。これは、実際の通貨を使用せずに Symbol の機能を試せるようにするために存在しています。

    [メインネット](default:メインネット) の XYM が必要な場合は、[取引所](https://coinmarketcap.com/currencies/symbol/#Markets) を通じて購入する必要があります。

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

- 資金を受け取るためのテストネット [アカウント](default:アカウント) を、[コード](../accounts/create-from-private-key.md) または [ウォレット](../../userbook/wallet/create-account.md) を使用して作成していること。
- Faucet で本人確認を行うための 𝕏 アカウントを持っていること。

## テストネット資金を請求する方法 {: #how-to-claim-testnet-funds }

{% import 'tutorial.jinja2' as tutorial %}

{{ tutorial.list_begin() }}

{{ tutorial.step_begin("../../images/faucet-open.png") }}
ウェブブラウザを開き、Symbol テストネット Faucet（[testnet.symbol.tools](https://testnet.symbol.tools)）にアクセスします。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-sign-in.png") }}
**Sign in with Twitter**（現在の 𝕏）をクリックし、認証フローに従います。

このステップは、Faucet の悪用を防ぐために、1アカウントあたりのテスト資金を 10,000 XYM に制限するために設けられています。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-authorize.png") }}
サインイン後、𝕏 から Faucet アプリケーションがアカウント情報にアクセスすることを許可するか確認されます。

権限を確認し、**Authorize app** をクリックして続行します。承認されると、再び Faucet にリダイレクトされます。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-address.png") }}
**Your Testnet Address** フィールドに、資金を受け取りたいアドレスを入力します。

アドレスが `T` で始まっていることを確認してください。これはテストネットアカウントであることを意味します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-xym.png") }}
**XYM Amount** フィールドに、請求したい XYM の量を指定します。
1回のリクエストあたりの最大量は 10,000 XYM です。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-claim.png") }}
**Claim** をクリックしてリクエストを送信します。
リクエストが成功すると、Faucet は指定された量の XYM をあなたのアドレスに転送します。
{{ tutorial.step_end() }}

{{ tutorial.step_begin("../../images/faucet-view-explorer.png") }}
右上の通知にある **View in Explorer** をクリックして、トランザクションが処理されたことを確認します。

エクスプローラーには、承認状態を含むトランザクションの詳細が表示されます。通常のネットワーク条件下では、トランザクションは概ね1分以内に承認されます。

[ウォレット](default:ウォレット) を設定している場合は、そこから転送を監視することもできます。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

## Faucet への通貨の返却 {: #returning-funds-to-the-faucet }

テストが終了したら、未使用の XYM を Faucet に返却することを検討してください。Faucet のアドレスは、資金を送ってきたアドレスと同じです。

送信元アドレスは、ブロックチェーンエクスプローラーでトランザクションを確認するか、アカウントのトランザクション履歴を検索することで見つけることができます。

さらに良い方法として、テスト用のトランザクションの送信先として Faucet アドレスを使用してみてください。これにより、他の開発者のために Faucet の在庫を維持しながら、トランザクション送信の練習をすることができます。

## 次のステップ {: #next-steps }

[転送トランザクションの送信](../transactions/transfer.md) を試してみませんか？