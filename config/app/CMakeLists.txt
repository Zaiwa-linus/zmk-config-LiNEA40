if(CONFIG_ZMK_BEHAVIOR_BT_LAYER)
  zephyr_library()
  zephyr_library_sources(src/behavior_bt_layer.c)
endif()
