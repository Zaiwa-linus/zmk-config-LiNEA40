/*
 * LiNEA40 BT profile + base layer select behavior
 *
 * Selects a BLE profile and deactivates host layers 1..4.
 */

#define DT_DRV_COMPAT zmk_behavior_bt_base

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/ble.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define LINEA40_BT_BASE_FIRST_PROFILE     0
#define LINEA40_BT_BASE_LAST_PROFILE      4
#define LINEA40_BT_BASE_FIRST_HOST_LAYER  1
#define LINEA40_BT_BASE_HOST_LAYER_COUNT  4

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata profile_values[] = {
    {
        .display_name = "BT0 -> Layer 0",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = 0,
    },
    {
        .display_name = "BT1 -> Layer 0",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = 1,
    },
    {
        .display_name = "BT2 -> Layer 0",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = 2,
    },
    {
        .display_name = "BT3 -> Layer 0",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = 3,
    },
    {
        .display_name = "BT4 -> Layer 0",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
        .value = 4,
    },
};

static const struct behavior_parameter_metadata_set metadata_sets[] = {{
    .param1_values = profile_values,
    .param1_values_len = ARRAY_SIZE(profile_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(metadata_sets),
    .sets = metadata_sets,
};

#endif

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static void deactivate_host_layers(void) {
    for (uint8_t i = 0; i < LINEA40_BT_BASE_HOST_LAYER_COUNT; i++) {
        zmk_keymap_layer_deactivate(LINEA40_BT_BASE_FIRST_HOST_LAYER + i);
    }
}
#endif

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    int32_t profile = binding->param1;

    if (profile < LINEA40_BT_BASE_FIRST_PROFILE ||
        profile > LINEA40_BT_BASE_LAST_PROFILE) {
        LOG_ERR("Unsupported BT base profile: %d", profile);
        return -ERANGE;
    }

    int ret = zmk_ble_prof_select((uint8_t)profile);
    if (ret < 0) {
        return ret;
    }

    deactivate_host_layers();
#endif
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_bt_base_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

BEHAVIOR_DT_INST_DEFINE(0, NULL, NULL, NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                        &behavior_bt_base_driver_api);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
