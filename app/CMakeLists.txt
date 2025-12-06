# Copyright (c) 2024 The ZMK Contributors
# SPDX-License-Identifier: MIT

if(CONFIG_ZMK_BEHAVIOR_BT_LAYER)
  target_sources(app PRIVATE src/behavior_bt_layer.c)
endif()
