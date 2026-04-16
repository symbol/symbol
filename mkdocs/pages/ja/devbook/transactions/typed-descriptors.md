---
title: 型付き記述子
---

# JavaScriptで型付き記述子を使用してトランザクションを作成する {: #creating-transactions-using-typed-descriptors-in-javascript }

ネットワークとのやり取りのほとんどはトランザクションを通じて行われるため、[トランザクション](default:トランザクション) はSymbolブロックチェーンの基本的な要素です。これにより、[XYM](default:XYM)やその他の[モザイク](default:モザイク)を、任意でメッセージを添えて、ある[アカウント](default:アカウント)から別のアカウントへ送信することができます。

チュートリアル全体のすべてのJavaScriptの例では、構文がコンパクトであるため、トランザクションの作成に <js:SymbolTransactionFactory.create> メソッドを使用しています。
ただし、このメソッドは型安全ではありません。汎用オブジェクトを受け入れ、それが正しいフィールドがあることに依存しています。

このページでは、代わりに <js:SymbolFacade.createTransactionFromTypedDescriptor> を使用する方法を説明します。
この代替手段は、明確に定義されたパラメータを受け入れ、より優れた型安全性とIDEサポートの向上を提供します。

ここで紹介するコードは、[転送トランザクションの作成](./transfer.md) チュートリアルと同じですが、トランザクション作成のステップのみが異なります。
簡潔にするため、ここではそのセクションのみを示します。
トランザクションの署名やアナウンスを含む、プロセスの残りの部分は変わりません。

=== "JavaScript"

    ```js linenums="41"
    --8<-- "devbook/transactions/transfer.typed.mjs:41:55"
    ```

    [完全なチュートリアルコードをダウンロードする]({{ config.repo_url }}/raw/refs/heads/{{config.extra.symbol.branch}}/mkdocs/overrides/devbook/transactions/transfer.typed.mjs){ .source-link }

## 作成プロセス {: #creation-process }

トランザクションは、トランザクション記述子の作成とトランザクション自体の作成という2つのステップで、型安全な方法で作成されます。

### 記述子の作成 {: #creating-the-descriptor }

=== "JavaScript"

    ```js linenums="41"
    --8<-- "devbook/transactions/transfer.typed.mjs:41:52"
    ```

構造化されたパラメータを持つコンストラクタにより、型付き記述子はJavaScriptでトランザクションを構築する際に型安全性を提供します。

例えば、コードで使用されている <js:TransferTransactionV1Descriptor> を参照してください。

そのような記述子が利用可能な場合、チュートリアルは常に関連するリファレンスページとこのガイドの両方にリンクします。

### トランザクションの作成 {: #creating-the-transaction }

=== "JavaScript"

    ```js linenums="53"
    --8<-- "devbook/transactions/transfer.typed.mjs:53:55"
    ```

記述子の準備ができたら、トランザクションの作成は簡単です。記述子を <js:SymbolFacade.createTransactionFromTypedDescriptor> メソッドに渡し、希望する手数料と有効期限を提供するだけです。

[転送トランザクションの作成](./transfer.md#building-the-transaction) チュートリアルと同様に、トランザクションの手数料はトランザクションのサイズに依存するため、構築後に計算する必要があることに注意してください。

!!! warning "注意: 型付きバージョンと型なしバージョンでは、有効期限の指定方法が異なります"

    <js:SymbolTransactionFactory.create> に渡される有効期限はミリ秒単位で指定され、*ネットワーク時間* に相対的です。
    対照的に、 <js:SymbolFacade.createTransactionFromTypedDescriptor> に渡される有効期限は秒単位で指定され、*システム時間* （つまり、コードを実行しているマシンのローカルクロック）に相対的です。

    このアプローチは、現在のネットワーク時間を取得する必要がなくなるため便利です。例えば、トランザクションの有効期限を2時間にするには、上記のコードのように `#!js 2 * 60 * 60` 秒の有効期限を提供するだけです。

    ただし、システムクロックがネットワーク時間と適切に同期されていない場合、トランザクションが予想よりも早く期限切れになったり、提供された有効期限がネットワークの最大許容オフセットである2時間を超えた場合に完全に拒否されたりする可能性があります。

    **したがって、型安全なメソッドを使用するアプリケーションは、システムクロックが適切に同期されていることを確認するために、定期的にネットワーク時間を確認する必要があります。**

トランザクションが作成されたら、通常通りに使用できます。
型付きメソッドと型なしメソッドを使用して作成されたトランザクションの間に違いはありません。
