---
title: ネームスペース情報の取得
---

# ネームスペース情報の取得 {: #getting-namespace-information }

このチュートリアルでは、 [ネームスペース](default:ネームスペース)のプロパティと、それが指し示す [モザイク](default:モザイク)または[アカウント](default:アカウント)を取得する方法を説明します。

## 前提条件 {: #prerequisites }

このチュートリアルはネットワークからデータを読み取るだけです。アカウントは必要ありません。

開始する前に、[開発環境のセットアップ](../start/setup.md)を済ませておいてください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/namespaces/get-namespace-info', ['py', 'js']) }}

このスニペットでは、 `NODE_URL` 環境変数を使用してSymbol APIノードを設定します。
値が指定されない場合は、デフォルトの [テストネット](default:テストネット) ノードが使用されます。

`NAMESPACE_NAME` 環境変数は、照会するネームスペースを指定します。
設定されていない場合は、ネットワークのネイティブ通貨である <XYM:> にリンクされたネームスペース `symbol.xym` がデフォルトで設定されます。

## コード解説 {: #code-explanation }

### ネームスペースIDの生成 {: #generating-the-namespace-id }

{{ tutorial.code_snippet(['py:16:20', 'js:11:15']) }}

ネームスペースIDは、 <dy:IdGenerator.generateNamespacePath> を使用して、ネームスペース名からローカルで計算されます。
この関数は、 `symbol.xym` のような完全修飾名を受け取り、それを `.` で分割し、階層の各レベルのネームスペースIDの配列を返します。
最後の要素は、最も深いネームスペースのIDです。

### ネームスペース情報の取得 {: #fetching-namespace-information }

{{ tutorial.code_snippet(['py:22:44', 'js:17:46']) }}

<get:/namespaces/{namespaceId}> エンドポイントは、以下のネームスペースの現在のプロパティを取得します。

* **Registration type (登録タイプ):** 値 `0` は [ルートネームスペース](default:ルートネームスペース)を示し、 `1` は [サブネームスペース](default:サブネームスペース) を示します。

* **Owner address (所有者アドレス):** [ネームスペースを登録した](../../textbook/namespaces.md#ownership)アカウント。

* **Depth (深さ):** ネームスペース階層のレベル数。
    例えば、 `foo` の深さは `1` 、 `foo.bar` の深さは `2` 、 `foo.bar.baz` の深さは `3` です。

* **Levels (レベル):** 階層の各レベルのネームスペースID。
    `level0` は常にルートネームスペースIDです。より深い階層の場合は `level1` および `level2` が表示されます。

* **Start and end heights (開始および終了ブロック高):** [ネームスペースが有効](../../textbook/namespaces.md#duration)な [ブロック](default:ブロック) の範囲。

### エイリアスの確認 {: #checking-the-alias }

{{ tutorial.code_snippet(['py:46:58', 'js:48:59']) }}

ネームスペース階層の各レベルは、独自の[エイリアス](../../textbook/namespaces.md#linking)を持つことができる独立したネームスペースです。
レスポンスには、照会されたレベルのエイリアス情報が含まれており、モザイクまたはアカウントのどちらにリンクされているかを示します。

* **Alias type 0 (エイリアスタイプ 0):** エイリアスはリンクされていません。
* **Alias type 1 (エイリアスタイプ 1):** ネームスペースはモザイクにリンクされています。レスポンスにはリンクされた [モザイクID](default:モザイクID) が含まれます。
* **Alias type 2 (エイリアスタイプ 2):** ネームスペースはアカウントにリンクされています。レスポンスにはリンクされた [アドレス](default:アドレス) が含まれます。

## 出力 {: #output }

以下に示す出力は、テストネット上の `symbol.xym` ネームスペースを照会する、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="3 6 7 8 9 10 11 12 13 14"
--8<-- 'devbook/namespaces/get-namespace-info.log'
```

出力の主なポイント:

* **Namespace ID** (3行目): `symbol.xym` に対して計算されたIDは `0xe74b99ba41f4afee` です。

* **Registration type** (6行目): 値 `1` は、これがサブネームスペース（ `symbol` の子）であることを確認します。

* **Owner address** (7行目): `symbol` ネームスペース階層を登録したアカウント。

* **Depth** (8行目): 値 `2` は、2段階の階層（ `symbol` （レベル0）と `xym` （レベル1））であることを示します。

* **Level IDs** (9-10行目): `level0` はルートの `symbol` ネームスペースID（ `A95F1F8A96159516` ）であり、 `level1` は `xym` サブネームスペースID（ `E74B99BA41F4AFEE` ）です。
    最後のレベルIDは3行目のネームスペースIDと一致しており、照会されているネームスペースであることが確認できます。

* **End height** (12行目): 値 `18446744073709551615` （ `0xFFFFFFFFFFFFFFFF` ）は、このネームスペースの有効期限が決して切れないことを意味します。

* **Alias** (13-14行目): エイリアスタイプ `1` は、ネームスペースがXYMモザイク（ `72C0212E67A08BCE` ）にリンクされていることを確認します。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                                 | 関連ドキュメント                             |
|------------------------------------------------------|----------------------------------------|
| [ネームスペースIDの生成](#generating-the-namespace-id)       | <dy:IdGenerator.generateNamespacePath> |
| [ネームスペースプロパティの取得](#fetching-namespace-information) | <get:/namespaces/{namespaceId}>        |
| [ネームスペースエイリアスの確認](#checking-the-alias)             | <get:/namespaces/{namespaceId}>        |
