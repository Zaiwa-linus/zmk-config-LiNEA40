

## DYA Studio 導入計画

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

### 4. Kconfig 設定の追加

`.conf` ファイルに以下を追加:
```
CONFIG_ZMK_STUDIO=y
CONFIG_ZMK_STUDIO_LOCKING=n
CONFIG_ZMK_RUNTIME_INPUT_PROCESSOR=y
CONFIG_ZMK_RUNTIME_INPUT_PROCESSOR_STUDIO_RPC=y
```

### 5. keymap の変更

DYA Studio に接続するために `&studio_unlock` キーをキーマップに追加する必要がある。

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

### 7. 対応手順

1. **新ブランチを切る** — 現状の v0.2.1 動作を壊さないよう新ブランチで作業// done
2. **west.yml を変更** — cormoran フォーク + DYA モジュールに切り替え
3. **ビルドして API エラーを確認** — v0.3 での API 変更を洗い出す
4. **カスタムモジュールを修正** — コンパイルエラーに合わせて API 呼び出しを修正
5. **keymap に `&studio_unlock` を追加**
6. **Kconfig に Studio 設定を追加**
7. **ビルド＆フラッシュして DYA Studio との接続を確認**

### 8. 参考リンク

- [DYA Studio](https://studio.dya.cormoran.works/)
- [DYA Studioを導入してみるぞ！moNa2編｜おぐ](https://note.com/heace/n/nf06b797ffa79)
- [Compatible DYAStudio｜RaZiLy](https://note.com/razily/n/n7a23e5a7512c)
- [ZMK Config を変更する | DYA Dash](https://cormoran.github.io/dya-dash-keyboard/feature-guide/zmk_config/)


