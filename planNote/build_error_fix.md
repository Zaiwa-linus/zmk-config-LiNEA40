## ビルドエラー対処メモ

### エラー内容
左右両方のビルドで同じエラーが発生:
```
zmk-rgbled-widget/src/widget.c:19:10: fatal error: zmk/split/central.h: No such file or directory
```

### 原因
`zmk-rgbled-widget` (caksoylar/zmk-rgbled-widget, revision: main) が `zmk/split/central.h` をインクルードしているが、
ZMK v0.2.1 (Zephyr 3.5.0ベース) にはこのヘッダが存在しない。
rgbled-widget の main ブランチが ZMK の最新版（v0.2.1より新しいAPI）に追従した結果、互換性が壊れている。

**注意:** このエラーはbattery_typistモジュールとは無関係。rgbled-widgetの問題。

### 対処案

#### 案A: zmk-rgbled-widget のリビジョンを固定する（推奨）
`config/west.yml` で `revision: main` を、v0.2.1と互換性のあるコミットハッシュに固定する。

```yaml
# 変更前
- name: zmk-rgbled-widget
  remote: caksoylar
  revision: main

# 変更後（互換コミットを特定して指定）
- name: zmk-rgbled-widget
  remote: caksoylar
  revision: <v0.2.1対応の最後のコミットハッシュ>
```

**コミットハッシュの探し方:**
1. https://github.com/caksoylar/zmk-rgbled-widget/commits/main を確認
2. `zmk/split/central.h` を導入したコミットを見つけ、その1つ前のコミットを使う
3. または `git log --all -S "zmk/split/central.h"` で検索

#### 案B: ZMK本体のバージョンを上げる
`config/west.yml` で ZMK のリビジョンを最新に更新する。

```yaml
# 変更前
- name: zmk
  remote: zmkfirmware
  revision: v0.2.1

# 変更後
- name: zmk
  remote: zmkfirmware
  revision: main
```

**リスク:** 他のモジュール（pmw3610-driverなど）やシールド定義との互換性が壊れる可能性あり。
ZMK v0.2.1 → main は破壊的変更が多いため、keymap・overlay・confの修正が必要になる場合がある。

#### 案C: rgbled-widget を一時的に外す
`build.yaml` と `west.yml` から rgbled_adapter を除外して、battery_typistモジュールの動作確認を先に行う。

```yaml
# build.yaml: shieldから rgbled_adapter を除外
- board: seeeduino_xiao_ble
  shield: LiNEA40_left
  cmake-args: ...
- board: seeeduino_xiao_ble
  shield: LiNEA40_right
  cmake-args: ...
```

**メリット:** battery_typistの動作確認を最短で行える
**デメリット:** LED表示機能が一時的に無効になる

### 推奨手順
1. まず **案C** でrgbled-widgetを外し、battery_typistモジュールのビルドが通るか確認
2. battery_typistが動作確認できたら、**案A** でrgbled-widgetの互換バージョンを特定して復帰
