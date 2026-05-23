# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This repository manages QMK and Vial keyboard firmware configurations for multiple split keyboards (crkbd, cornelius, lskbd). The upstream QMK and Vial-QMK sources are git submodules; this repo stores only the custom keyboard definitions and keymaps, which are injected into the submodule trees at build time.

## Prerequisites

- QMK CLI (`qmk`) installed and environment set up per https://docs.qmk.fm/#/newbs_getting_started
- Submodules initialized: `make git-submodule`

## Build Commands

All make targets accept three variables: `kb` (keyboard), `kr` (revision), `km` (keymap).

### VIA firmware

```sh
make qmk-clean
kb=crkbd make qmk-init
kb=crkbd kr=rev4_1/standard km=via make qmk-compile
```

Output: `keyboards/crkbd/qmk/qmk_firmware/.build/crkbd_rev4_1_standard_via.uf2`

### Vial firmware

```sh
make vial-qmk-clean
kb=crkbd make vial-qmk-init
kb=crkbd kr=rev4_1/standard km=vial make vial-qmk-compile
```

Output: `keyboards/crkbd/vial-kb/vial-qmk/.build/crkbd_rev4_1_standard_vial.uf2`

### Flash to keyboard

```sh
kb=crkbd kr=rev4_1/standard km=via make qmk-flash
```

### Rebuild everything

```sh
make update-all
```

## Architecture

### Directory layout

```
keyboards/<kb>/
  qmk/qmk_firmware/        # QMK keyboard definition (config.h, keyboard.json, keymaps/)
  vial-kb/vial-qmk/        # Vial keymaps only (keymaps/ with vial.json)
  the-via/                  # VIA definition JSON files (loaded in VIA app)
src/
  qmk/qmk_firmware/        # Submodule: upstream QMK
  vial-kb/vial-qmk/        # Submodule: upstream Vial-QMK
```

### How build injection works

- **QMK (`qmk-init`)**: creates a symlink `src/qmk/qmk_firmware/keyboards/tmp/<kb>` → `keyboards/<kb>/qmk/qmk_firmware`, then compiles as `tmp/<kb>/<kr>` with keymap `<km>`.
- **Vial (`vial-qmk-init`)**: copies `keyboards/<kb>/qmk/qmk_firmware/*` into `src/vial-kb/vial-qmk/keyboards/tmp/<kb>`, removes the keymaps directory, and symlinks `keyboards/<kb>/vial-kb/vial-qmk/keymaps` in its place. This lets Vial keymaps live separately while sharing the base hardware definition.

Built artifacts are copied to `keyboards/<kb>/qmk/qmk_firmware/.build/` or `keyboards/<kb>/vial-kb/vial-qmk/.build/` with filenames of the form `<kb>_<kr>_<km>.(hex|uf2)`.

### Supported keyboards and revisions

| Keyboard   | Revisions                                    | Keymaps           |
|------------|----------------------------------------------|-------------------|
| crkbd      | rev1, rev4_0/standard, rev4_0/mini, rev4_1/standard, rev4_1/mini | via, vial, vial_mini |
| cornelius  | rev1, rev2                                   | via, vial         |
| lskbd      | rev1                                         | via, vial         |

### crkbd rev4_1 hardware notes

- MCU: RP2040 (Raspberry Pi Pico), bootloader: `rp2040` (double-tap reset to enter bootloader)
- Split communication: USART serial on GP12
- RGB matrix: 46 LEDs (23 per half) via WS2812 on GP10, max brightness 50
- OLED, encoder, mousekey, NKRO, extrakey all enabled
- Keymap uses `LAYOUT_split_3x6_3_ex2` (with extra column keys) when `LAYOUT_split_3x6_3_ex2` is defined; falls back to `LAYOUT_split_3x6_3` otherwise

### Keymap structure

Keymaps for VIA/Vial are in `keyboards/<kb>/qmk/qmk_firmware/keymaps/via/keymap.c` and `keyboards/<kb>/vial-kb/vial-qmk/keymaps/vial/keymap.c`. Both use 4 layers (0–3):
- Layer 0: base (QWERTY)
- Layer 1: lower (numbers, arrows)
- Layer 2: raise (symbols)
- Layer 3: adjust (QK_BOOT, RGB controls)

Vial keymaps additionally include `vial.json` (Vial UI layout definition) and `config.h`/`rules.mk`.

### Vial GUI labels vs QMK API mapping

The tap-hold toggles in Vial GUI's QMK Settings tab use QMK's old API names (hardcoded in vial-kb/vial-gui's `src/main/resources/base/qmk_settings.json`). Vial-QMK firmware has migrated to the new API, so the mapping is:

| Vial GUI label            | QSID/bit    | Internal function              | Effect when checked                                  | QMK API                                   |
|---------------------------|-------------|--------------------------------|------------------------------------------------------|-------------------------------------------|
| Permissive Hold           | QSID=8 bit0 | `get_permissive_hold`          | Permissive Hold ON                                   | `PERMISSIVE_HOLD`                         |
| Ignore Mod Tap Interrupt  | QSID=8 bit1 | `get_hold_on_other_key_press`  | `HOLD_ON_OTHER_KEY_PRESS` **OFF** (inverted)         | opposite of `HOLD_ON_OTHER_KEY_PRESS`     |
| Tapping Force Hold        | QSID=8 bit2 | `get_quick_tap_term`           | `QUICK_TAP_TERM` set to 0 (suppresses repeat-tap hold) | `QUICK_TAP_TERM`                        |
| Retro Tapping             | QSID=8 bit3 | `get_retro_tapping`            | Retro Tapping ON                                     | `RETRO_TAPPING`                           |

Note: "Ignore Mod Tap Interrupt" is inverted — leaving it **unchecked** (default) is equivalent to `HOLD_ON_OTHER_KEY_PRESS` being enabled.

Reference: `src/vial-kb/vial-qmk/quantum/qmk_settings.c` (around line 263–281)

## CI

Two GitHub Actions workflows run on every push, each uploading the `.uf2` as a workflow artifact:

- `build-via.yml`: builds `crkbd rev4_1/standard via`
- `build-vial.yml`: builds `crkbd rev4_1/standard vial`
