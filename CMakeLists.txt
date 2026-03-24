cmake_minimum_required(VERSION 3.20)

project(SDKExample C ASM)

# ── Target / MCU config ──────────────────────────────────────────
set(MCU_FLAGS "-mcpu=cortex-m0plus -mthumb")
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32G0xx/Source/Templates/gcc/STM32G070xx_FLASH.ld")

# ── Compile flags ────────────────────────────────────────────────
set(COMMON_FLAGS "${MCU_FLAGS} -fdata-sections -ffunction-sections -Wall")

set(CMAKE_C_FLAGS   "${COMMON_FLAGS} -std=gnu11")
set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -x assembler-with-cpp")

set(CMAKE_C_FLAGS_DEBUG   "-Og -g3 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG")

# ── Linker flags ─────────────────────────────────────────────────
set(CMAKE_EXE_LINKER_FLAGS
    "${MCU_FLAGS} -T${LINKER_SCRIPT} \
     -specs=nano.specs -specs=nosys.specs -lc -lm -lgcc \
     -Wl,--gc-sections -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map,--cref \
     -Wl,--print-memory-usage"
)

# ── Include directories ─────────────────────────────────────────
include_directories(
    # Application
    "App/Inc"
    # Port / Middleware adaptation
    "Port/Inc"
    # BSP (board support)
    "BSP/Inc"
    # HAL
    "Drivers/STM32G0xx_HAL_Driver/Inc"
    "Drivers/CMSIS/Include"
    "Drivers/CMSIS/Core/Include"
    "Drivers/CMSIS/Device/ST/STM32G0xx/Include"
    # LVGL
    "Middlewares"
    "Middlewares/lvgl"
)

# ── Compile definitions ─────────────────────────────────────────
add_definitions(
    -DUSE_HAL_DRIVER
    -DSTM32G070xx
)

# ── Source files ─────────────────────────────────────────────────

# App layer
set(APP_SOURCES
    "App/Src/main.c"
    "App/Src/ui_menu.c"
)

# Port layer (middleware adaptation + debug)
set(PORT_SOURCES
    "Port/Src/lv_port.c"
    "Port/Src/uart_log.c"
    "Port/Src/comm.c"
    "Port/Src/stm32g0xx_hal_msp.c"
)

# BSP layer (startup, system, interrupts, syscalls)
set(BSP_SOURCES
    "BSP/Src/startup_stm32g070xx.s"
    "BSP/Src/system_stm32g0xx.c"
    "BSP/Src/stm32g0xx_it.c"
    "BSP/Src/syscalls.c"
)

# HAL drivers
set(HAL_SOURCES
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_cortex.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_dma.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_dma_ex.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_exti.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash_ex.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_gpio.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_pwr.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_pwr_ex.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_rcc.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_rcc_ex.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_spi.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_spi_ex.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_tim.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_tim_ex.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_uart.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_uart_ex.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_usart.c"
    "Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_usart_ex.c"
)

# LVGL sources (v8.3.11) — core modules only, no extra widgets
file(GLOB_RECURSE LVGL_CORE_SRC    "Middlewares/lvgl/src/core/*.c")
file(GLOB_RECURSE LVGL_DRAW_SRC    "Middlewares/lvgl/src/draw/*.c")
file(GLOB_RECURSE LVGL_FONT_SRC    "Middlewares/lvgl/src/font/*.c")
file(GLOB_RECURSE LVGL_HAL_SRC     "Middlewares/lvgl/src/hal/*.c")
file(GLOB_RECURSE LVGL_MISC_SRC    "Middlewares/lvgl/src/misc/*.c")
file(GLOB_RECURSE LVGL_WIDGET_SRC  "Middlewares/lvgl/src/widgets/*.c")
file(GLOB_RECURSE LVGL_EXTRA_SRC   "Middlewares/lvgl/src/extra/*.c")

# Exclude extra widgets that depend on disabled features (LV_USE_DROPDOWN=0, etc.)
list(FILTER LVGL_EXTRA_SRC EXCLUDE REGEX "extra/widgets/(calendar|menu|meter|msgbox|span|spinbox|tabview|tileview|win)/")

set(LVGL_SOURCES
    ${LVGL_CORE_SRC}
    ${LVGL_DRAW_SRC}
    ${LVGL_FONT_SRC}
    ${LVGL_HAL_SRC}
    ${LVGL_MISC_SRC}
    ${LVGL_WIDGET_SRC}
    ${LVGL_EXTRA_SRC}
)

# ── Executable ───────────────────────────────────────────────────
add_executable(${PROJECT_NAME}.elf
    ${APP_SOURCES}
    ${PORT_SOURCES}
    ${BSP_SOURCES}
    ${HAL_SOURCES}
    ${LVGL_SOURCES}
)

# ── Link SDK library ────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME}.elf
    "${CMAKE_SOURCE_DIR}/BSP/Lib/SDK_fixed.lib"
)

# ── Post-build: generate HEX and BIN ────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Generating HEX and BIN files"
)
