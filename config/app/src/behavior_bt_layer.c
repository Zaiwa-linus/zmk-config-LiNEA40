/*
 * Copyright (c) 2024 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 * BT Profile Layer Behavior
 * Combines Bluetooth profile selection with automatic layer switching.
 */

#define DT_DRV_COMPAT zmk_behavior_bt_layer

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>

#if IS_ENABLED(CONFIG_ZMK_BLE)
#include <zmk/ble.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_bt_layer_config {
    uint8_t layer_profile_1;
    uint8_t layer_profile_2;
};

static int update_layers_for_profile(const struct device *dev, uint8_t profile) {
    const struct behavior_bt_layer_config *config = dev->config;

    uint8_t layer1 = config->layer_profile_1;
    uint8_t layer2 = config->layer_profile_2;

    LOG_DBG("bt_layer: Updating layers for profile %d (layer1=%d, layer2=%d)",
            profile, layer1, layer2);

    switch (profile) {
        case 1:
            zmk_keymap_layer_activate(layer1);
            zmk_keymap_layer_deactivate(layer2);
            LOG_INF("bt_layer: Profile 1 - Layer %d ON, Layer %d OFF", layer1, layer2);
            break;
        case 2:
            zmk_keymap_layer_activate(layer2);
            zmk_keymap_layer_deactivate(layer1);
            LOG_INF("bt_layer: Profile 2 - Layer %d OFF, Layer %d ON", layer1, layer2);
            break;
        default:
            zmk_keymap_layer_deactivate(layer1);
            zmk_keymap_layer_deactivate(layer2);
            LOG_INF("bt_layer: Profile %d - Layer %d OFF, Layer %d OFF", profile, layer1, layer2);
            break;
    }

    return 0;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    uint8_t profile = binding->param1;

    LOG_DBG("bt_layer: Pressed with profile %d", profile);

#if IS_ENABLED(CONFIG_ZMK_BLE)
    int ret = zmk_ble_prof_select(profile);
    if (ret < 0) {
        LOG_ERR("bt_layer: Failed to select BT profile %d (err %d)", profile, ret);
        return ret;
    }
    LOG_INF("bt_layer: Selected BT profile %d", profile);
#endif

    return update_layers_for_profile(dev, profile);
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_bt_layer_init(const struct device *dev) {
    return 0;
}

static const struct behavior_driver_api behavior_bt_layer_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define BT_LAYER_INST(n)                                                       \
    static const struct behavior_bt_layer_config behavior_bt_layer_config_##n = { \
        .layer_profile_1 = DT_INST_PROP(n, layer_profile_1),                   \
        .layer_profile_2 = DT_INST_PROP(n, layer_profile_2),                   \
    };                                                                         \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_bt_layer_init, NULL, NULL,             \
                            &behavior_bt_layer_config_##n,                     \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,  \
                            &behavior_bt_layer_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BT_LAYER_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
