# CLAUDE.md

このリポジトリで作業する Claude Code 向けのガイドです。

## 概要

自作分割キーボード **LiNEA40** の [ZMK](https://zmk.dev) ファームウェア設定リポジトリです。

| | |
|---|---|
| コントローラ | Seeed XIAO BLE (nRF52840) × 2 |
| 構成 | 左右分割・BLE 接続（右手側がセントラル） |
| ZMK バージョン | v0.2.1 |
| ポインティングデバイス | PMW3610 トラックボール（右手側） |
| エンコーダ | EC11（左手側） |
| インジケータ | RGB LED ウィジェット（rgbled_adapter） |
| ZMK Studio | 右手側で有効（`CONFIG_ZMK_STUDIO=y`） |
| カスタムビヘイビア | `batt_disp` / `bt_layer` / `bt_base`（C 実装、`config/` を Zephyr モジュール化して同梱） |

## ディレクトリ構成

```
config/
├── west.yml                   依存モジュールと ZMK のリビジョン定義
├── LiNEA40.keymap             実際に使われるキーマップ（ZMK_CONFIG として参照される）
├── LiNEA40.json               キーマップエディタ用レイアウト定義
└── boards/shields/LiNEA40/
    ├── Kconfig.shield         シールド名の定義
    ├── Kconfig.defconfig      分割・セントラル役割などのデフォルト設定
    ├── LiNEA40.dtsi           左右共通のマトリクス / 物理レイアウト
    ├── LiNEA40_left.overlay   左手側の devicetree（EC11 など）
    ├── LiNEA40_right.overlay  右手側の devicetree（PMW3610 SPI など）
    ├── LiNEA40_left.conf      左手側の Kconfig
    ├── LiNEA40_right.conf     右手側の Kconfig（トラックボール設定の大半はここ）
    └── LiNEA40.keymap         シールド同梱のデフォルトキーマップ（通常は編集不要）
build.yaml                     GitHub Actions のビルドマトリクス
Makefile                       Docker コンテナ経由のローカルビルド
document/                      ドキュメント
```

### キーマップの編集場所

編集するのは **[config/LiNEA40.keymap](config/LiNEA40.keymap)** です。
`config/boards/shields/LiNEA40/LiNEA40.keymap` はシールド定義に同梱されたデフォルトで、
ビルド時は `ZMK_CONFIG` で指定される `config/` 側が優先されます。

レイヤー構成（番号は `#define` と実際のノード順が一致している必要があります）:

| # | 名前 | 用途 |
|---|---|---|
| 0 | `default_layer` | ベース |
| 1 / 2 | `mac` / `ios` | ホスト OS 別の修飾キー差し替え。`bt_layer` が BT プロファイルと連動して有効化 |
| 3 | `MOUSE` | トラックボール操作時に自動で有効（automouse） |
| 4 | `MARK` | 記号 |
| 5 / 6 / 7 | `CURSOR_win` / `CURSOR_mac` / `CURSOR_ios` | 数字・カーソル。OS 別 |
| 8 | `FUNCTION` | ファンクション、BT プロファイル選択 |
| 9 | `SCROLL` | トラックボールがスクロールになる層。BT 管理、`batt_disp`、`bootloader`、`studio_unlock` も同居 |
| 10–12 | `extra_0`–`extra_2` | `status = "disabled"`。ZMK Studio が実行時にレイヤーを追加するための空きスロット |

レイヤー番号のマクロには `LYR_` を付けています（`LYR_SCROLL` など）。
接頭辞なしの `SCROLL` のような名前にすると、プリプロセッサが**レイヤーのノード名まで数値に置換**してしまい、
ZMK Studio でレイヤー名が `9` のように表示されてしまうためです。

コンボ、センサー回転ビヘイビア、入力プロセッサ（カーソル加速・スクロール変換）も同ファイル内で定義しています。

### カスタムビヘイビア

C 実装が `config/boards/shields/LiNEA40/src/` にあります。

| ビヘイビア | 機能 |
|---|---|
| `batt_disp` | 左右のバッテリー残量を `L:XX% R:XX%` という文字列として HID 入力する |
| `bt_layer <n>` | BT プロファイル `n` を選択し、対応するホストレイヤー（1–4）を有効化する |
| `bt_base <n>` | BT プロファイル `n` を選択し、ホストレイヤーを解除してベースへ戻す。現在キーには未割り当てで、ZMK Studio から割り当てる用 |

**Zephyr はシールドディレクトリの `CMakeLists.txt` を処理しません。** そのため
`config/zephyr/module.yml` で `config/` 自体を Zephyr モジュールとして宣言し、
`config/CMakeLists.txt` から `target_sources()` でソースを追加しています。
GitHub Actions では `config/` が west のマニフェストリポジトリなので自動的にモジュールとして検出されますが、
ローカルビルドでは `ZMK_EXTRA_MODULES` に `config/` のパスを含める必要があります（`Makefile` は対応済み）。

いずれも `#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)` と
`#if DT_HAS_COMPAT_STATUS_OKAY(...)` で自己ガードしているため、
左手側や `settings_reset` のビルドでも安全にコンパイルされます。

### トラックボールの調整場所

CPI、反転、オートマウスレイヤーのタイムアウトなどは
[config/boards/shields/LiNEA40/LiNEA40_right.conf](config/boards/shields/LiNEA40/LiNEA40_right.conf) の
`CONFIG_PMW3610_*` で設定します。SPI ピンやセンサーノード自体は
[LiNEA40_right.overlay](config/boards/shields/LiNEA40/LiNEA40_right.overlay) 側です。

## ビルド

**ローカルビルドの手順は [document/local-build-docker.md](document/local-build-docker.md) を参照してください。**
Docker（Docker Desktop / OrbStack）でワークスペースを組み立てる方法、
`west` コマンドの引数、書き込み手順、トラブルシューティングをまとめています。

要点のみ:

- ローカルビルドは `zmkfirmware/zmk-build-arm:stable` コンテナ内で行う。ホストに Zephyr SDK は不要
- `Makefile` は `/workspaces/{zmk,zmk-config,zmk-modules}` にマウントされた常駐コンテナを前提とし、
  `make container_name=<コンテナ名>` で呼び出す
- push すれば GitHub Actions が [build.yaml](build.yaml) のマトリクスに沿ってビルドし、
  Artifacts から `.uf2` を取得できる

## 注意点

- **レイヤー番号は 2 箇所で二重管理されています。** `config/LiNEA40.keymap` の `#define` と、
  `LiNEA40_right.overlay` のトラックボール設定（`automouse-layer` / `snipe-layers` / `scroll-layers`）です。
  後者は数値リテラルなので、レイヤーを追加・削除したら必ず両方を更新してください。
  現在は MOUSE=3 / SCROLL=9 に対応しています。snipe は使わないので `snipe-layers` は指定していません。
- `.conf` や overlay を変更したときは `west build -p` で pristine ビルドしてください。
  差分ビルドでは Kconfig / devicetree が再生成されません。
- 依存モジュールのリビジョンは `config/west.yml` が正です。ローカルの clone を更新するときは
  ここと揃えてください（`zmk-pmw3610-driver` は `main` 追従なので挙動が変わる可能性があります）。
- 左右の `.uf2` を取り違えて書き込むと正しく動作しません。

## このリポジトリは公開されています

コミットするファイルに個人情報（ローカルの絶対パス、メールアドレス、アカウント名など）を
含めないでください。ドキュメント内のパスは `$WS` のような変数で表記します。
