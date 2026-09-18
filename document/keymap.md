# キーマップ

`config/LiNEA40.keymap` を [keymap-drawer](https://github.com/caksoylar/keymap-drawer) で図にしたものです。
キーマップを変更したら `make keymap-svg` で再生成してください。

![LiNEA40 keymap](img/LiNEA40-keymap.svg)

## レイヤー

| # | 名前 | 入り方 |
|---|---|---|
| 0 | `default` | — |
| 1 | `mac` | `BT 1`（FUNCTION レイヤー）で有効化。BT プロファイル1と連動 |
| 2 | `ios` | `BT 2`（FUNCTION レイヤー）で有効化。BT プロファイル2と連動 |
| 3 | `MOUSE` | トラックボールを動かすと自動で有効（600ms 後に解除） |
| 4 | `MARK` | 左親指 `LANG2` を長押し |
| 5 | `CURSOR_win` | 右親指 `LANG1` を長押し |
| 6 | `CURSOR_mac` | `mac` レイヤー中に右親指 `LANG1` を長押し |
| 7 | `CURSOR_ios` | `ios` レイヤー中に右親指 `LANG1` を長押し |
| 8 | `FUNCTION` | 左親指 `SPACE` を長押し。この間トラックボールはスクロールになる |
| 9 | `GAME` | `T` + `B` のコンボでトグル。もう一度同じコンボで解除（試験的） |

`extra_0`–`extra_2`（10–12）は ZMK Studio が実行時にレイヤーを追加するための空きスロットで、
`status = "disabled"` のため図には含めていません。

## 図の読み方

- キー中央が単押し、左上が長押し（モディファイア）、左下が長押しで開くレイヤー名
- `▽` は `&trans`（下のレイヤーをそのまま通す）
- 薄い赤のキーは、そのレイヤーを開いているキー自身
- 青い小さなキーはコンボ。2キー同時押しで発動し、どのレイヤーでも有効

## 再生成

```sh
make keymap-svg
```

初回は `.venv-keymap/` に仮想環境を作って keymap-drawer を入れます（`.gitignore` 済み）。
2回目以降は生成だけ走ります。

表示名の調整は `keymap_drawer.config.yaml` で行います。
このキーマップは C で書いた独自ビヘイビア（`bt_layer` / `bt_base` / `batt_disp`）を使っており、
keymap-drawer はそれらの意味を知らないため、`raw_binding_map` で表示名を与えています。
ビヘイビアを追加したら、ここにも1行足してください。

## Python のバージョン要件

keymap-drawer 0.23 以降は **Python 3.11 以上**を要求します。
3.10 以下で `pip install keymap-drawer` すると古い 0.21.0 が入り、
現在の tree-sitter（0.25 以降）で削除された API を呼ぶため次のように失敗します。

```
AttributeError: 'tree_sitter.Language' object has no attribute 'query'
AttributeError: 'tree_sitter.Query' object has no attribute 'captures'
```

`make keymap-svg` は実行前に Python のバージョンを確認し、3.11 未満なら
分かりやすいエラーで止まります。システムの `python3` が古い場合は、
使うインタプリタを指定してください。

```sh
make keymap-svg KEYMAP_PYTHON=python3.12
```

3.11 以上であればバージョン固定は不要で、最新の keymap-drawer と
tree-sitter の組み合わせで動きます。

## ブラウザで試す

インストールせずに確認するだけなら https://caksoylar.github.io/keymap-drawer/ に
`config/LiNEA40.keymap` の中身を貼り付けても描画できます。
ただし独自ビヘイビアの表示名は当たりません。
