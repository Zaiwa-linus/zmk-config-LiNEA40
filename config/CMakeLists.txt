# LiNEA40 custom behaviors.
#
# Each source guards itself with DT_HAS_COMPAT_STATUS_OKAY / DT_INST_FOREACH_STATUS_OKAY,
# so listing them unconditionally is safe for builds that do not use them
# (settings_reset, and the peripheral half).

target_sources(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/boards/shields/LiNEA40/src/behavior_battery_display.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/boards/shields/LiNEA40/src/behavior_bt_layer.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/boards/shields/LiNEA40/src/behavior_bt_base.c)
