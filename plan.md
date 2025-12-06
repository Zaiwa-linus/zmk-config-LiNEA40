BT Layer Behavior 概要
======================

目的
----
- 独自ビヘイビア `zmk,behavior-bt-layer` で BT プロファイル選択とレイヤ強制切替を連動させ、接続先に応じてキーレイアウトを自動で合わせる。

動作仕様
--------
- 呼び出しシグネチャ: `&bt_layer <profile_index>`（パラメータ1個）。
- 押下時 `on_keymap_binding_pressed`:
  - `zmk_ble_prof_select(profile_index)` で BT スロットを切替。
  - `update_layers_for_profile(profile_index)` でレイヤを強制設定:
    - プロファイル1 → レイヤ1 ON, レイヤ2 OFF
    - プロファイル2 → レイヤ2 ON, レイヤ1 OFF
    - それ以外 (0/3/4 等) → レイヤ1/2 とも OFF
- 離したときは何もしない（opaque）。
- レイヤ番号はインデックスそのまま。`config/roBa.keymap` では layer1=mac, layer2=ios。ベースレイヤ(layer0)はこのビヘイビアでは触らない。

関連ファイルと設定
------------------
- `app/Kconfig`: `CONFIG_ZMK_BEHAVIOR_BT_LAYER` 定義（`ZMK_BLE` 依存, split では中央のみ）。
- `app/CMakeLists.txt`: フラグ有効時に `src/behavior_bt_layer.c` をビルド。
- `app/dts/bindings/behaviors/zmk,behavior-bt-layer.yaml`: DTS バインディング (one_param)。
- `app/dts/behaviors/bt_layer.dtsi`: `bt_layer` ノードを登録し keymap から呼べるようにする。
- `boards/shields/roBa/roBa_R.conf`: `CONFIG_ZMK_BEHAVIOR_BT_LAYER=y` で有効化。
- `zephyr/module.yml`: Kconfig/CMake のルートを `app/` に向けて検出させる。

サンプルコード
--------------
**ビヘイビア本体** (`app/src/behavior_bt_layer.c` 抜粋):
```c
#define LAYER_1 1
#define LAYER_2 2

static void update_layers_for_profile(uint8_t profile_index) {
    switch (profile_index) {
    case 1:
        zmk_keymap_layer_deactivate(LAYER_2);
        zmk_keymap_layer_activate(LAYER_1);
        break;
    case 2:
        zmk_keymap_layer_deactivate(LAYER_1);
        zmk_keymap_layer_activate(LAYER_2);
        break;
    default:
        zmk_keymap_layer_deactivate(LAYER_1);
        zmk_keymap_layer_deactivate(LAYER_2);
        break;
    }
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    uint8_t profile_index = binding->param1;
    int ret = zmk_ble_prof_select(profile_index);
    if (ret < 0) {
        return ret;
    }
    update_layers_for_profile(profile_index);
    return ZMK_BEHAVIOR_OPAQUE;
}
```

**Kconfig** (`app/Kconfig`):
```kconfig
config ZMK_BEHAVIOR_BT_LAYER
    bool "BT Profile Layer Behavior"
    default n
    depends on ZMK_BLE
    depends on !ZMK_SPLIT || ZMK_SPLIT_ROLE_CENTRAL
```

**CMake** (`app/CMakeLists.txt`):
```cmake
if(CONFIG_ZMK_BEHAVIOR_BT_LAYER)
  target_sources(app PRIVATE src/behavior_bt_layer.c)
endif()
```

**DTS バインディング/ノード**:
```yaml
# app/dts/bindings/behaviors/zmk,behavior-bt-layer.yaml
compatible: "zmk,behavior-bt-layer"
include: one_param.yaml
```
```dts
/* app/dts/behaviors/bt_layer.dtsi */
/ {
    behaviors {
        /omit-if-no-ref/ bt_layer: bt_layer {
            compatible = "zmk,behavior-bt-layer";
            #binding-cells = <1>;
        };
    };
};
```

**キー割り当て例** (`config/roBa.keymap` の setting レイヤ抜粋):
```dts
#include <behaviors/bt_layer.dtsi>

        setting {
            bindings = <
&bt BT_SEL 0  &bt BT_SEL 1  &bt BT_SEL 2  &bt BT_SEL 3  &bt BT_SEL 4
                                        &bt_layer 0  &bt_layer 1  &bt_layer 2  &bt_layer 3  &bt_layer 4
            >;
        };
```
- `&bt_layer 1` → プロファイル1 + mac レイヤ ON
- `&bt_layer 2` → プロファイル2 + ios レイヤ ON
- `&bt_layer 0/3/4` → プロファイル切替のみ、mac/ios レイヤ OFF
