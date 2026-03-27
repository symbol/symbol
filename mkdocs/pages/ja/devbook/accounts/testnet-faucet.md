---
title: 蛇口 (Faucet) からの資金調達
---

# 蛇口 (Faucet) からのテストネット資金の入手

Symbol [テストネット] (default: テストネット) では、テスト目的で開発者の [アカウント] (default: アカウント) に [XYM] (default: XYM) を無料で配布する「蛇口 (Faucet)」を提供しています [cite: 4, 80]。このガイドでは、ウェブベースの Faucet を使用してテストネット資金を請求する方法を説明します。

!!! note "メモ"
    テストネットの XYM には実質的な価値はありません [cite: 7, 22]。これは、実際の通貨を使用せずに Symbol の機能を試せるようにするために存在しています [cite: 7, 22]。

    メインネットの XYM が必要な場合は、[取引所](https://coinmarketcap.com/currencies/symbol/#Markets) を通じて購入する必要があります [cite: 4]。

## 前提条件

開始する前に、以下を確認してください。

- 資金を受け取るためのテストネット [アカウント] (default: アカウント) を、[コードから](../accounts/create-from-private-key.md) または [ウォレットを使用して](../../userbook/wallet/create-account.md) 作成していること [cite: 4, 75]。
- Faucet で本人確認を行うための 𝕏 アカウントを持っていること。

## テストネット資金を請求する方法

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
**Your Testnet Address** フィールドに、資金を受け取りたい [アドレス] (default: アドレス) を入力します [cite: 4]。

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
右上の通知にある **View in Explorer** をクリックして、トランザクションが処理されたことを確認します [cite: 4, 78]。

エクスプローラーには、承認状態を含むトランザクションの詳細が表示されます。通常のネットワーク条件下では、トランザクションは1分以内に承認されるはずです。

[ウォレット] (default: ウォレット) を設定している場合は、そこから転送を監視することもできます [cite: 4, 75]。
{{ tutorial.step_end() }}

{{ tutorial.list_end() }}

## Faucet への資金の返却

テストが終了したら、未使用の XYM を Faucet に返却することを検討してください。Faucet のアドレスは、資金を送ってきたアドレスと同じです。

送信元アドレスは、ブロックチェーンエクスプローラーでトランザクションを確認するか、アカウントのトランザクション履歴を検索することで見つけることができます。

さらに良い方法として、テスト用の [トランザクション] (default: トランザクション) の送信先として Faucet アドレスを使用してみてください [cite: 4, 78]。これにより、他の開発者のために Faucet の在庫を維持しながら、トランザクション送信の練習をすることができます。

## 次のステップ

[転送トランザクションの送信](../transactions/transfer.md) を試してみませんか？