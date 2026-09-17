# Docker を使ったローカルビルド手順 (macOS)

GitHub Actions を待たずに手元でファームウェアをビルドするための手順です。
ZMK 公式のビルドイメージ `zmkfirmware/zmk-build-arm:stable` を使うため、
ホスト側に Zephyr SDK や arm-none-eabi ツールチェーンを入れる必要はありません。

Apple Silicon / Intel のどちらでも動作します（イメージは multi-arch）。

## 1. 前提

- Docker が動く環境（Docker Desktop または OrbStack）
- `git`
- 空きディスク 5GB 程度（イメージ 2.2GB + Zephyr ツリー）

ホスト側の `west` / `cmake` / `arm-none-eabi-gcc` は使いません。すべてコンテナ内で完結します。

## 2. ワークスペースを用意する

`Makefile` は以下のディレクトリ構成を前提にしています。
任意の作業ディレクトリ（以下 `$WS`）の下に3つを並べてください。

```
$WS/
├── zmk/                      ZMK 本体 (v0.2.1)
├── zmk-modules/
│   ├── zmk-pmw3610-driver    トラックボール用ドライバ
│   └── zmk-rgbled-widget     RGB LED インジケータ
└── zmk-config-LiNEA40/       このリポジトリ
```

リビジョンは [config/west.yml](../config/west.yml) の定義と揃えます。

```sh
export WS=~/keyboard          # 任意の場所
mkdir -p "$WS" && cd "$WS"

# このリポジトリ（未クローンなら）
git clone https://github.com/keyfreaks/zmk-config-LiNEA40.git

# ZMK 本体
git clone --depth 1 --branch v0.2.1 https://github.com/zmkfirmware/zmk.git zmk

# 外部モジュール
mkdir -p zmk-modules && cd zmk-modules
git clone --depth 1 --branch main       https://github.com/inorichi/zmk-pmw3610-driver.git
git clone --depth 1 --branch v0.3-branch https://github.com/caksoylar/zmk-rgbled-widget.git
cd "$WS"
```

## 3. ビルド用コンテナを起動する

3つのディレクトリを `/workspaces/` 配下にマウントした常駐コンテナを作ります。
コンテナ名 `zmk-build` は `Makefile` に渡す `container_name` と一致させます。

```sh
docker run -d --name zmk-build \
  -v "$WS/zmk":/workspaces/zmk \
  -v "$WS/zmk-config-LiNEA40":/workspaces/zmk-config \
  -v "$WS/zmk-modules":/workspaces/zmk-modules \
  zmkfirmware/zmk-build-arm:stable sleep infinity
```

バインドマウントなので、生成された `.uf2` はホスト側からそのまま参照できます。

## 4. Zephyr を取得する（初回のみ）

```sh
docker exec -w /workspaces/zmk zmk-build west init -l app
docker exec -w /workspaces/zmk zmk-build west update        # 数分かかります
docker exec -w /workspaces/zmk zmk-build west zephyr-export
```

`west update` は Zephyr 本体と HAL 一式を取得するため、回線によっては 5〜10 分程度かかります。
成果物はマウント経由でホスト側の `$WS/zmk/` に残るので、コンテナを作り直しても再取得は不要です。

## 5. ビルドする

### Makefile を使う場合

```sh
cd "$WS/zmk-config-LiNEA40"
make container_name=zmk-build
```

### west を直接叩く場合

左手側:

```sh
docker exec -w /workspaces/zmk/app zmk-build \
  west build -p -d build/left -b seeeduino_xiao_ble -- \
    -DSHIELD="LiNEA40_left rgbled_adapter" \
    -DZMK_CONFIG="/workspaces/zmk-config/config" \
    -DZMK_EXTRA_MODULES="/workspaces/zmk-modules/zmk-pmw3610-driver;/workspaces/zmk-modules/zmk-rgbled-widget;/workspaces/zmk-config/config"
```

右手側（セントラル / ZMK Studio 有効）:

```sh
docker exec -w /workspaces/zmk/app zmk-build \
  west build -p -d build/right -b seeeduino_xiao_ble -S studio-rpc-usb-uart -- \
    -DSHIELD="LiNEA40_right rgbled_adapter" \
    -DZMK_CONFIG="/workspaces/zmk-config/config" \
    -DZMK_EXTRA_MODULES="/workspaces/zmk-modules/zmk-pmw3610-driver;/workspaces/zmk-modules/zmk-rgbled-widget;/workspaces/zmk-config/config" \
    -DCONFIG_ZMK_STUDIO=y -DCONFIG_ZMK_STUDIO_LOCKING=n
```

`ZMK_EXTRA_MODULES` の最後に `zmk-config/config` 自身が入っている点に注意してください。
このリポジトリはカスタムビヘイビアの C ソースを持ち、それを Zephyr モジュールとして
読み込ませるために必要です。これを外すとビヘイビアがコンパイルされず、リンクに失敗します。
GitHub Actions では `config/` が west のマニフェストリポジトリなので自動検出され、この指定は不要です。

`-p` は pristine（クリア）ビルドです。keymap だけの変更なら省略して差分ビルドできます。

## 6. 成果物

```
$WS/zmk/app/build/left/zephyr/zmk.uf2      → 左手側
$WS/zmk/app/build/right/zephyr/zmk.uf2     → 右手側
```

`make` を使った場合は `$WS/zmk/app/build/LiNEA40_{left,right}.uf2` にもコピーされます。

ビルド成功時は末尾にメモリ使用量が出ます。参考値（v0.2.1 時点）:

| | FLASH | RAM |
|---|---|---|
| 左 | 約 174KB / 788KB (22%) | 約 36KB / 256KB (14%) |
| 右 | 約 261KB / 788KB (33%) | 約 78KB / 256KB (31%) |

## 7. 書き込み

XIAO BLE のリセットボタンを素早く2回押すとブートローダーが起動し、`XIAO-SENSE` という
USB マスストレージとしてマウントされます。そこへ `.uf2` をコピーすると自動的に再起動して反映されます。

左右それぞれ対応する `.uf2` を書き込んでください。取り違えると正しく動作しません。

## 2回目以降

コンテナが停止していたら起動し直すだけです。

```sh
docker start zmk-build
cd "$WS/zmk-config-LiNEA40" && make container_name=zmk-build
```

## トラブルシューティング

**`docker exec` が "No such container" になる**
コンテナが削除されています。手順3からやり直してください。`$WS/zmk` は残っているので `west update` は不要です。

**`Makefile` がビルドをスキップする**
`make` はターゲットの `.uf2` とソースのタイムスタンプを比較します。強制的に作り直すには `make clean container_name=zmk-build` を実行してください。

**設定変更が反映されない**
`.conf` や overlay を変更した場合は Kconfig / devicetree の再生成が必要です。`-p` を付けた pristine ビルドにしてください。

**カスタムビヘイビアで `undefined reference` が出る**
`ZMK_EXTRA_MODULES` に `zmk-config/config` を含め忘れています。上記「west を直接叩く場合」を参照してください。

**`west update` が途中で失敗する**
ネットワーク起因が大半です。同じコマンドを再実行すれば続きから再開されます。

## 代替手段: GitHub Actions

ローカル環境を用意せずに済む方法です。[.github/workflows/build.yml](../.github/workflows/build.yml) が
ZMK 公式のビルドワークフローを呼んでおり、[build.yaml](../build.yaml) のマトリクスに沿って
左右 + `settings_reset` をビルドします。push すると自動で走り、Actions の Artifacts から `.uf2` を取得できます。

手元で走らせる場合:

```sh
gh workflow run build.yml
```
