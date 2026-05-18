---
title: モザイク供給量の変更
tutorial_level: beginner
---

# モザイク供給量の変更 {: #changing-mosaic-supply }

`supply_mutable`（供給量可変）フラグを有効にして作成された [モザイク](default: モザイク) は、作成後に総供給量を増加または減少させることができます。

このチュートリアルでは、モザイクの供給量を変更する方法を説明します。

## 前提条件 {: #prerequisites }

開始する前に、以下を準備してください。

* `supply_mutable` フラグが設定されたモザイクを所有する [アカウント](default: アカウント)。
    [モザイクの作成](./create-mosaic.md) チュートリアルを参照してください。
* トランザクション手数料を支払うための [XYM](default: XYM)。
    [蛇口 (Faucet) からテストネットの通貨を入手する](../accounts/testnet-faucet.md) を参照してください。

詳細については、テキストブックの [供給量可変](../../textbook/mosaics.md#supply-mutability) を参照してください。

## 供給量の増加（ミント） {: #increasing-supply-(minting) }

新しいユニットをミント（鋳造）するには、モザイクの作成チュートリアルの [モザイク供給量変更トランザクションの構築](./create-mosaic.md#building-the-mosaic-supply-change-transaction) ステップを以下のパラメータで再利用します。

1. `action` を `increase`（増加）に設定します。
2. `delta`（差分）に追加する絶対単位の数を設定します。
    差分は絶対単位で表されるため、モザイクの [可分性](../../textbook/mosaics.md#divisibility) によって、整数単位への換算が決まることに注意してください。

新しいユニットは、モザイク作成者のアカウント残高に追加されます。

## 供給量の減少（バーン） {: #decreasing-supply-(burning) }

既存のユニットをバーン（焼却）するには、同じ <ser:MosaicSupplyChangeTransactionV1> タイプを以下のパラメータで使用します。

1. `action` を `decrease`（減少）に設定します。
2. `delta` に削除する絶対単位の数を設定します。
    ミントの場合と同様に、差分はモザイクの [可分性](../../textbook/mosaics.md#divisibility) に基づいた絶対単位で表されます。

ユニットは、モザイク作成者のアカウント残高から削除されます。
作成者が十分なユニットを保持していない場合、トランザクションは検証エラーで失敗します。
