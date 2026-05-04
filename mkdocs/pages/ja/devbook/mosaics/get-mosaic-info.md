---
title: モザイク情報の取得
tutorial_level: beginner
---

# モザイク情報の取得 {: #getting-mosaic-information }

Symbol上のすべての[モザイク](default:モザイク)には、供給量、可分性、振る舞いのフラグといったオンチェーンプロパティのセットがあります。

このチュートリアルでは、モザイクのプロパティと、それにリンクされている[ネームスペース](default:ネームスペース)のエイリアスを取得する方法を説明します。

## 前提条件 {: #prerequisites }

このチュートリアルはネットワークからデータを読み取るだけです。[アカウント](default:アカウント)は必要ありません。

開始する前に、[開発環境のセットアップ](../start/setup.md)を済ませておいてください。

## 完全なコード {: #full-code }

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/mosaics/get-mosaic-info', ['py', 'js']) }}

このスニペットでは、 `NODE_URL` 環境変数を使用してSymbol APIノードを設定します。
値が指定されない場合は、デフォルトの[テストネット](default:テストネット)ノードが使用されます。

`MOSAIC_ID` 環境変数は、照会するモザイクを指定します。
設定されていない場合は、テストネット上の<XYM:>の[モザイク ID](default:モザイクID)である `72C0212E67A08BCE` がデフォルトで設定されます。

## コード解説 {: #code-explanation }

### モザイク情報の取得 {: #fetching-mosaic-information }

{{ tutorial.code_snippet(['py:14:29', 'js:11:32']) }}

<get:/mosaics/{mosaicId}> エンドポイントは、以下のモザイクの現在のプロパティを取得します。

* **Supply (供給量):** 現在流通している[絶対（アトミック）](../../textbook/mosaics.md#divisibility)単位の総数。
    [初期供給量](../../textbook/mosaics.md#initial-supply)と混同しないでください。
* **Divisibility (可分性):** モザイクがサポートする[小数点以下の桁数](default:可分性)。
    例えば、XYMの可分性は `6` であり、1 XYMは1,000,000絶対単位に等しいことを意味します。
* **Flags (フラグ):** モザイクの振る舞いの制限をエンコードしたビットマスク。
    各フラグは単一のビットを占有します：
    [`supply_mutable`（供給量変更可能）](../../textbook/mosaics.md#supply-mutability) (1)、
    [`transferable`（譲渡可能）](../../textbook/mosaics.md#transferability) (2)、
    [`restrictable`（制限可能）](../../textbook/mosaics.md#restrictability) (4)、
    および [`revokable`（取り消し可能）](../../textbook/mosaics.md#revocability) (8)。
    複数のフラグは加算的に組み合わされます。例えば、値が `6` の場合は `transferable` (2) + `restrictable` (4) を意味します。
* **Duration (有効期限):** モザイクが有効なままである[ブロック数](../../textbook/mosaics.md#duration)。
    値が `0` の場合は、モザイクの有効期限が決して切れないことを意味します。
* **Start height (開始ブロック高):** モザイクが作成された[ブロック](default:ブロック)の高さ。
* **Revision (リビジョン):** モザイク定義が変更されるたびに増加します。

### 供給量のフォーマット {: #formatting-the-supply }

{{ tutorial.code_snippet(['py:31:36', 'js:34:41']) }}

APIによって返される供給量の値は、絶対量単位で表されます。
これを全体の単位に変換するために、コードはモザイクの可分性を使用して、供給量を整数部分と小数部分に分割します。

XYM（可分性 `6` ）の場合、 `8325447775994408` 絶対単位の供給量は `8325447775.994408` 全体単位に等しくなります。

### ネームスペースエイリアスの取得 {: #fetching-namespace-aliases }

{{ tutorial.code_snippet(['py:38:53', 'js:43:58']) }}

モザイクは、人間が読めるネームスペースエイリアスにリンクさせることができます。
<post:/namespaces/mosaic/names> エンドポイントはモザイクIDを受け取り、現在それらにリンクされているネームスペース名を返します。

異なるネームスペースが同じモザイクにリンクしている場合、モザイクは複数のネームスペースエイリアスを持つことができます。
ネームスペースがリンクされていない場合、レスポンスはエイリアスが存在しないことを示します。

## 出力 {: #output }

以下に示す出力は、テストネット上のXYMモザイクを照会する、プログラムの典型的な実行結果に対応しています。

```text linenums="1" hl_lines="5 6 7 8 9 10 11 13 16"
--8<-- 'devbook/mosaics/get-mosaic-info.log'
```

出力の主なポイント:

* Mosaic ID (5行目): テストネット上のXYMモザイク識別子（ 72C0212E67A08BCE ）。
* Supply (6行目): 絶対単位での総供給量。
* Divisibility (7行目): 値 6 は、1 XYM = 1,000,000 (10^6^) 絶対単位であることを意味します。
* Flags (8行目): 値 2 は transferable（譲渡可能）に解決され、XYMがアカウント間で自由に送信できることを意味します。
* Duration (9行目): 値 0 は、XYMの有効期限が決して切れないことを意味します。
* Supply in whole units (13行目): モザイクの可分性を使用して、絶対単位から全体単位に変換された供給量。
* Namespace alias (16行目): モザイクは symbol.xym ネームスペースにリンクされています。

## 結論 {: #conclusion }

このチュートリアルでは、以下の方法を説明しました。

| ステップ                                           | 関連ドキュメント                |
|------------------------------------------------|---------------------------|
| [モザイクプロパティの取得](#fetching-mosaic-information) | <get:/mosaics/{mosaicId}> |
| [ネームスペースエイリアスの取得](#fetching-namespace-aliases) | <dy:post:/namespaces/mosaic/names>

## 次のステップ {: #next-step }

* [モザイク定義の証明](../chain/prove-mosaic-definition.md) を行い、データがチェーンに記録されているものと一致することを検証する
* [アカウント残高の照会](../accounts/query-balance.md) を行い、アカウントがモザイクをどれだけ保持しているかを確認する
