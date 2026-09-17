CONFIG_DIR = config

SRCS_LEFT = $(shell find $(CONFIG_DIR) -type f ! -name "*_right*")
SRCS_RIGHT = $(shell find $(CONFIG_DIR) -type f ! -name "*_left*")

TARGET_LEFT = ../zmk/app/build/left/zephyr/zmk.uf2
TARGET_RIGHT = ../zmk/app/build/right/zephyr/zmk.uf2

.PHONY: build clean

build: $(TARGET_LEFT) $(TARGET_RIGHT)

#build: $(TARGET_RIGHT)

$(TARGET_LEFT): $(SRCS_LEFT)
	docker exec -w /workspaces/zmk/app -it $(container_name) west build -d build/left -b seeeduino_xiao_ble -- -DSHIELD="LiNEA40_left rgbled_adapter" -DZMK_CONFIG="/workspaces/zmk-config/config" -DZMK_EXTRA_MODULES="/workspaces/zmk-modules/zmk-pmw3610-driver;/workspaces/zmk-modules/zmk-rgbled-widget;/workspaces/zmk-config/config"
	docker exec -w /workspaces/zmk/app -it $(container_name) cp build/left/zephyr/zmk.uf2 build/LiNEA40_left.uf2

$(TARGET_RIGHT): $(SRCS_RIGHT)
	docker exec -w /workspaces/zmk/app -it $(container_name) west build -d build/right -b seeeduino_xiao_ble -S studio-rpc-usb-uart -S zmk-usb-logging -- -DSHIELD="LiNEA40_right rgbled_adapter" -DZMK_CONFIG="/workspaces/zmk-config/config" -DZMK_EXTRA_MODULES="/workspaces/zmk-modules/zmk-pmw3610-driver;/workspaces/zmk-modules/zmk-rgbled-widget;/workspaces/zmk-config/config" -DCONFIG_ZMK_STUDIO=y -DCONFIG_ZMK_STUDIO_LOCKING=n
	docker exec -w /workspaces/zmk/app -it $(container_name) cp build/right/zephyr/zmk.uf2 build/LiNEA40_right.uf2

clean:
	docker exec -it $(container_name) rm -rf /workspaces/zmk/app/build

# ---- キーマップ画像の生成 -------------------------------------------------
# keymap-drawer で config/LiNEA40.keymap を SVG 化する。
# tree-sitter は 0.24.0 に固定する必要がある（README 参照）。
KEYMAP_VENV = .venv-keymap
KEYMAP_SVG  = document/img/LiNEA40-keymap.svg
KEYMAP_LAYERS = default mac ios MOUSE MARK CURSOR_win CURSOR_mac CURSOR_ios FUNCTION

.PHONY: keymap-svg

$(KEYMAP_VENV)/bin/keymap:
	python3 -m venv $(KEYMAP_VENV)
	$(KEYMAP_VENV)/bin/pip install --quiet --upgrade pip
	$(KEYMAP_VENV)/bin/pip install --quiet keymap-drawer
	$(KEYMAP_VENV)/bin/pip install --quiet "tree-sitter==0.24.0" "tree-sitter-devicetree==0.14.1"

keymap-svg: $(KEYMAP_VENV)/bin/keymap
	@mkdir -p $(dir $(KEYMAP_SVG))
	$(KEYMAP_VENV)/bin/keymap -c keymap_drawer.config.yaml parse -z config/LiNEA40.keymap > $(KEYMAP_VENV)/parsed.yaml
	$(KEYMAP_VENV)/bin/keymap -c keymap_drawer.config.yaml draw \
	    -d config/boards/shields/LiNEA40/LiNEA40.dtsi \
	    -o $(KEYMAP_SVG) \
	    $(KEYMAP_VENV)/parsed.yaml \
	    -s $(KEYMAP_LAYERS)
	@echo "Wrote $(KEYMAP_SVG)"
