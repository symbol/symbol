# ノードの保守

## 監視 {: #monitoring }

定期的な監視は、ハーベスティング、投票、またはAPIの可用性に影響を与える前に問題を検出するのに役立ちます。
正常なノードは、オンラインを維持し、ネットワークと同期し、リクエストに応答する必要があります。

以下の項目を定期的に確認することで、予期せぬ停止を防ぐことができます。
ターミナルコマンドは、ノードのディレクトリから実行されることを前提としています。

* **REST ヘルスエンドポイントをリモートで確認する**

    REST Gatewayが有効になっている場合、ノードはヘルスエンドポイント <get:/node/health> を公開します。

    正常なレスポンスは以下のようになります。

    ```json
    {
        "status": {
            "apiNode": "up",
            "db": "up"
        }
    }
    ```

    いずれかのコンポーネントが `down` を返している場合、さらなる調査が必要です。

* **Shoestringでノードのヘルスを確認する**
{: #check-node-health-with-shoestring :}

    Shoestringには、組み込みのヘルスチェックコマンドが含まれています。

    ```bash
    python3 -m shoestring health --config config.ini --directory .
    ```

    このコマンドは、ローカルノードの基本的なステータスチェックを実行し、サービスが正常に稼働しているように見えるか、およびすべての必要な鍵が正しく登録され、[有効期限が近づいていないか](#key-and-certificate-renewals)を報告します。

    正常なノードのレポートは以下のようになります。

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

* **Dockerコンテナのステータスを確認する**

    ShoestringノードはDockerコンテナ内で実行されます。
    すべての必要なコンテナが実行されていることを確認するには、以下のコマンドを使用します。

    ```bash
    docker compose ps
    ```

    再起動ループに陥っている、または終了（exited）と表示されているコンテナは、通常、設定または起動時の問題を示しています。

* **ログの表示**
{: #view-logs :}

    ログは多くの場合、ノードの問題を診断する最も速い方法です。

    最近のログを表示するには：

    ```bash
    docker compose logs --tail=50
    ```

    ログをリアルタイムで追跡するには（`Ctrl+C`で停止）：

    ```bash
    docker compose logs -f
    ```

    オプションで、 `client` 、 `db` 、 `broker` など、調査したいサービスを指定できます。

    繰り返し発生するエラー、データベースの問題、接続の失敗、またはファイルの欠落に関する警告を探してください。

* **ネットワーク同期の確認**

    実行中のノードが、必ずしも同期されたノードであるとは限りません。
    ご自身のノードの現在のブロック高を、ブロックチェーンエクスプローラーやノードリストと比較してください。

    役立つリソースには以下のものがあります。

    * [Symbol Explorer](https://symbol.fyi/nodes)
    * [Nodewatch](https://nodewatch.symbol.tools/symbol/nodes)
    * [Symbol Node List](https://symbol-tools.com/symbolTools/view/tool/nodeList.html)

    自身のノードが他のノードより大幅に遅れた状態が長期間続く場合は、さらなる調査が必要になる可能性があります。

## ディスク容量の管理 {: #disk-space-management }

ディスク容量の不足は、ノードが不安定になる一般的な原因です。
利用可能なストレージが不足したノードは、同期を停止します。

ローカル、リモート、またはクラウドサーバーを使用している場合はインフラプロバイダーの監視ツールを使用して、利用可能なディスク容量を把握しておいてください。

ストレージが危機的に少なくなる前に、ノードを正常に停止することをお勧めします。

より大きなストレージに移行する必要がある場合、ファイルを移動する前に**ノードが完全に停止されていれば**、ノードのディレクトリを安全に移動できます。

!!! tip "ヒント"
    問題を最小限に抑えるために、[インストール](./install.md#hardware) ガイドで推奨されているハードウェアおよびストレージのガイドラインに常に従ってください。

## 再起動と復旧 {: #restarting-and-recovery }

予期しないシャットダウン、電源喪失、またはシステムエラーにより、ノードが停止したり、正しく起動できなくなったりすることがあります。
ほとんどの問題は、クリーンな再起動といくつかの基本的なチェックで解決できます。

* **ノードを正常に停止する**
{: #stop-the-node-cleanly :}

    メンテナンスや復旧作業を行う前に、すべてのサービスを正常に終了（graceful stop）させます。

    ```bash
    docker compose down
    ```

    これにより、コンテナが正常にシャットダウンし、データベース破損のリスクを軽減できます。
    ノードが正常にシャットダウンするまでに最大5分かかる場合があることに留意してください。

* **ノードを再度起動する**
{: #start-the-node-again :}

    シャットダウン後にノードを起動するには：

    ```bash
    docker compose up -d
    ```

    起動後、すべてのサービスが初期化されるまで少し待ちます。

* **コンテナのステータスを確認する**

    必要なすべてのコンテナが実行されていることを確認します。

    ```bash
    docker compose ps -a
    ```

    いずれかのコンテナがすぐに終了したり、繰り返し再起動したりする場合は、[ログを調べてください](#view-logs)。

* **古いロックファイル**

    ノードが予期せず中断された場合、 `.lock` ファイルが `data` ディレクトリに残り、起動を妨げることがあります。

    そのようなファイルが見つかった場合は、[ノードの復旧](./quickstart.md#node-recovery) セクションをお読みください。

## ノードソフトウェアのアップデート {: #updating-node-software }

オペレーティングシステムとノードソフトウェアを最新の状態に保つことで、安定性、互換性、セキュリティが向上します。

* **Shoestringのアップデート**

    Shoestring自体はPythonパッケージとしてインストールされています。
    利用可能な最新バージョンにアップデートするには：

    ```bash
    python3 -m pip install --upgrade symbol-shoestring
    ```

    アップデート後、インストールされたバージョンを確認します。

    ```bash
    python3 -m pip show symbol-shoestring
    ```

* **ノードコンテナのアップデート**

    Shoestringは、すべてのSymbolコンポーネントの最新バージョンをダウンロードしてインストールするための `upgrade` コマンドを提供しています。
    定期的に実行するようにしてください。

    !!! warning "警告"

        アップグレードする前に[ノードを正常に停止](#stop-the-node-cleanly)し、その後[再起動](#start-the-node-again)してください。

    ```bash
    python3 -m shoestring upgrade \
        --config config.ini \
        --directory . \
        --overrides overrides.ini
    ```

    ノードを再起動した後、[正常に動作しているか確認してください](#monitoring)。

    !!! note "設定の変更"

        まれに、アップデートによってShoestring設定ファイルのフォーマットが変更され、既存の `config.ini` と互換性がなくなる場合があります。

        このような破壊的な変更を伴うアップデートは広くアナウンスされ、現在の `config.ini` をバックアップし、[ `init` コマンドで新しいバージョンをダウンロード](./quickstart.md#2-generate-the-network-configuration)し、以前の変更を手動でマージする必要がある場合があります。

## 鍵と証明書の更新 {: #key-and-certificate-renewals }

ノードの正常な動作には、いくつかの鍵と証明書が必要です。

セキュリティ上の理由から、これらはいずれも無期限に有効であるようには意図されていないため、定期的に更新する必要があります。

以下のセクションで示すように、Shoestringはこのプロセスを簡素化するための更新コマンドを提供しています。

### TLS証明書の更新 {: #tls-certificate-renewal }

他のピアノードとの通信は、ノードの [メインキー](default:メインキー) を使用して、Shoestringが[セットアップフェーズ](./quickstart.md#6-create-the-node-installation)で生成した[TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security)証明書を使用して暗号化します。

この証明書のデフォルトの有効期間は375日であり、 `renew-certificates` コマンドで更新します。

```bash
python3 -m shoestring renew-certificates \
    --config config.ini \
    --directory . \
    --ca-key-path ca.key.pem
```

`File Not Found` エラーが表示される場合は、 `--directory` パラメータに絶対パスを使用してみてください。

[`health` コマンド](#check-node-health-with-shoestring)は、有効期限が近づいており証明書を更新する必要がある場合に警告を出します。
[Symbol Node List](https://symbol-tools.com/symbolTools/view/tool/nodeList.html)でリモートから現在の証明書の有効期限を確認することもできます。

!!! danger "メインアカウントキーの保護"

    新しい証明書を作成するには、ノードの [メインキー](default:メインキー) の秘密鍵が含まれるPEMファイルへのアクセスが必要です。

    このファイルは安全に保管する必要があり、通常はノード自体に残しておくべきではありません。

!!! warning "ノードキーの保持"

    デフォルトでは、上記のコマンドは新しいノードキーペアを生成します。

    これにより、 [委任ハーベスティング](default:委任ハーベスティング) を行う人をノードに登録する際に使用する公開鍵が変更されます。

    既存の委任者は、**ノードがブロックチェーンの完全な再同期を実行する必要がない限り**、通常は中断されることなく引き続き委任ハーベスティングを行います。

    既存のノードキーを引き続き使用し、この可能性を回避するには、証明書を更新する際に `--retain-node-key` パラメータを使用します。

### 投票キーの更新 {: #voting-key-renewal }

[投票ノード](default:投票ノード) には [投票キー](default:投票キー) が必要であり、限られた有効期間で登録する必要があります。

セキュリティ上の理由から、各投票キーの有効期間は最大180日（約6ヶ月）であるため、ノードが [メインネット](default:メインネット) と [テストネット](default:テストネット) の両方でファイナライゼーションの投票資格を維持するには、鍵を定期的に更新する必要があります。

任意の時点で投票プロセスに必要な鍵は1つだけですが、最大3つの鍵を同時に登録できます。
これにより有効期間を重複させることができるため、ある鍵の期限が切れた場合でもバックアップキーを引き続き利用できます。

Shoestringが[投票ノードを作成](./quickstart.md#configure-voting-optional)する際、投票キーを登録しています。
更新するためのコマンドも提供されています。

```bash
python3 -m shoestring renew-voting-keys --config config.ini --directory .
```

このコマンドは、現在の鍵の有効期限が切れるとアクティブになる新しい鍵を作成し、以下のために必要なトランザクションを準備します。

* 期限切れの投票キーのリンクを解除する。
* 新しい鍵をノードにリンクする。

トランザクションは `renew_voting_keys_transaction.dat` という名前のファイルに保存されます。

その後、[ノードの作成と実行](./quickstart.md#8-sign-the-transactions)ガイドで説明されているプロセスに従って、これらを手動で署名し、アナウンスする必要があります。

!!! danger "メインアカウントキーの保護"

    これらのトランザクションに署名するには、ノードの [メインキー](default:メインキー) の秘密鍵が含まれるPEMファイルへのアクセスが必要です。

    このファイルは安全に保管する必要があり、通常はノードに残しておくべきではありません。

## バックアップ {: #backups }

他の多くのサービスとは異なり、Symbolノードは通常、ブロックチェーンデータベースのバックアップを必要としません。

ブロックチェーンはすでに多くの独立したノードに複製されています。
ローカルのチェーンデータが失われたり破損したりした場合でも、通常はネットワークからノードを再同期できます。

このため、バックアップは自動的に復元できないローカルファイルに焦点を当てる必要があります。

* Shoestring設定ファイル `config.ini`
* ノードIDキーファイル `ca.key.pem`
* ノードのカスタマイズ設定 `overrides.ini`

ハーベスティングが有効になっている場合、完全な再同期により、現在登録されている委任者を追跡する `harvesters.dat` ファイルが通常は再生成されます。

そのため、ブロックチェーンが[スナップショットから復元](https://github.com/symbol/product/tree/dev/tools/shoestring#need-to-resync-your-node)されない限り、このファイルのバックアップは通常不要です。
