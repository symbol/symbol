---
title: 秘密鍵からの作成
tutorial_level: beginner
---

# 秘密鍵からのアカウント作成 {: #creating-accounts-from-private-keys }

このチュートリアルでは、既存の [秘密鍵](default:秘密鍵) を使用するか、新しいランダムなアカウントを生成して、Symbolブロックチェーンの [アカウント](default:アカウント) を作成する方法を説明します。

## 前提条件 {: #prerequisites }

開発環境のセットアップがまだ完了していない場合は、[開発環境のセットアップ](../start/setup.md) から始めてください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/accounts/create-from-private-key', ['py', 'js']) }}

## コード解説 {: #code-explanation }

### ファサードの初期化 {: #initializing-the-facade }

{{ tutorial.code_snippet(['py:5:6', 'js:5:6']) }}

<dy:SymbolFacade> は、Symbolの暗号化処理とネットワークユーティリティへのアクセスを提供します。
[アドレス](default:アドレス) などのネットワーク固有の値が正しく生成されるように、ネットワーク名（`testnet` または `mainnet`）を指定して初期化します。

### 秘密鍵の定義 {: #defining-a-private-key }

{{ tutorial.code_snippet(['py:8:16', 'js:8:17']) }}

この例では、まず環境変数 `PRIVATE_KEY` から16進数文字列として秘密鍵を取得することから始めます。
変数が設定されている場合、その値は <dy:PrivateKey> オブジェクトに変換されます。
設定されていない場合は、代わりに <dy:PrivateKey.random> を使用して新しいランダムな秘密鍵が生成されます。

!!! warning "秘密鍵を安全に保管してください"
    秘密鍵は、アカウントとそのアカウントが保持するすべての資産を完全に制御する権限を与えます。
    秘密鍵を紛失すると、アカウントへのアクセス権を恒久的に失うことになります。
    他の誰かが秘密鍵を入手すると、その人がアカウントを制御できてしまいます。

    秘密鍵を誰とも共有せず、常に安全な場所に保管してください。

### アカウントの作成 {: #creating-the-account }

{{ tutorial.code_snippet(['py:18:30', 'js:19:32']) }}

秘密鍵を定義した後、 公開鍵とアドレスを導出してアカウントを作成します。

1. **キーペアの作成:** <dy:KeyPair> コンストラクタは秘密鍵を受け取り、数学的に対応する [公開鍵](default:公開鍵) を派生させます。
    秘密鍵は秘密にしておく必要がありますが、公開鍵は誰とでも安全に共有できます。

2. **アドレスの導出:** <dy:network.publicKeyToAddress> メソッドは、公開鍵を[アドレス](default:アドレス)に変換します。アドレスはアカウントを識別するための、より短く人間が読みやすいネットワーク固有の識別子です。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な実行結果に対応しています。

```text
--8<-- 'devbook/accounts/create-from-private-key.log'
```

環境変数を指定せずにプログラムを実行するたびに、異なるランダムなアカウントが生成されます。
秘密鍵が提供された場合は、常に同じ公開鍵とアドレスが導出されます。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                        | 関連ドキュメント                      |
|---------------------------------------------|---------------------------------|
| [秘密鍵の読み込む](#defining-a-private-key)    | <dy:PrivateKey>                 |
| [ランダムな秘密鍵の作成](#defining-a-private-key) | <dy:PrivateKey.random>          |
| [公開鍵の取得](#creating-the-account)        | <dy:KeyPair.publicKey>          |
| [アドレスの取得](#creating-the-account)          | <dy:network.publicKeyToAddress> |

## 次のステップ

アカウントを作成したら、以下のことができます。

- [蛇口 (Faucet) からテストネットの通貨を入手する](./testnet-faucet.md)
- [最初のトランザクションを送信する](../transactions/transfer.md)
