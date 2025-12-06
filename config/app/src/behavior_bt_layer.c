#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/ble.h>
#include <zmk/layers.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_bt_layer_config {};

struct behavior_bt_layer_data {};

static void update_layers_for_profile(uint8_t profile) {
    LOG_INF("Updating layers for profile %d", profile);

    switch (profile) {
    case 1:
        zmk_layer_activate(1);
        zmk_layer_deactivate(2);
        break;
    case 2:
        zmk_layer_activate(2);
        zmk_layer_deactivate(1);
        break;
    default:
        zmk_layer_deactivate(1);
        zmk_layer_deactivate(2);
        break;
    }
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT) && !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    return ZMK_BEHAVIOR_OPAQUE;
#endif

    const uint8_t profile = binding->param1;

    int err = zmk_ble_prof_select(profile);
    if (err) {
        LOG_ERR("Failed to select profile %d: %d", profile, err);
        return err;
    }

    update_layers_for_profile(profile);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_bt_layer_init(const struct device *dev) {
    ARG_UNUSED(dev);
    return 0;
}

static const struct behavior_driver_api behavior_bt_layer_driver_api = {
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define BT_LAYER_INST(n)                                                                          \
    static struct behavior_bt_layer_data behavior_bt_layer_data_##n;                              \
    static const struct behavior_bt_layer_config behavior_bt_layer_config_##n = {};               \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_bt_layer_init, NULL, &behavior_bt_layer_data_##n,         \
                            &behavior_bt_layer_config_##n, APPLICATION,                           \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_bt_layer_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BT_LAYER_INST)
