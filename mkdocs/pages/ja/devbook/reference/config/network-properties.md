# ネットワークプロパティ

Symbolには、カスタマイズ可能なネットワーク関連の設定が多数あります。
これらの設定は、`.properties` ファイルを通じて直接[Catapult](default:Catapult)クライアントに提供できます。

ただし、設定を変更する最も簡単な方法は、[Shoestring](default: Shoestring)と`overrides.ini`ファイルを使用することです。

以下の各テーブルのヘッダーは、そのテーブルのプロパティが含まれるファイルを示しています。

!!! warning "注意"

    いずれかの設定プロパティをネットワークの他のノードと異なる値に設定すると、ノードが[フォーク](default:フォーク)し、事実上ネットワークから切断されてしまいます。

    自ノードにのみ影響するため、[ノードプロパティ](./node-properties.md)のみが安全に編集可能です。

## ネットワーク設定 {: #network-configuration }

<div class="md-typeset__scrollwrap">
<div class="md-typeset__table">
--8<-- 'devbook/reference/config/config-network.properties.html'
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
