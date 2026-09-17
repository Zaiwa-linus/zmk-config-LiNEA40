/*
 * LiNEA40 battery display behavior
 *
 * On key press, types the battery level of both halves as HID keystrokes:
 *   "L:XX% R:XX%"
 * where L = left (peripheral, source 0) and R = right (central, local).
 *
 * A 30 ms inter-key delay is added to avoid BLE character drop.
 * Only the central half produces output; on the peripheral the press is a no-op.
 */

#define DT_DRV_COMPAT zmk_behavior_battery_display

#include <stdio.h>
#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/battery.h>
#include <dt-bindings/zmk/keys.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/bluetooth/central.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

/* Milliseconds the key is held before release. */
#define TAP_MS  10
/* Milliseconds to wait after each key release before the next press. */
#define WAIT_MS 30

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

static uint32_t char_to_keycode(char c) {
    switch (c) {
    case '0': return NUMBER_0;
    case '1': return NUMBER_1;
    case '2': return NUMBER_2;
    case '3': return NUMBER_3;
    case '4': return NUMBER_4;
    case '5': return NUMBER_5;
    case '6': return NUMBER_6;
    case '7': return NUMBER_7;
    case '8': return NUMBER_8;
    case '9': return NUMBER_9;
    case 'L': return LS(L);
    case 'R': return LS(R);
    case ':': return COLON;
    case '%': return PERCENT;
    case ' ': return SPACE;
    default:  return 0;
    }
}

static void queue_str(const struct zmk_behavior_binding_event *event, const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        uint32_t keycode = char_to_keycode(str[i]);
        if (keycode == 0) {
            continue;
        }
        struct zmk_behavior_binding kp_binding = {
            .behavior_dev = "key_press",
            .param1 = keycode,
            .param2 = 0,
        };
        zmk_behavior_queue_add(event, kp_binding, true, TAP_MS);
        zmk_behavior_queue_add(event, kp_binding, false, WAIT_MS);
    }
}

#endif /* CONFIG_ZMK_SPLIT_ROLE_CENTRAL */

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    uint8_t left_pct = 0;
    /* central (right) = local battery */
    uint8_t right_pct = zmk_battery_state_of_charge();

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
    /* peripheral (left) = source index 0 */
    zmk_split_get_peripheral_battery_level(0, &left_pct);
#endif

    char buf[32];
    /* %% in format string produces a literal '%' in output */
    snprintf(buf, sizeof(buf), "L:%u%% R:%u%%", (unsigned)left_pct, (unsigned)right_pct);
    LOG_DBG("Battery: %s", buf);
    queue_str(&event, buf);
#endif /* CONFIG_ZMK_SPLIT_ROLE_CENTRAL */
    return 0;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return 0;
}

static const struct behavior_driver_api behavior_battery_display_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define BATT_DISP_INST(n)                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_battery_display_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BATT_DISP_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
