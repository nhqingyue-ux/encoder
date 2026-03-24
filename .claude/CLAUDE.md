# CLAUDE.md - Encoder Controller Project

## What is this?

A rotary encoder input device based on STM32G070xx (Cortex-M0+, 128KB Flash, 36KB RAM)
with a round 240×240 SPI LCD. It acts as an input peripheral for a host controller,
sending directional and button events via UART.

## Hardware

| Component | Details |
|-----------|---------|
| MCU | STM32G070xx (Cortex-M0+, 64MHz, 128KB Flash, 36KB RAM) |
| Display | 240×240 round SPI LCD (SPI1) |
| External Flash | W25Qxx (SPI2) — stores image assets (legacy, not used by LVGL) |
| Input | Rotary encoder + push button (SW on PF1) |
| Communication | USART3 (PB8=TX, PB9=RX, 115200 8N1) → Host |
| Debug | Same USART3 (debug logs in debug builds only) |

## Directory Structure

```
encoder/
├── App/                     ← Application layer
│   ├── Inc/ui_menu.h
│   └── Src/
│       ├── main.c           ← Entry point + LVGL main loop (~110 lines)
│       └── ui_menu.c        ← Round display UI (R/L/Y indicator)
│
├── Port/                    ← Middleware adaptation layer
│   ├── Inc/
│   │   ├── lv_port.h
│   │   └── comm.h
│   └── Src/
│       ├── lv_port.c        ← LVGL display flush + encoder input driver
│       ├── comm.c           ← USART3 init + protocol TX (R\n, L\n, Y\n)
│       ├── uart_log.c       ← Debug printf (compiled out in release)
│       └── stm32g0xx_hal_msp.c
│
├── BSP/                     ← Board support package
│   ├── Inc/                 ← main.h, SDK.h, hal_conf.h, it.h
│   ├── Src/                 ← startup, system, interrupts, syscalls
│   └── Lib/SDK_fixed.lib    ← Vendor SDK (precompiled, objcopy-fixed)
│
├── Drivers/                 ← ST HAL + CMSIS (standard)
├── Middlewares/lvgl/         ← LVGL v8.3.11
├── docs/protocol.md         ← Communication protocol spec
├── cmake/arm-none-eabi.cmake ← Toolchain file
└── CMakeLists.txt
```

## Build

**Toolchain**: ARM GNU Toolchain 14.2 (official, not Homebrew)
- Path: `~/Applications/arm-gnu-14.2/bin/arm-none-eabi-gcc`

**Commands**:
```bash
# Configure + build release (production)
cmake --preset release
cmake --build build/release

# Configure + build debug (with LOG_Printf output)
cmake --preset debug
cmake --build build/debug

# Flash
st-flash erase
st-flash write build/release/SDKExample.bin 0x08000000

# Read serial (debug builds)
python3 -c "import serial; s=serial.Serial('/dev/cu.usbserial-21210',115200,timeout=3); print(s.read(2048))"
```

## Key Design Decisions

1. **SDK.lib**: Vendor precompiled static library (ARM elf32-littlearm). Original had
   ARMCC symtab issues; `objcopy` was used to create `SDK_fixed.lib` for GCC compatibility.

2. **LVGL v8.3.11**: Chosen for round display UI. Config: 12KB heap (`LV_MEM_SIZE`),
   `LV_USE_ARC=1`, `LV_FONT_DEFAULT=unscii_8`. Display flush uses SDK's `LCD_ShowImg`.

3. **Shared USART3**: Single exposed UART carries both protocol messages and debug logs.
   Release builds (`NDEBUG`) compile out all `LOG_Printf` calls — UART is protocol-only.

4. **Release uses `-Os`**: `-O2` overflows 128KB Flash due to inlining. `-Os` → 78% usage.

## Communication Protocol

Single ASCII char + `\n` (one-way: encoder → host):

| Message | Meaning |
|---------|---------|
| `R\n`   | Clockwise rotation (Right) |
| `L\n`   | Counter-clockwise rotation (Left) |
| `Y\n`   | Button press (Yes) |

Full spec: `docs/protocol.md`

## Resource Budget

|         | Release (-Os) | Debug (-Og) |
|---------|---------------|-------------|
| Flash   | 102KB / 128KB (78%) | 123KB / 128KB (94%) |
| RAM     | 20KB / 36KB (56%)   | 21KB / 36KB (57%)   |

## Gotchas / Pitfalls

- **`LV_ASSERT_HANDLER` defaults to `while(1)`** — if LVGL runs out of heap,
  the MCU silently hangs. Increase `LV_MEM_SIZE` if adding UI objects.
- **Round display**: All UI elements must stay within r≈110px of center (120,120).
  Corners of the 240×240 framebuffer are not visible.
- **`-O2` overflows Flash**: Always use `-Os` for release.
- **SDK.lib needs objcopy**: The original `SDK.lib` has ARMCC-specific section headers.
  Use `SDK_fixed.lib` in `BSP/Lib/`.
- **No newlib**: Toolchain uses `-specs=nano.specs -specs=nosys.specs`. The `syscalls.c`
  provides stubs. Don't use `malloc`/`printf` directly — use LVGL's `lv_mem_*` and `LOG_Printf`.

## Git

- Branch: `main`
- User: `kiki <kiki@encoder>`
- `.gitignore`: `build/`, `.DS_Store`, `*.orig`, `Core/Src/SDK.lib`
