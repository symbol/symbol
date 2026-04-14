---
title: アカウント制限の追加
---

# アカウントへの制限の追加 {: #adding-restrictions-to-an-account }

アカウントは、以下の項目に対して制限を課すことができます。

- インタラクション可能な他の [アカウント](default: アカウント)
- 取引可能な [モザイク](default: モザイク)
- 実行可能な操作（トランザクションタイプ）の種類

これらの制限は[アカウント制限](default: アカウント制限)を使用して設定されます。

このチュートリアルでは、アカウントの **送信トランザクション** を制限し、許可された単一のアドレスにのみトランザクションを送信できるようにする方法を実演します。

もし制限がすでに有効である場合は、代わりにその制限を解除する方法を実演します。

制限の有効化または無効化を行った後、未承認のアドレスに対してテスト用の転送トランザクションをアナウンスし、ネットワークがそれをどのように拒否するかを確認します。

!!! note "モザイク制限との違い"

    Symbolは、このチュートリアルで説明するアカウントレベルの制限とは別に、モザイクレベルで定義される [モザイク制限](default: モザイク制限) もサポートしています。

    これらは異なる仕組みです。異なるトランザクションタイプを使用して設定され、異なるルールに基づいて動作します。

    アカウント制限はアカウントがインタラクションできるモザイクを制限でき、モザイク制限はモザイクとインタラクションできるアカウントを制限できるため、概念的な重複が混乱の元となることがよくあります。

## 前提条件 {: #prerequisites }

開始する前に、以下を確認してください。

- 開発環境をセットアップしていること。
    [開発環境のセットアップ](../start/setup.md) を参照してください。
- 制限を課すための [アカウント](default: アカウント) を、[コード](./create-from-private-key.md) または [ウォレット](../../userbook/wallet/create-account.md) を使用して作成していること（あるいは提供されているデフォルトアカウントを使用してください）。
- トランザクション手数料を支払うための [XYM](default: XYM) を入手していること。
    [蛇口 (Faucet) からのテストネット通貨の入手](./testnet-faucet.md) を参照してください。

さらに、トランザクションがどのようにアナウンスされ承認されるかを理解するために、[転送トランザクション](../transactions/transfer.md) のチュートリアルを復習してください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/accounts/account-restrictions', ['py', 'js']) }}

## コード解説 {: #code-explanation }

コードは、2つのヘルパー関数の定義から始まります。
トランザクションのアナウンス方法や承認の追跡方法の詳細については、[転送トランザクション](../transactions/transfer.md) のチュートリアルを参照してください。その他のヘルパー関数については、以下のセクションで説明します。

その後、チュートリアルは以下の手順で進みます。

- [必要な鍵の設定](#setting-up-the-accounts)
- [現在のネットワーク状態の取得](#fetching-network-time-and-fees)
- [現在の制限状態の検出](#detecting-the-restriction-state)

アカウントがすでに制限されているかどうかに応じて、以下のいずれかのトランザクションが作成されます。

- [制限の有効化](#enabling-the-restriction)
- [制限の解除](#removing-the-restriction)

その後、トランザクションは [アナウンスおよび承認](#submitting-the-transaction) され、最後に [テスト転送](#sending-a-test-transfer) が送信されます。

### アカウントの設定 {: #setting-up-the-accounts }

{{ tutorial.code_snippet(['py:105:113', 'js:111:119']) }}

アカウントは自分自身に対してのみ制限を設定できるため、このチュートリアルでは単一の [秘密鍵](default: 秘密鍵) が必要です。
秘密鍵は `SIGNER_PRIVATE_KEY` 環境変数（64文字の16進数文字列）を通じて提供できます。提供されない場合は、デフォルト値が使用されます。

アカウントはトランザクションをアナウンスするのに十分な資金を保有している必要があります。デフォルトの鍵を使用する場合、対応するアカウントにはすでに資金が供給されている可能性があります。

この段階で、許可されたアドレスも設定されます。制限によって、後に送信トランザクションはこのアドレスのみに限定されます。

### ネットワーク時間と手数料の取得 {: #fetching-network-time-and-fees }

{{ tutorial.code_snippet(['py:116:134', 'js:122:140']) }}

[転送トランザクション](../transactions/transfer.md) チュートリアルで説明されているプロセスに従い、ネットワーク時間と推奨手数料をそれぞれ <get:/node/time> および <get:/network/fees/transaction> から取得します。

### 制限状態の検出 {: #detecting-the-restriction-state }

以下の関数は、<get:/restrictions/account/{address}> エンドポイントを使用して、指定されたアドレスに適用されている現在のアカウント制限を取得します。制限が設定されていない場合、関数は空のリストを返します。

{{ tutorial.code_snippet(['py:47:60', 'js:53:66']) }}

返されたリストを評価して、チュートリアルの実行パスを決定します。その内容に基づいて、制限を有効化するか解除するか、適切な設定トランザクションが構築されます。

{{ tutorial.code_snippet(['py:138:146', 'js:144:155']) }}

アカウントに複数の制限が設定されている場合、エンドポイントから返された最初の制限のみが削除されます。このチュートリアルの範囲内では、そのような状況は発生しないはずです。

### 制限の有効化 {: #enabling-the-restriction }

アカウントがインタラクションできるアドレスのリストを制限するには、<ser:AccountAddressRestrictionTransactionV1> を使用します。

このチュートリアルでは扱いませんが、他の2つのアカウント制限タイプは以下の通りです。

- <ser:AccountMosaicRestrictionTransactionV1>
- <ser:AccountOperationRestrictionTransactionV1>

{{ tutorial.code_snippet(['py:63:80', 'js:69:87']) }}

トランザクションには以下のフィールドが含まれます。

- `signer_public_key`: 制限設定を変更するアカウントの [公開鍵](default: 公開鍵)。

- `restriction_flags`: これらは <ser:AccountRestrictionFlags> です。

    - `ADDRESS` は、制限がアドレスに適用されることを指定します。他の可能なスコープは `MOSAIC_ID` と `TRANSACTION_TYPE` です。
    - `OUTGOING` は、送信トランザクションのみが影響を受けることを指定します。受信トランザクションの制限は、このフラグを除外することで独立して設定できます。

    デフォルトでは、リストされた値は「許可リスト（allowlist）」を形成します。指定されたアドレスのみが許可されます。

    リストされたアドレスを禁止する「拒否リスト（blocklist）」モードで制限を設定するには、`BLOCK` フラグを含めます。

    ネットワークはこれらのフラグを現在の値と XOR（排他的論理和）演算します。このチュートリアルでは、有効化する前に制限が存在しないことを確認しているため、この時点での現在の値は 0 です。

- `restriction_additions`: 制限に追加するアドレス（またはモザイクID、トランザクションタイプ）のリスト。

    この例では、リストには許可されたアドレスのみが含まれます。

### 制限の解除 {: #removing-the-restriction }

制限を無効にするには、設定されているフラグとリストされたアドレスの両方をクリアする必要があります。

{{ tutorial.code_snippet(['py:83:101', 'js:90:107']) }}

制限を有効にした時と同じ `restriction_flags` の値が再度提供されます。フラグはネットワークによって XOR されるため、同じ値を提供するとそれらがオフに切り替わり、実質的に制限がクリアされます。

現在制限に設定されているアドレスは `restriction_deletions` フィールドに指定され、設定から削除されます。

<dy:Address.fromDecodedAddressHexString> メソッドは、REST API から返される16進文字列形式を、トランザクション構築時に期待されるアドレス表現に変換します。

### トランザクションの送信 {: #submitting-the-transaction }

構築されたトランザクションは、[転送トランザクション](../transactions/transfer.md) チュートリアルで説明されている通り、署名、アナウンス、承認されます。

{{ tutorial.code_snippet(['py:148:154', 'js:157:164']) }}

### テスト転送の送信 {: #sending-a-test-transfer }

その後、未承認のアドレスに対してテスト用の転送が試行されます。

{{ tutorial.code_snippet(['py:156:170', 'js:166:180']) }}

制限が有効になっている場合、転送は `Address_Interaction_Prohibited` エラーで失敗します。制限が解除されている場合、転送は正常に承認されます。

制限設定トランザクションとテスト転送は独立してアナウンスされ、承認されます。それぞれに個別の承認が必要なため、全体の実行時間が長くなる可能性があります。

このプロセスは、両方のトランザクションを単一の [アグリゲートトランザクション](default: アグリゲートトランザクション) に組み込んで一緒にアナウンスすることで最適化できます。

## 出力 {: #output }

以下に示す出力は、プログラムの典型的な2つの実行結果に対応しています。

=== ":material-lock-plus: 制限の有効化"

    ```text linenums="1" hl_lines="2-3 9 21-24 41"
    --8<-- 'devbook/accounts/account-restrictions-enable.log'
    ```

    出力の主なポイント:

    - **2-3行目**: 関与するアカウントのアドレス。
    - **9行目** (`Response: No restrictions found`): 現在制限は設定されていません。
    - **21行目** (`"restriction_flags": 16385`): `0x4001` は `ADDRESS` と `OUTGOING` の組み合わせに対応します。
    - **22-24行目** (`"restriction_additions"`): デコードされた16進数形式の、許可されたアドレスのリスト。この値は3行目に示されているアドレスに対応します。
    - **41行目** (`test transfer failed`): 期待通り、未承認の受信者アドレスにより `Address_Interaction_Prohibited` エラーが発生しています。

=== ":material-lock-open: 制限の解除"

    ```text linenums="1" hl_lines="2-3 9 21 23-25 44"
    --8<-- 'devbook/accounts/account-restrictions-disable.log'
    ```

    出力の主なポイント:

    - **2-3行目**: 関与するアカウントのアドレス。
    - **9行目** (`Response: [ ... ]`): 既存の制限が検出されました。
    - **21行目** (`restriction_flags`): 制限を有効にした時と同じフラグ値。
    - **23-25行目** (`restriction_deletions`): 以前に設定されていたアドレスが削除されます。
    - **44行目** (`test transfer confirmed`): 制限が解除されたため、転送が正常に承認されました。

出力に示されているトランザクションハッシュを使用して、[Symbol Testnet Explorer](https://testnet.symbol.fyi/) でトランザクションを検索できます。

## 結論 {: # }

このチュートリアルでは、以下の方法を説明しました。

| ステップ | 関連ドキュメント |
|------------------------------------------------------------------------------------|----------------------------------------------|
| [現在の制限設定の取得](#detecting-the-restriction-state) | <get:/restrictions/account/{address}> |
| [制限の有効化](#enabling-the-restriction) | <ser:AccountAddressRestrictionTransactionV1> |
| [制限の解除](#removing-the-restriction) | <ser:AccountAddressRestrictionTransactionV1> |