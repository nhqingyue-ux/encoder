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
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

# ── Linker flags ─────────────────────────────────────────────────
set(CMAKE_EXE_LINKER_FLAGS
    "${MCU_FLAGS} -T${LINKER_SCRIPT} -specs=nano.specs -specs=nosys.specs -lc -lm -lgcc \
     -Wl,--gc-sections -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref \
     -Wl,--print-memory-usage"
)

# ── HAL sources ──────────────────────────────────────────────────
file(GLOB HAL_SRCS
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

# ── LVGL sources ─────────────────────────────────────────────────
# Core + draw + font + hal + misc + widgets (base)
file(GLOB_RECURSE LVGL_CORE_SRCS
    "Middlewares/lvgl/src/core/*.c"
    "Middlewares/lvgl/src/draw/*.c"
    "Middlewares/lvgl/src/font/*.c"
    "Middlewares/lvgl/src/hal/*.c"
    "Middlewares/lvgl/src/misc/*.c"
    "Middlewares/lvgl/src/widgets/*.c"
)
# Only the extra modules we actually use
file(GLOB LVGL_EXTRA_SRCS
    "Middlewares/lvgl/src/extra/lv_extra.c"
    "Middlewares/lvgl/src/extra/layouts/flex/lv_flex.c"
    "Middlewares/lvgl/src/extra/layouts/grid/lv_grid.c"
    "Middlewares/lvgl/src/extra/themes/basic/lv_theme_basic.c"
    "Middlewares/lvgl/src/extra/widgets/list/lv_list.c"
    "Middlewares/lvgl/src/extra/widgets/spinner/lv_spinner.c"
)
set(LVGL_SRCS ${LVGL_CORE_SRCS} ${LVGL_EXTRA_SRCS})

# ── Application sources ──────────────────────────────────────────
set(APP_SRCS
    Core/Src/main.c
    Core/Src/stm32g0xx_it.c
    Core/Src/stm32g0xx_hal_msp.c
    Core/Src/system_stm32g0xx.c
    Core/Src/startup_stm32g070xx.s
    Core/Src/syscalls.c
    Core/Src/uart_log.c
    Core/Src/lv_port.c
    Core/Src/ui_menu.c
)

# ── Executable ───────────────────────────────────────────────────
add_executable(${PROJECT_NAME}.elf
    ${APP_SRCS}
    ${HAL_SRCS}
    ${LVGL_SRCS}
)

# ── Precompiled SDK library ───────────────────────────────────────
target_link_libraries(${PROJECT_NAME}.elf
    "${CMAKE_SOURCE_DIR}/Core/Src/SDK_fixed.lib"
)

# ── Include paths ─────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME}.elf PRIVATE
    Core/Inc
    Drivers/STM32G0xx_HAL_Driver/Inc
    Drivers/STM32G0xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32G0xx/Include
    Drivers/CMSIS/Core/Include
    Middlewares                      # for #include "lvgl/lvgl.h"
    Middlewares/lvgl                 # for lv_conf.h lookup
)

# ── Preprocessor defines ─────────────────────────────────────────
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
    STM32G070xx
    USE_HAL_DRIVER
    BURN_IMG42
)

# ── Post-build: generate .bin and .hex ───────────────────────────
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Generating HEX and BIN files"
)
