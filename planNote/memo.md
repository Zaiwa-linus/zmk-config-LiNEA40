## DYA Studio 導入作業メモ（作業完了）


### 1. 概要

[DYA Studio](https://studio.dya.cormoran.works/) は cormoran 氏が開発した ZMK Studio ベースのWebキーマップ編集ツール。
ブラウザから USB / Bluetooth 経由でリアルタイムにキーマップ変更が可能。
トラックボールスクロール設定、オートマウス設定、バッテリー履歴表示などに対応。

### 2. 現状の問題

| 項目 | 現状 | DYA Studio 要件 |
|------|------|-----------------|
| ZMK バージョン | `v0.2.1`（zmkfirmware/zmk） | cormoran/zmk `v0.3-branch+dya` |
| DYA 用モジュール | なし | 4つのモジュールが必要 |
| Studio 設定 | ZMK Studio有効だが公式版 | DYA Studio 用設定に変更が必要 |
| keymap | `&studio_unlock` なし | `&studio_unlock` キーが必要 |

**ZMK v0.2.1 → v0.3 への移行が必須。** cormoran 氏の ZMK フォーク（v0.3ベース）を使用する必要がある。

### 3. west.yml の変更内容

現在の `config/west.yml`:
```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: inorichi
      url-base: https://github.com/inorichi
    - name: caksoylar
      url-base: https://github.com/caksoylar
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: v0.2.1
      import: app/west.yml
    - name: zmk-pmw3610-driver
      remote: inorichi
      revision: main
    - name: zmk-rgbled-widget
      remote: caksoylar
      revision: e9472d6
  self:
    path: config
```

変更後:
```yaml
manifest:
  remotes:
    - name: cormoran
      url-base: https://github.com/cormoran
    - name: inorichi
      url-base: https://github.com/inorichi
    - name: caksoylar
      url-base: https://github.com/caksoylar
  projects:
    - name: zmk
      remote: cormoran
      revision: v0.3-branch+dya
      import: app/west.yml
    - name: zmk-module-ble-management
      remote: cormoran
      revision: main
    - name: zmk-module-battery-history
      remote: cormoran
      revision: main
    - name: zmk-module-settings-rpc
      remote: cormoran
      revision: main
    - name: zmk-module-runtime-input-processor
      remote: cormoran
      revision: main
    - name: zmk-pmw3610-driver
      remote: inorichi
      revision: main
    - name: zmk-rgbled-widget
      remote: caksoylar
      revision: e9472d6
  self:
    path: config
```

### 4. Kconfig 設定の追加（`LiNEA40_right.conf`）

以下を追加済み:
```
# DYA Studio
CONFIG_ZMK_STUDIO=y
CONFIG_ZMK_STUDIO_LOCKING=n
CONFIG_ZMK_STUDIO_TRANSPORT_BLE=y
CONFIG_ZMK_STUDIO_LOCK_BLE_DIRECT_ADVERTISING_ON_UNLOCK=y
CONFIG_ZMK_RUNTIME_INPUT_PROCESSOR=y
CONFIG_ZMK_RUNTIME_INPUT_PROCESSOR_STUDIO_RPC=y
CONFIG_ZMK_BLE_MANAGEMENT=y
CONFIG_ZMK_BLE_MANAGEMENT_STUDIO_RPC=y
CONFIG_ZMK_BATTERY_HISTORY=y
CONFIG_ZMK_BATTERY_HISTORY_STUDIO_RPC=y
CONFIG_ZMK_SETTINGS_RPC=y
CONFIG_ZMK_SETTINGS_RPC_STUDIO=y
```

### 5. keymap の変更（`LiNEA40.keymap`）

- `&studio_unlock` を WIRELESS レイヤー右手中段右端に配置
- `#include <input/processors/runtime-input-processor.dtsi>` を追加
- `&trackball_listener` に `input-processors = <&mouse_runtime_input_processor>;` を追加
- scroller に `<&scroll_runtime_input_processor>` を追加

### 6. カスタムモジュールの API 互換性リスク

v0.2.1 → v0.3 で ZMK 内部 API が変更されている可能性がある。以下のAPIに注意:

| API | 用途 | リスク |
|-----|------|--------|
| `zmk_hid_keyboard_press/release` | キー入力送信 | シグネチャ変更の可能性 |
| `zmk_endpoints_send_report(HID_USAGE_KEY)` | HIDレポート送信 | 引数やAPI名変更の可能性 |
| `zmk_split_get_peripheral_battery_level` | ペリフェラルバッテリー取得 | ヘッダパスやAPI変更の可能性 |
| `BEHAVIOR_DT_INST_DEFINE` | ビヘイビア登録マクロ | 引数変更の可能性 |
| `#include <zmk/split/bluetooth/central.h>` | ヘッダパス | パス変更の可能性 |

`behavior_os_layer.c` も同様に影響を受ける可能性あり。

#### 実際に発生した API 変更（v0.3 対応で修正済み）

`behavior_battery_type.c` で以下2点を修正:
- `#include <zmk/split/bluetooth/central.h>` → `#include <zmk/split/central.h>`
- `zmk_split_get_peripheral_battery_level()` → `zmk_split_central_get_peripheral_battery_level()`

### 7. 対応手順

1. **新ブランチを切る** ✅
2. **west.yml を変更** — cormoran フォーク + DYA モジュール4つに切り替え ✅
3. **ビルドして API エラーを確認** — v0.3 での API 変更を洗い出し ✅
4. **カスタムモジュールを修正** — `behavior_battery_type.c` の API 呼び出し2箇所を修正 ✅
5. **keymap に `&studio_unlock` を追加** — WIRELESS レイヤー右手中段右端に配置 ✅
6. **Kconfig に Studio 設定を追加** — `LiNEA40_right.conf` に DYA Studio 用設定14行追加 ✅
7. **keymap に runtime input processor を追加** — トラックボール・スクロールに対応 ✅
8. **ビルド＆フラッシュして DYA Studio との接続を確認** — 未実施

### 8. 変更ファイル一覧（production との差分）

| ファイル | 変更内容 |
|---------|---------|
| `config/west.yml` | ZMK を cormoran フォーク v0.3 に変更、DYA モジュール4つ追加 |
| `config/boards/shields/LiNEA40/LiNEA40_right.conf` | DYA Studio 用 Kconfig 設定14行追加 |
| `config/LiNEA40.keymap` | runtime-input-processor 追加、`&studio_unlock` 追加 |
| `modules/custom_battery_typist/src/behavior_battery_type.c` | v0.3 API 変更に対応（ヘッダパス・関数名） |
| `.claude/settings.local.json` | Claude Code ローカル設定（開発用） |

### 9. 参考リンク

- [DYA Studio](https://studio.dya.cormoran.works/)
- [DYA Studioを導入してみるぞ！moNa2編｜おぐ](https://note.com/heace/n/nf06b797ffa79)
- [Compatible DYAStudio｜RaZiLy](https://note.com/razily/n/n7a23e5a7512c)
- [ZMK Config を変更する | DYA Dash](https://cormoran.github.io/dya-dash-keyboard/feature-guide/zmk_config/)


