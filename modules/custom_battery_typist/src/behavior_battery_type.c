/*
 * Battery Level Typing Behavior for ZMK
 *
 * Sends battery levels as keystrokes: "L:50% R:80%"
 * Assumes US keyboard layout on the host OS.
 */

#define DT_DRV_COMPAT zmk_behavior_battery_type

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/battery.h>
#include <zmk/endpoints.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
#include <zmk/split/bluetooth/central.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ----------------------------------------------------------------
 * Configuration: delay between keystrokes (ms)
 * Increase if characters are dropped over BLE.
 * ---------------------------------------------------------------- */
#define KEYSTROKE_DELAY_MS 30

/* ----------------------------------------------------------------
 * Output format definition
 *
 * Change these strings to modify the output format.
 * Example formats:
 *   "L:50% R:80%"       (default)
 *   "Batt -> L50 R80"   (alternative)
 *
 * FORMAT_PREFIX  : printed before left battery value
 * FORMAT_MIDDLE  : printed between left and right battery values
 * FORMAT_SUFFIX  : printed after right battery value
 * FORMAT_NO_DATA : printed when battery data is unavailable
 * ---------------------------------------------------------------- */
#define FORMAT_PREFIX   "L:"
#define FORMAT_MIDDLE   "% R:"
#define FORMAT_SUFFIX   "%"
#define FORMAT_NO_DATA  "--"

/* ----------------------------------------------------------------
 * US layout keycode mapping
 *
 * Each character maps to a HID keycode and whether Shift is needed.
 * ---------------------------------------------------------------- */
struct char_keycode {
    uint32_t keycode;
    bool shift;
};

#include <dt-bindings/zmk/keys.h>

static const struct char_keycode CHAR_MAP[] = {
    ['0'] = { .keycode = HID_USAGE_KEY_KEYBOARD_0_AND_RIGHT_PARENTHESIS, .shift = false },
    ['1'] = { .keycode = HID_USAGE_KEY_KEYBOARD_1_AND_EXCLAMATION_POINT, .shift = false },
    ['2'] = { .keycode = HID_USAGE_KEY_KEYBOARD_2_AND_AT, .shift = false },
    ['3'] = { .keycode = HID_USAGE_KEY_KEYBOARD_3_AND_HASH, .shift = false },
    ['4'] = { .keycode = HID_USAGE_KEY_KEYBOARD_4_AND_DOLLAR, .shift = false },
    ['5'] = { .keycode = HID_USAGE_KEY_KEYBOARD_5_AND_PERCENT, .shift = false },
    ['6'] = { .keycode = HID_USAGE_KEY_KEYBOARD_6_AND_CARET, .shift = false },
    ['7'] = { .keycode = HID_USAGE_KEY_KEYBOARD_7_AND_AMPERSAND, .shift = false },
    ['8'] = { .keycode = HID_USAGE_KEY_KEYBOARD_8_AND_ASTERISK, .shift = false },
    ['9'] = { .keycode = HID_USAGE_KEY_KEYBOARD_9_AND_LEFT_PARENTHESIS, .shift = false },
    ['A'] = { .keycode = HID_USAGE_KEY_KEYBOARD_A, .shift = true },
    ['B'] = { .keycode = HID_USAGE_KEY_KEYBOARD_B, .shift = true },
    ['C'] = { .keycode = HID_USAGE_KEY_KEYBOARD_C, .shift = true },
    ['D'] = { .keycode = HID_USAGE_KEY_KEYBOARD_D, .shift = true },
    ['E'] = { .keycode = HID_USAGE_KEY_KEYBOARD_E, .shift = true },
    ['F'] = { .keycode = HID_USAGE_KEY_KEYBOARD_F, .shift = true },
    ['G'] = { .keycode = HID_USAGE_KEY_KEYBOARD_G, .shift = true },
    ['H'] = { .keycode = HID_USAGE_KEY_KEYBOARD_H, .shift = true },
    ['I'] = { .keycode = HID_USAGE_KEY_KEYBOARD_I, .shift = true },
    ['J'] = { .keycode = HID_USAGE_KEY_KEYBOARD_J, .shift = true },
    ['K'] = { .keycode = HID_USAGE_KEY_KEYBOARD_K, .shift = true },
    ['L'] = { .keycode = HID_USAGE_KEY_KEYBOARD_L, .shift = true },
    ['M'] = { .keycode = HID_USAGE_KEY_KEYBOARD_M, .shift = true },
    ['N'] = { .keycode = HID_USAGE_KEY_KEYBOARD_N, .shift = true },
    ['O'] = { .keycode = HID_USAGE_KEY_KEYBOARD_O, .shift = true },
    ['P'] = { .keycode = HID_USAGE_KEY_KEYBOARD_P, .shift = true },
    ['Q'] = { .keycode = HID_USAGE_KEY_KEYBOARD_Q, .shift = true },
    ['R'] = { .keycode = HID_USAGE_KEY_KEYBOARD_R, .shift = true },
    ['S'] = { .keycode = HID_USAGE_KEY_KEYBOARD_S, .shift = true },
    ['T'] = { .keycode = HID_USAGE_KEY_KEYBOARD_T, .shift = true },
    ['U'] = { .keycode = HID_USAGE_KEY_KEYBOARD_U, .shift = true },
    ['V'] = { .keycode = HID_USAGE_KEY_KEYBOARD_V, .shift = true },
    ['W'] = { .keycode = HID_USAGE_KEY_KEYBOARD_W, .shift = true },
    ['X'] = { .keycode = HID_USAGE_KEY_KEYBOARD_X, .shift = true },
    ['Y'] = { .keycode = HID_USAGE_KEY_KEYBOARD_Y, .shift = true },
    ['Z'] = { .keycode = HID_USAGE_KEY_KEYBOARD_Z, .shift = true },
    ['a'] = { .keycode = HID_USAGE_KEY_KEYBOARD_A, .shift = false },
    ['b'] = { .keycode = HID_USAGE_KEY_KEYBOARD_B, .shift = false },
    ['c'] = { .keycode = HID_USAGE_KEY_KEYBOARD_C, .shift = false },
    ['d'] = { .keycode = HID_USAGE_KEY_KEYBOARD_D, .shift = false },
    ['e'] = { .keycode = HID_USAGE_KEY_KEYBOARD_E, .shift = false },
    ['f'] = { .keycode = HID_USAGE_KEY_KEYBOARD_F, .shift = false },
    ['g'] = { .keycode = HID_USAGE_KEY_KEYBOARD_G, .shift = false },
    ['h'] = { .keycode = HID_USAGE_KEY_KEYBOARD_H, .shift = false },
    ['i'] = { .keycode = HID_USAGE_KEY_KEYBOARD_I, .shift = false },
    ['j'] = { .keycode = HID_USAGE_KEY_KEYBOARD_J, .shift = false },
    ['k'] = { .keycode = HID_USAGE_KEY_KEYBOARD_K, .shift = false },
    ['l'] = { .keycode = HID_USAGE_KEY_KEYBOARD_L, .shift = false },
    ['m'] = { .keycode = HID_USAGE_KEY_KEYBOARD_M, .shift = false },
    ['n'] = { .keycode = HID_USAGE_KEY_KEYBOARD_N, .shift = false },
    ['o'] = { .keycode = HID_USAGE_KEY_KEYBOARD_O, .shift = false },
    ['p'] = { .keycode = HID_USAGE_KEY_KEYBOARD_P, .shift = false },
    ['q'] = { .keycode = HID_USAGE_KEY_KEYBOARD_Q, .shift = false },
    ['r'] = { .keycode = HID_USAGE_KEY_KEYBOARD_R, .shift = false },
    ['s'] = { .keycode = HID_USAGE_KEY_KEYBOARD_S, .shift = false },
    ['t'] = { .keycode = HID_USAGE_KEY_KEYBOARD_T, .shift = false },
    ['u'] = { .keycode = HID_USAGE_KEY_KEYBOARD_U, .shift = false },
    ['v'] = { .keycode = HID_USAGE_KEY_KEYBOARD_V, .shift = false },
    ['w'] = { .keycode = HID_USAGE_KEY_KEYBOARD_W, .shift = false },
    ['x'] = { .keycode = HID_USAGE_KEY_KEYBOARD_X, .shift = false },
    ['y'] = { .keycode = HID_USAGE_KEY_KEYBOARD_Y, .shift = false },
    ['z'] = { .keycode = HID_USAGE_KEY_KEYBOARD_Z, .shift = false },
    [' '] = { .keycode = HID_USAGE_KEY_KEYBOARD_SPACEBAR, .shift = false },
    [':'] = { .keycode = HID_USAGE_KEY_KEYBOARD_SEMICOLON_AND_COLON, .shift = true },
    ['%'] = { .keycode = HID_USAGE_KEY_KEYBOARD_5_AND_PERCENT, .shift = true },
    ['-'] = { .keycode = HID_USAGE_KEY_KEYBOARD_MINUS_AND_UNDERSCORE, .shift = false },
    ['>'] = { .keycode = HID_USAGE_KEY_KEYBOARD_PERIOD_AND_GREATER_THAN, .shift = true },
};

#define CHAR_MAP_SIZE (sizeof(CHAR_MAP) / sizeof(CHAR_MAP[0]))

/* ----------------------------------------------------------------
 * Keystroke sending helpers
 * ---------------------------------------------------------------- */

static const uint32_t HID_LSHIFT = HID_USAGE_KEY_KEYBOARD_LEFT_SHIFT;

static int send_char(char c) {
    if ((uint8_t)c >= CHAR_MAP_SIZE || CHAR_MAP[(uint8_t)c].keycode == 0) {
        LOG_WRN("No keycode mapping for char: 0x%02x", c);
        return -EINVAL;
    }

    const struct char_keycode *mapping = &CHAR_MAP[(uint8_t)c];

    if (mapping->shift) {
        zmk_hid_keyboard_press(HID_LSHIFT);
        zmk_endpoints_send_report(HID_USAGE_KEY);
        k_msleep(KEYSTROKE_DELAY_MS);
    }

    zmk_hid_keyboard_press(mapping->keycode);
    zmk_endpoints_send_report(HID_USAGE_KEY);
    k_msleep(KEYSTROKE_DELAY_MS);

    zmk_hid_keyboard_release(mapping->keycode);
    if (mapping->shift) {
        zmk_hid_keyboard_release(HID_LSHIFT);
    }
    zmk_endpoints_send_report(HID_USAGE_KEY);
    k_msleep(KEYSTROKE_DELAY_MS);

    return 0;
}

static int send_string(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        int ret = send_char(str[i]);
        if (ret < 0) {
            return ret;
        }
    }
    return 0;
}

static int send_number(int value) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", value);
    return send_string(buf);
}

/* ----------------------------------------------------------------
 * Battery level retrieval
 * ---------------------------------------------------------------- */

static int get_central_battery(void) {
    /* zmk_battery_state_of_charge() returns 0-100 or negative on error */
    int soc = zmk_battery_state_of_charge();
    return (soc >= 0) ? soc : -1;
}

static int get_peripheral_battery(void) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
    int soc = zmk_split_get_peripheral_battery_level(0);
    return (soc >= 0) ? soc : -1;
#else
    return -1;
#endif
}

/* ----------------------------------------------------------------
 * Behavior implementation
 * ---------------------------------------------------------------- */

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    int left_bat = get_central_battery();
    int right_bat = get_peripheral_battery();

    /* Send: FORMAT_PREFIX + left_value + FORMAT_MIDDLE + right_value + FORMAT_SUFFIX */
    send_string(FORMAT_PREFIX);

    if (left_bat >= 0) {
        send_number(left_bat);
    } else {
        send_string(FORMAT_NO_DATA);
    }

    send_string(FORMAT_MIDDLE);

    if (right_bat >= 0) {
        send_number(right_bat);
    } else {
        send_string(FORMAT_NO_DATA);
    }

    send_string(FORMAT_SUFFIX);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_battery_type_init(const struct device *dev) {
    return 0;
}

static const struct behavior_driver_api behavior_battery_type_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};

#define BATTERY_TYPE_INST(n)                                                    \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_battery_type_init, NULL, NULL, NULL,    \
                            POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,   \
                            &behavior_battery_type_driver_api);

DT_INST_FOREACH_STATUS_OKAY(BATTERY_TYPE_INST)
