/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_bt_layer

#include <zephyr/device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/ble.h>
#include <zmk/keymap.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

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
        LOG_ERR("Failed to select BT profile %d: %d", profile_index, ret);
        return ret;
    }
    update_layers_for_profile(profile_index);
    LOG_DBG("BT Layer: selected profile %d", profile_index);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_bt_layer_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL,
                        CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_bt_layer_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
