# ノードプロパティ

Symbolには、カスタマイズ可能なノード関連の設定が多数あります。

これらの設定は、`.properties`ファイルを通じて直接[Catapult](default:Catapult)クライアントに提供できます。

設定を変更する最も簡単な方法は、[Shoestring](default:Shoestring)と`overrides.ini`ファイルを使用することです。

以下の各テーブルのヘッダーは、そのテーブルのプロパティが含まれるファイルを示しています。

!!! note "メモ"
    **ネットワーク**関連のプロパティについては、[ネットワークプロパティ](./network-properties.md)のページを参照してください。

## ユーザー設定 {: #user-configuration }

<div class="md-typeset__scrollwrap">
<div class="md-typeset__table">
--8<-- 'devbook/reference/config/config_user.properties.html'
</div>
</div>

## ノード設定 {: #node-configuration }

<div class="md-typeset__scrollwrap">
<div class="md-typeset__table">
--8<-- 'devbook/reference/config/config_node.properties.html'
</div>
</div>

## ハーベスティング設定 {: #harvesting-configuration }
 
<div class="md-typeset__scrollwrap">
<div class="md-typeset__table">
--8<-- 'devbook/reference/config/config_harvesting.properties.html'
</div>
</div>

<style>
    /* Hide irrelevant column */
    .md-typeset table th:nth-child(3),
    .md-typeset table td:nth-child(3){
        display: none;
    }

    .md-typeset .md-typeset__table table td {
        border: none;
        padding-bottom: 0.25rem;
        font-size: 0.8rem;
    }

    .md-typeset td:nth-child(4) {
        word-break:break-word;
    }
</style>
