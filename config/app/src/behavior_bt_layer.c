#define DT_DRV_COMPAT zmk_behavior_bt_layer

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/ble.h>
#include <zmk/keymap.h>

struct bt_layer_config {
    uint8_t profile1_layer;
    uint8_t profile2_layer;
};

static int update_layers_for_profile(const struct bt_layer_config *cfg, uint8_t profile) {
    const zmk_keymap_layer_id_t layer1 = zmk_keymap_layer_index_to_id(cfg->profile1_layer);
    const zmk_keymap_layer_id_t layer2 = zmk_keymap_layer_index_to_id(cfg->profile2_layer);

    if (layer1 == ZMK_KEYMAP_LAYER_ID_INVAL || layer2 == ZMK_KEYMAP_LAYER_ID_INVAL) {
        LOG_ERR("Invalid layer indexes configured: %u, %u", cfg->profile1_layer,
                cfg->profile2_layer);
        return -EINVAL;
    }

    int err = 0;

    switch (profile) {
    case 1:
        LOG_INF("Profile %u: enable layer %u, disable layer %u", profile, cfg->profile1_layer,
                cfg->profile2_layer);
        err = zmk_keymap_layer_deactivate(layer2);
        if (err) {
            return err;
        }
        return zmk_keymap_layer_activate(layer1);
    case 2:
        LOG_INF("Profile %u: enable layer %u, disable layer %u", profile, cfg->profile2_layer,
                cfg->profile1_layer);
        err = zmk_keymap_layer_deactivate(layer1);
        if (err) {
            return err;
        }
        return zmk_keymap_layer_activate(layer2);
    default:
        LOG_INF("Profile %u: disable layers %u and %u", profile, cfg->profile1_layer,
                cfg->profile2_layer);
        err = zmk_keymap_layer_deactivate(layer1);
        if (err) {
            return err;
        }
        return zmk_keymap_layer_deactivate(layer2);
    }
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);

    if (dev == NULL) {
        LOG_ERR("Binding device lookup failed for %s", binding->behavior_dev);
        return -ENODEV;
    }

    const struct bt_layer_config *cfg = dev->config;
    const uint8_t profile = binding->param1;

    const int ble_err = zmk_ble_prof_select(profile);
    if (ble_err) {
        LOG_ERR("Failed to select BLE profile %u: %d", profile, ble_err);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    const int layer_err = update_layers_for_profile(cfg, profile);
    if (layer_err) {
        LOG_ERR("Failed to update layers for profile %u: %d", profile, layer_err);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_bt_layer_init(const struct device *dev) { return 0; }

static const struct behavior_driver_api behavior_bt_layer_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define BT_LAYER_CONFIG(inst)                                                                      \
    {                                                                                              \
        .profile1_layer = DT_INST_PROP(inst, profile1_layer),                                      \
        .profile2_layer = DT_INST_PROP(inst, profile2_layer),                                      \
    }

#define BT_LAYER_INST(inst)                                                                        \
    static const struct bt_layer_config bt_layer_config_##inst = BT_LAYER_CONFIG(inst);            \
    BEHAVIOR_DT_INST_DEFINE(inst, behavior_bt_layer_init, NULL, NULL, &bt_layer_config_##inst,     \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                      \
                            &behavior_bt_layer_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BT_LAYER_INST)
