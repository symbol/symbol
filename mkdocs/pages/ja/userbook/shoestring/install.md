# Shoestringのインストール

このページでは、 [Shoestring](default:Shoestring) ツールのインストール方法と、それが正しく動作することを確認する方法について説明します。

ShoestringはPythonパッケージとして配布されており、 `pip` を使用してインストールできます。

## 要件 {: #requirements }

### ソフトウェア {: #software }

Shoestringをインストールする前に、システムで以下のソフトウェアが利用可能であることを確認してください。

* Python 3.10以降。
* `pip` （Pythonパッケージマネージャー）。
* Docker Compose（v2）をサポートする[Docker](https://www.docker.com)。

    ??? info "Docker Composeのインストール"

        プラグインとしてのインストールを推奨します。
        例えば、UbuntuやDebianでは以下を実行します。

        ```bash
        sudo apt install docker-compose-v2
        ```

        プラグインバージョンがシステムで利用できない場合は、古い `docker-compose` パッケージをインストールし、以下のコマンドの `docker compose` を `docker-compose` に置き換えてください（ハイフンに注意）。

* [OpenSSL](https://www.openssl.org/) コマンドラインツール。

    ??? info "OpenSSLのインストール"

        ノードで使用される暗号鍵を生成するには、**OpenSSLコマンドラインツール**が必要です。
        多くのシステムでは、これらはすでにインストールされているか、オペレーティングシステムのパッケージマネージャーを通じて利用可能です。
        存在しない場合は、 <https://openssl-library.org/source/> からダウンロードできます。
        
        システムでOpenSSLが利用可能かどうかを確認するには、 `openssl version` を実行します。
        コマンドがインストールされているバージョンを出力すれば、ツールは正しくインストールされており、システムのパスで利用可能です。

### ハードウェア {: #hardware }

ブロックチェーンノードの実行は、ディスクスペース、メモリ、およびCPUリソースの面で**非常に要求が厳しい**ものです。
以下の最小要件を満たしていない場合、ネットワークの他のノードに追いつくのに苦労するノードが作成されます。
グローバルなブロックチェーンは影響を受けませんが、ノードが同期を維持できずハーベスティング報酬を得るのが困難になります。

**専用のCPUとRAM**を使用することを**強く推奨**します。
これらが共有されている場合（一部の仮想サーバープロバイダーのように）、パフォーマンスは大きな影響を受けます。

=== "最小要件"

    | 要件 | ピアノード | APIノード | デュアル / 投票ノード |
    |----------------|---------------|---------------|--------------------|
    | **CPU** | 2 コア       | 4 コア       | 4 コア            |
    | **RAM** | 8 GB          | 16 GB         | 16 GB              |
    | **ディスクサイズ** | 500 GB        | 750 GB        | 750 GB             |
    | **ディスク速度** | 1500 IOPS SSD | 1500 IOPS SSD | 1500 IOPS SSD      |

=== "推奨要件"

    | 要件 | ピアノード | APIノード | デュアル / 投票ノード |
    |----------------|---------------|---------------|--------------------|
    | **CPU** | 4 コア       | 8 コア       | 8 コア            |
    | **RAM** | 16 GB         | 32 GB         | 32 GB              |
    | **ディスクサイズ** | 500 GB        | 750 GB        | 750 GB             |
    | **ディスク速度** | 1500 IOPS SSD | 1500 IOPS SSD | 1500 IOPS SSD      |

## pipを使用したインストール {: #installing-with-pip }

Pythonパッケージマネージャーを使用してShoestringをインストールします。

```bash
python3 -m pip install symbol-shoestring
```

このコマンドはShoestringツールとその依存関係をインストールします。

!!! warning "トラブルシューティング"

    一部のPythonの依存関係はソースからビルドされるため、一部のシステムではShoestringのインストールに追加のシステムパッケージが必要になる場合があります。

    ヘッダー、ライブラリ、またはコンパイラツールの欠落に関連するエラーでインストールが失敗した場合は、システムに必要な**開発用パッケージ**をインストールし、再度インストールを実行してください。

    一般的な症状には、`pyconfig.h`、OpenSSLヘッダー、またはビルドツールチェーンの欠落に言及するエラーが含まれます。

    UbuntuおよびDebianでは、通常、以下をインストールすれば十分です。

    ```bash
    sudo apt install python3-dev libssl-dev
    ```
    
    その後、Shoestringのインストールを再度実行してください。

## インストールの確認 {: #verifying-the-installation }

インストール後、以下を実行してツールが利用可能であることを確認します。

```bash
python3 -m shoestring --help
```

インストールが成功した場合、コマンドは利用可能なShoestringコマンドのリストを出力します。

特定のコマンドで利用可能なオプションを確認するには、次を使用します。

```bash
python3 -m shoestring <command> --help
```

## Shoestringのアップデート {: #updating-shoestring }

Shoestringを最新バージョンにアップグレードするには、以下を実行します。

```bash
python3 -m pip install --upgrade symbol-shoestring
```

## 次のステップ {: #next-steps }

Shoestringをインストールした後の次のステップは、[ノードの作成と起動](./quickstart.md)です。
