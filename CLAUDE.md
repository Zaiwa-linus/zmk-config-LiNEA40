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

レイヤー構成: `default_layer` / `MOUSE` / `MARK` / `CURSOR` / `FUNCTION` / `SCROLL` / `WIRELESS`

コンボ、センサー回転ビヘイビア、入力プロセッサ（カーソル加速・スクロール変換）も同ファイル内で定義しています。

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

- **`Makefile` と `build.yaml` の引数が一致していません。** `Makefile` の左手側ターゲットは
  `rgbled_adapter` シールドと `ZMK_EXTRA_MODULES` を渡しておらず、CI と異なる成果物になります。
  ローカルで CI と同じものを作る場合は両方を明示的に指定してください。
- `.conf` や overlay を変更したときは `west build -p` で pristine ビルドしてください。
  差分ビルドでは Kconfig / devicetree が再生成されません。
- 依存モジュールのリビジョンは `config/west.yml` が正です。ローカルの clone を更新するときは
  ここと揃えてください（`zmk-pmw3610-driver` は `main` 追従なので挙動が変わる可能性があります）。
- 左右の `.uf2` を取り違えて書き込むと正しく動作しません。

## このリポジトリは公開されています

コミットするファイルに個人情報（ローカルの絶対パス、メールアドレス、アカウント名など）を
含めないでください。ドキュメント内のパスは `$WS` のような変数で表記します。
