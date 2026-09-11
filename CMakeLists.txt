cmake_minimum_required(VERSION 3.22)
set(FW_DIR "C:/Users/L/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0")
project(stm32-exam C ASM)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/Core/Inc
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
  ${FW_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
  ${FW_DIR}/Drivers/CMSIS/Include
)

add_definitions(-DUSE_HAL_DRIVER -DSTM32F401xE)

file(GLOB APP_SOURCES "${CMAKE_SOURCE_DIR}/Core/Src/*.c")
# Remove can.c temporarily
list(REMOVE_ITEM APP_SOURCES "${CMAKE_SOURCE_DIR}/Core/Src/can.c")
set(STARTUP_SOURCES ${CMAKE_SOURCE_DIR}/Core/Src/startup_stm32f401xe.s)

set(HAL_SOURCES
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
  ${FW_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
)

add_executable(${CMAKE_PROJECT_NAME}
  ${APP_SOURCES}
  ${HAL_SOURCES}
  ${STARTUP_SOURCES}
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
  -T${CMAKE_SOURCE_DIR}/STM32F401RETx_FLASH.ld
  -Wl,--gc-sections
  -Wl,-Map=${CMAKE_PROJECT_NAME}.map
  --specs=nano.specs --specs=nosys.specs
  -Wl,-u,__errno
  -Wl,--print-memory-usage
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex  $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
)