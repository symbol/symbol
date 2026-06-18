# ノードの作成と実行

このチュートリアルでは、 [Shoestring](default:Shoestring) ツールを使用して新しいSymbol [ノード](default:ノード) をデプロイし、起動する方法を示します。

このチュートリアルでは、Shoestringとその依存関係がすでに[インストール済み](./install.md)であることを前提としています。

非推奨のBootstrapで作成されたノードがすでにあり、その設定や鍵を再利用したい場合は、このガイド全体に記載されているBootstrapに関する注意事項を確認してください。

## 新しいノードの作成 {: #create-a-new-node }

Shoestringを使用したノードのデプロイには、以下の手順が含まれます。

??? info "Shoestringウィザードの使用"

    Shoestringが提供する対話型のセットアップウィザードを使用することもできます。

    ```bash
    python3 -m shoestring.wizard
    ```

    ウィザードは質問に回答することで、このチュートリアルで作成されるのと同じ設定ファイルを生成します。

    ウィザードによって作成されたノードは、以下のコマンドラインの手順で作成されたノードと同じように動作します。

    ウィザードを完了した後、ノードが自動的に起動されなかった場合は、[ノードの起動](#start-the-node)から続行してください。

### 1. 作業ディレクトリの作成 {: #1-create-a-working-directory }

Shoestringは現在のディレクトリ内にノードファイルを作成します。

ノード用のディレクトリを作成し、そのディレクトリに移動します。

```bash
mkdir symbol-node
cd symbol-node
```

このチュートリアルのすべてのコマンドは、このディレクトリで実行することを前提としています。

### 2. ネットワーク設定の生成 {: #2-generate-the-network-configuration }

最初のステップは、ターゲットとするネットワーク（ [メインネット](default:メインネット) または [テストネット](default:テストネット) ）のデフォルト設定ファイルを取得することです。

以下を実行します。

```bash
python3 -m shoestring init --package mainnet config.ini
```

このコマンドは、 [メインネット](default:メインネット) で動作するために必要な設定ファイルをダウンロードし、 `config.ini` ファイルを作成します。

[テストネット](default:テストネット) の場合は、代わりに `sai` パッケージを使用します。
後続のコマンドで `--network` パラメータを使用する場合は、 `mainnet` の代わりに `testnet` を指定してください。

```bash
python3 -m shoestring init --package sai config.ini
```

### 3. Shoestringの設定 {: #3-configure-shoestring }

前の手順で生成された `config.ini` ファイルには、必要な設定のほとんどがすでに含まれています。
ご自身のノード用にカスタマイズする必要がある値は通常いくつかしかなく、それらはすべて `#!ini [node]` セクションにあります。

* ノードの[ロール](../../textbook/nodes.md#roles)。Shoestringでは _features（機能）_ と呼ばれます。

    リストから必要なノード機能を設定します： `PEER` 、 `API` 、 `HARVESTER` 、および `VOTER` 。
    縦棒 `|` で区切ることで、複数の機能を組み合わせることができます。例：

    ```ini title="config.ini"
    [node]
    features = PEER | API
    ```

    詳細な説明については、[ロール](../../textbook/nodes.md#roles)のページを確認してください。
    要約すると以下のようになります。

    * `PEER`: ノードはコンセンサスに参加します。
    * `API`: ノードはクライアントアプリケーション用のAPIを公開します。
    * `HARVESTER`: ノードはブロック生成に参加し、 [ハーベスティング](default:ハーベスティング) 報酬を得る資格があります。
        最低10,000 <XYM:> が必要です。
        以下の[ハーベスティングの設定（オプション）](#configure-harvesting-optional)を参照してください。
    * `VOTER`: ノードは [ファイナライズ](default:ファイナライズ) に参加します。
        最低3,000,000 <XYM:> が必要です。
        以下の[投票の設定（オプション）](#configure-voting-optional)を参照してください。

* `caCommonName` および `nodeCommonName` プロパティは、ノード用に生成される[証明書](https://en.wikipedia.org/wiki/X.509)の一般名（CN: Common Names）として使用されます。

    これらはノードの動作には影響しませんが、空であってはなりません。空の場合、証明書の生成に失敗します。

    例：

    ```ini title="config.ini"
    [node]
    caCommonName = My Symbol Node CA
    nodeCommonName = My Symbol Node
    ```

* HTTPSリクエストを受け入れる必要がある `API` ノードは、 `apiHttps` プロパティを `true` に設定する必要があります。
    そうしないと、ノードはHTTPリクエストしか受け入れません。

    ```ini title="config.ini"
    [node]
    apiHttps = false
    ```

    HTTPSサポートを有効にするには、次に示すように有効な `host` を設定する必要もあります。

### 4. ノードの設定 {: #4-configure-the-node }

ノード固有の設定を定義するには、追加の設定ファイルが必要です。
これがないと、ノードは正しく使用できません。

以下の内容で `overrides.ini` という名前の新しいファイルを作成します。

```ini title="overrides.ini"
[node.localnode]
host = my-symbol-node.com
friendlyName = My Symbol Node
```

* `host` は、ネットワーク上の他のノードが自身のノードにアクセスするためのパブリックIPアドレスまたはホスト名です。

    到達不可能なホスト（ `localhost` や `127.0.0.1` など）を使用した場合、ノードがネットワークに接続できても、他のノードは接続をできません。

* `friendlyName` は、 <https://symbol.fyi/nodes> などのノードリストに表示される、人間が読める名前です。

!!! info "さらなるカスタマイズ"

    `overrides.ini` ファイルを使用して、任意のCatapultの[ネットワークプロパティ](../../devbook/reference/config/network-properties.md)または[ノードプロパティ](../../devbook/reference/config/node-properties.md)を変更できます。

    これを行うには、 `[設定ファイル名.設定セクション]` という構文を使用してセクションを追加し、その下に目的のプロパティを定義します。

    ネットワークプロパティのいずれかをネットワークの他のノードと異なる値に設定すると、ノードが [フォーク](default:フォーク) し、ネットワークから切り離されることに注意してください。

`config.ini` はShoestring自体を設定するのに対し、 `overrides.ini` は生成されるノードソフトウェアをカスタマイズします。

### 5. ノードIDの作成 {: #5-create-the-node-identity }

各ノードには、他のノードとの安全な通信のための暗号化IDが必要です。
このIDの中心となるのが [秘密鍵](default:秘密鍵) であり、これはノードの [メインキー](default:メインキー) に対応します。

この鍵は[PEMファイル](https://en.wikipedia.org/wiki/Privacy-Enhanced_Mail)に保存されており、これを取得するには2つの方法があります。

* OpenSSLを使用して新しい鍵を作成する：

    ```bash
    openssl genpkey -algorithm ed25519 -out ca.key.pem
    ```

* 既存の秘密鍵をインポートする：

    ```bash
    python3 -m shoestring pemtool --output ca.key.pem
    ```

    このコマンドは秘密鍵を貼り付けるよう求めますが、 `--input` パラメータを使用してファイル経由で提供することもできます。
    秘密鍵をファイルやコマンド履歴に露出させたままにしないよう、特に注意してください。

    !!! tip "Bootstrapのメインキーを再利用する"

        既存のBootstrapノードで使用しているのと同じメインアカウントを再利用したい場合、メインの秘密鍵はノードのBootstrap内の `target/addresses.yml` ファイルから取得できます。

        ファイルが暗号化されている場合は、まず以下のコマンドを実行します。

        ```sh
        symbol-bootstrap decrypt \
            --source target/addresses.yml \
            --destination target/addresses_plaintext.yml
        ```

        次に、 `target/addresses_plaintext.yml` ファイルから鍵を取得し、**鍵を抽出した後はプレーンテキストファイルを削除します**。

        秘密鍵を取得したら、上記のように `pemtool` コマンドを使用します。

どちらのオプションでも、秘密鍵を含む `ca.key.pem` という名前のPEMファイルが作成されます。

!!! warning "このファイルは安全に保管する必要があります"

今のところは、このディレクトリに残しておいてください。
後でオフラインに移動します。

!!! info "メインアカウントのアドレス"

    この秘密鍵に対応するアドレスと公開鍵を表示するには、Shoestringの `pemview` ツールを使用できます。

    ```bash
    python3 -m shoestring pemview --input ca.key.pem --network mainnet
    ```

    出力は以下のようになります。

    ```text linenums="1"
        i     | loaded ca.key.pem
        i     |     address: TCUCHCSSTTXRU3BMOBMAQXY2WMGSBICEH6WB77I
        i     |  public key: 5DA31F82685B4F03924B096A1343F297CF4552F...
    ```

    アカウントへ資金を送信するには、このアカウントのアドレスが必要です。
    その後、ハーベスティングを有効にする際などにトランザクション手数料を支払うために資金が必要になります。

    `--show-private` パラメータを使用して秘密鍵を表示し、[ウォレットにインポート](../wallet/import-account.md)することもできます。
    この手順により、アカウントをより便利に管理できるようになりますが、このガイドでは必須ではありません。

    あるいは、 `openssl` ツールを直接使用してアカウントの公開鍵を表示することもできます。

    ```bash
    openssl pkey -in ca.key.pem -text
    ```

!!! tip "残りのBootstrapキーをインポートする"

    [VRFキー](default:VRFキー)、 [リモートキー](default:リモートキー) 、または [投票キー](default:投票キー) など、以前のノードと同じ鍵を引き続き使用したい場合は、以下のコマンドで今すぐインポートしてください。

    ```sh
    python3 -m shoestring import-bootstrap \
        --config config.ini \
        --bootstrap [Bootstrapターゲットディレクトリへのパス] \
        --include-node-key
    ```

    `--bootstrap` パラメータは、通常、以前のBootstrap内の `target` ディレクトリ（例： `../bootstrap-node/target` ）を指すことに注意してください。

    このコマンドは、必要なすべての鍵を `bootstrap-import` ディレクトリにコピーし、それらを参照するように `config.ini` を更新します。

    ブロックデータはインポートされないことに注意してください。初回起動時に、ノードはブロックチェーン全体をダウンロードすることでネットワークと同期します。

    これ以降、以前のBootstrapは不要になります。

### 6. ノードのインストール環境を作成 {: #6-create-the-node-installation }

設定ファイルと鍵ファイルの準備ができたら、以下を使用してノードのインストール環境を作成します。

```bash
python3 -m shoestring setup \
  --config ./config.ini \
  --overrides ./overrides.ini \
  --package mainnet \
  --directory . \
  --ca-key-path ./ca.key.pem
```

このコマンドは以下のいくつかのタスクを実行します：

* ノードのディレクトリ構造を準備する。
* 秘密鍵から証明書を生成する。
* Dockerの設定ファイルを作成する。
* ハーベスティングが設定されている場合、有効にするためのトランザクションを作成する。

## ハーベスティングの設定（オプション） {: #configure-harvesting-optional }

`HARVESTER` ロールを有効にした場合は、追加の設定が必要です。有効にしていない場合は、このセクションをスキップします。

!!! tip "Bootstrapからハーベスティング設定を再利用する"

    上記のように、動作しているBootstrapからすべての鍵をインポートし、そこでハーベスティングがすでに設定されている場合も、このセクションをスキップします。

ハーベスティングを有効にするには、特別なトランザクションを通じて [VRFキー](default:VRFキー) をアカウントにリンクする必要があります。

セキュリティ上の理由から、メインアカウントも別のトランザクションを通じて [リモートキー](default:リモートキー) にリンクする必要があります。このリモートアカウントは新しく作成されたブロックに署名するためにノードによって使用されるため、ノード上に置く必要があり、そこでオンライン状態を維持するため、サーバーが侵害された場合に暴露される可能性があります。

しかし、リモートアカウント自体は資金を保持しません。すべてのハーベスティング報酬は、リンクされたメインアカウント（オフラインで保存したままにできる）に送られるためです。

Shoestringの `setup` コマンドは、すでにこれら両方のトランザクションを作成し、 `linking_transaction.dat` という名前のファイルで保存しています。以下のサブセクションでは、このファイル内のトランザクションに署名し、ネットワークにアナウンスする方法を説明します。

### 7. メインアカウントへの資金提供 {: #7-fund-the-main-account }

まず、これらのトランザクションをアナウンスするために必要な手数料を支払うため、ノードのメインアカウントはいくらかの <XYM:> を保持する必要があります。

上記の[ステップ5](#5-create-the-node-identity)で示されているようにアカウントのアドレスを取得し、別のアカウントからそこに資金を送ります。
通常は 1 <XYM:> で十分です。

### 8. トランザクションへの署名 {: #8-sign-the-transactions }

Shoestringの `signer` ツールは `linking_transaction.dat` を読み取り、 `ca.key.pem` （ノードのメインアカウント）に保存されている鍵でその中のトランザクションに署名します：

```bash
python3 -m shoestring signer \
    --config config.ini \
    --ca-key-path ca.key.pem \
    --save linking_transaction.dat
```

署名はトランザクションとともに `linking_transaction.dat` に書き戻されるため、新しいファイルは作成されません。

出力は以下のようになります：

```text linenums="1"
    i     | Aggregate transaction: ((signature: 00000000...,
            signer_public_key: 5DA31F82..., version: 0x3,
            network: NetworkType.TESTNET,
            type: TransactionType.AGGREGATE_COMPLETE,
            fee: 0x0000000000015180,
            deadline: 0x0000001946C3C12A, )
            transactions_hash: CCA5A2CE..., transactions: [],
            cosignatures: [], )
    i     | Inner transactions:
    i     |   ((signer_public_key: 5DA31F82..., version: 0x1,
                network: NetworkType.TESTNET,
                type: TransactionType.ACCOUNT_KEY_LINK, )
                linked_public_key: E44E749E...,
                link_action: LinkAction.LINK, )
    i     |   ((signer_public_key: 5DA31F82..., version: 0x1,
                network: NetworkType.TESTNET,
                type: TransactionType.VRF_KEY_LINK, )
                linked_public_key: 88307270...,
                link_action: LinkAction.LINK, )
    i     | Signed transaction TransactionType.AGGREGATE_COMPLETE
            with hash: BE1DA35E...
```

便宜上、2つのキーリンクトランザクションが1つの [アグリゲートトランザクション](default:アグリゲートトランザクション) 内にラップされていることに注意してください。

### 9. トランザクションのアナウンス {: #9-announce-the-transactions }

announce-transaction ツールを使用して、署名されたトランザクションをネットワークに送信します：

```bash
python3 -m shoestring announce-transaction \
    --config config.ini \
    --transaction linking_transaction.dat
```

このツールはネットワーク上の利用可能なノードに接続し、署名されたトランザクションを送信します。
出力は以下のようになります：

```text linenums="1"
    i     | connecting to https://401-sai-dual.symboltest.net:3001
    i     | preparing to announce transaction 5E59A0A0... of type
            TransactionType.AGGREGATE_COMPLETE
    i     | transaction was successfully sent to the network
```

### 10. 承認の待機 {: #10-wait-for-confirmation }

`announce-transaction` コマンドは承認を待たずに終了するため、結果を手動で確認する必要があります。

前のコマンドの出力から、トランザクションが送信されたノードの URL（1行目）と、トランザクションの ハッシュ（2行目）をメモしておきます。

次に、ブラウザでそのURLを開くか、`curl` を使用して問い合わせます：
`{NODE_URL}/transactionStatus/{FULL_TRANSACTION_HASH}`

例えば、上記の出力の場合、URLは以下のようになります：

[https://401-sai-dual.symboltest.net:3001/transactionStatus/5E59A0A0...](https://401-sai-dual.symboltest.net:3001/transactionStatus/5E59A0A009A885B8DA0A6E1CE18BAE26A83441414E595799F25DB94848CCDEEB)

!!! note "トランザクションが送信されたのと同じノードを確認する"

    ノードがトランザクションをネットワーク上の他のノードにアナウンスせずに拒否した場合、他のすべてのノードはこのURLに対して「transaction unknown（不明なトランザクション）」エラーで応答します。そのため、トランザクションのステータスは常に送信したのと同じノードで確認するようにしてください。

トランザクションが受け入れられた場合、ノードは以下のようなメッセージを返します：

```json
{
  "hash": "5E59A0A009A885B8DA0A6E1CE18BAE26A83441414E595799F25DB94848CCDEEB",
  "code": "Success",
  "deadline": "107120562182",
  "group": "confirmed"
}
```

一般的な結果は以下の通りです：

| 結果                       | 意味                                                                                                                                  |
|------------------------------|------------------------------------------------------------------------------------------------------------------------------------------|
| **Confirmed(承認済み)**                | トランザクションが承認されました。このガイドの続きに進んでください。                                                                                |
| **Unconfirmed(未承認)**              | トランザクションは有効ですが、まだブロックに含まれていません。最大30秒ほど待ってください。                                                       |
| **Unknown(不明)**                  | 別のノードに送信されたか、送信から時間が経過しすぎている、あるいは送信されていません。[ステップ9](#9-announce-the-transactions) からやり直してください。 |
| **Insufficient funds(資金不足)**       | [上記のステップ7](#7-fund-the-main-account) で説明したようにノードのメインアカウントに資金を提供し、再度アナウンスしてください。                              |
| **Signature Not Verifiable(署名検証不可)** | トランザクションがおそらく署名されていません。[ステップ8](#8-sign-the-transactions) からやり直してください。                                               |
| **Deadline expired(有効期限切れ)**         |.  トランザクションは `setup` 実行後6時間以内にアナウンスする必要があります。以下を参照してください |       

有効期限が切れた場合は、`--output-transaction-only` パラメータを使用して [ステップ6](#6-create-the-node-installation) を再度行い、新しい `linking_transaction.dat` を作成してください。その後、[ステップ9](#9-announce-the-transactions) から手順を再開してアナウンスを行います。

トランザクションが承認されると、ノードは[リモートキー](default:リモートキー)（セットアッププロセス中に自動的に作成された）  を使用してブロック生成に参加し、（`ca.key.pem` ファイルから取得した） [メインキー](default:メインキー) にハーベスティング報酬を蓄積する準備が整います。

!!! warning "インポータンス計算の遅延"

    メインアカウントの [インポータンス](default:インポータンス) が高いほど、ノードがブロックをハーベストし、報酬を受け取れる確率が高くなります。

    メインアカウントに資金を提供したばかりの場合、インポータンスの更新には [メインネット](default:メインネット) で12時間、 [テストネット](default:テストネット) で3時間かかります。

## 投票の設定（オプション） {: #configure-voting-optional }

[投票ノード](default:投票ノード) を作成するには、以下の要件を満たす必要があります。

[Shoestringの設定時](#3-configure-shoestring)に `VOTER` ロールを設定する。
ノードのメインアカウントに 3,000,000 <XYM:> 以上を保有している。

!!! info "投票キーの保守"

    セットアップ中、Shoestringはノード用の [投票キー](default:投票キー) を作成します。
    セキュリティ上の理由から、この鍵には最大有効期限は180日に設定されているため、定期的に更新する必要があります。

    ノードが起動して正常に動作し始めたら、[ノードの保守](./maintain.md#voting-key-renewal)ガイドを読み、更新プロセスについて確認してください。

## ノードの起動  {: #start-the-node }

ノードの設定が完了し、起動の準備が整いましたが、最後に、セキュリティに関する手順を行います：

!!! danger "秘密鍵をバックアップしてください"

    `ca.key.pem` ファイルにはメインアカウントの秘密鍵が含まれていますが、通常のノード運用には**必要ありません**。
    ノードが適切に設定された後は、このファイルをオフラインに移動する必要があります。

    `ca.key.pem` を安全なストレージ（理想的にはオフライン）にコピーし、**ノードを実行するマシンからは削除してください**。

Dockerを使用してノードコンテナを起動します：

```bash
docker compose up -d
```

`-d` オプションはコンテナをデタッチドモードで実行するため、ターミナルを閉じた後もノードの実行を継続できます。

!!! warning "Dockerの権限"

    以下のようなエラーが表示される場合：

    ```text
    permission denied while trying to connect to the Docker daemon socket
    ```

    実行ユーザーにDockerのコマンドを実行する権限がありません。

    Linuxでは、以下のコマンドを使用してユーザーを `docker` グループに追加できます：

    ```bash
    sudo usermod -aG docker $USER
    ```

    その後、一度ログアウトして再ログインしてから、再度コマンドを実行してください。

ノードを停止するには、同じフォルダーから以下を実行します：

```bash
docker compose down
```

ノードのデータはディスク上に保持されるため、`docker compose up -d` で再度ノードを起動します。

ノードを正常にシャットダウンするまでに最大 5分 かかる場合があることに注意してください。
途中で中断された場合、データが不整合な状態になり、[リカバリ（復旧）](#node-recovery)が必要になる可能性があります。

## ノードの検証 {: #verify-the-node }

ノードが正しく動作しているか確認します：


```bash
python3 -m shoestring health --config config.ini --directory .
```

ノードが正常に起動した場合、このコマンドは（有効にした機能であると想定して）ピアエンドポイント、REST API、およびWebSocketサービスがアクセス可能であることを報告します：

```text
     ...    | running health agent for peer certificate
      i     | ca certificate not near expiry (7299 day(s))
      i     | node certificate not near expiry (374 day(s))
keys/cert/ca.crt.pem: OK
keys/cert/node.crt.pem: OK
     ...    | running health agent for peer API
      i     | peer API accessible, height = 226081
     ...    | running health agent for REST API
      i     | REST API accessible, height = 226081
     ...    | running health agent for REST websockets
      i     | websocket connected to ws://127.0.0.1:3000/ws,
              subscribing and waiting for block
      i     | websocket received a block with height 226082
```

!!! info "同期"

    初回起動時、ノードはブロックチェーン全体をダウンロードすることでネットワークと同期します。

    このプロセスには24時間から48時間かかる場合があります。
    この間、APIロールを実行している場合、ノードへのRESTリクエストが遅延したり、正しくないチェーン高を返す可能性があります。

## ノードのリカバリ（復旧） {: #node-recovery }

停電などによる不適切なシャットダウンが発生した場合、`data` フォルダーにロックファイルが残り、ノードの起動が妨げられることがあります。

以下のコマンドは、ノードのリカバリを試み、正常な状態に復元します：

```bash
docker compose -f docker-compose-recovery.yaml up \
    --abort-on-container-exit
```

リカバリコマンドが完了したら、`data` ディレクトリ内の `.lock` ファイルが消えているか確認してください。
削除されていれば、`docker compose up -d` でノードを再度起動します。

!!! warning "最終手段"

    リカバリコマンドで問題が解決しない場合は、`reset-data` を使用できます。

    このコマンドはダウンロード済みのブロックチェーンデータをすべて削除するため、ノードは最初から同期し直す必要があります。
    ただし、ノードの設定や鍵は保持されます。

    ```bash
    python3 -m shoestring reset-data \
        --config ./config.ini \
        --directory .
    ```

## 次のステップ {: #next-steps }

ノードが稼働した後は、あとは時折[メンテナンス作業](./maintain.md)を行うだけです。
