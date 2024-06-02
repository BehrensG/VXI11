################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/SCPI/libscpi/src/error.c \
../Core/SCPI/libscpi/src/expression.c \
../Core/SCPI/libscpi/src/fifo.c \
../Core/SCPI/libscpi/src/ieee488.c \
../Core/SCPI/libscpi/src/lexer.c \
../Core/SCPI/libscpi/src/minimal.c \
../Core/SCPI/libscpi/src/parser.c \
../Core/SCPI/libscpi/src/units.c \
../Core/SCPI/libscpi/src/utils.c 

OBJS += \
./Core/SCPI/libscpi/src/error.o \
./Core/SCPI/libscpi/src/expression.o \
./Core/SCPI/libscpi/src/fifo.o \
./Core/SCPI/libscpi/src/ieee488.o \
./Core/SCPI/libscpi/src/lexer.o \
./Core/SCPI/libscpi/src/minimal.o \
./Core/SCPI/libscpi/src/parser.o \
./Core/SCPI/libscpi/src/units.o \
./Core/SCPI/libscpi/src/utils.o 

C_DEPS += \
./Core/SCPI/libscpi/src/error.d \
./Core/SCPI/libscpi/src/expression.d \
./Core/SCPI/libscpi/src/fifo.d \
./Core/SCPI/libscpi/src/ieee488.d \
./Core/SCPI/libscpi/src/lexer.d \
./Core/SCPI/libscpi/src/minimal.d \
./Core/SCPI/libscpi/src/parser.d \
./Core/SCPI/libscpi/src/units.d \
./Core/SCPI/libscpi/src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
Core/SCPI/libscpi/src/%.o Core/SCPI/libscpi/src/%.su Core/SCPI/libscpi/src/%.cyclo: ../Core/SCPI/libscpi/src/%.c Core/SCPI/libscpi/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DDATA_IN_D2_SRAM -DSTM32H743xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"/home/grzegorz/git/VXI11/SOFTWARE/Core/SCPI/libscpi/inc/scpi" -I"/home/grzegorz/git/VXI11/SOFTWARE/Core/SCPI/libscpi/inc" -I"/home/grzegorz/git/VXI11/SOFTWARE/Core/VXI11/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -Ofast -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-SCPI-2f-libscpi-2f-src

clean-Core-2f-SCPI-2f-libscpi-2f-src:
	-$(RM) ./Core/SCPI/libscpi/src/error.cyclo ./Core/SCPI/libscpi/src/error.d ./Core/SCPI/libscpi/src/error.o ./Core/SCPI/libscpi/src/error.su ./Core/SCPI/libscpi/src/expression.cyclo ./Core/SCPI/libscpi/src/expression.d ./Core/SCPI/libscpi/src/expression.o ./Core/SCPI/libscpi/src/expression.su ./Core/SCPI/libscpi/src/fifo.cyclo ./Core/SCPI/libscpi/src/fifo.d ./Core/SCPI/libscpi/src/fifo.o ./Core/SCPI/libscpi/src/fifo.su ./Core/SCPI/libscpi/src/ieee488.cyclo ./Core/SCPI/libscpi/src/ieee488.d ./Core/SCPI/libscpi/src/ieee488.o ./Core/SCPI/libscpi/src/ieee488.su ./Core/SCPI/libscpi/src/lexer.cyclo ./Core/SCPI/libscpi/src/lexer.d ./Core/SCPI/libscpi/src/lexer.o ./Core/SCPI/libscpi/src/lexer.su ./Core/SCPI/libscpi/src/minimal.cyclo ./Core/SCPI/libscpi/src/minimal.d ./Core/SCPI/libscpi/src/minimal.o ./Core/SCPI/libscpi/src/minimal.su ./Core/SCPI/libscpi/src/parser.cyclo ./Core/SCPI/libscpi/src/parser.d ./Core/SCPI/libscpi/src/parser.o ./Core/SCPI/libscpi/src/parser.su ./Core/SCPI/libscpi/src/units.cyclo ./Core/SCPI/libscpi/src/units.d ./Core/SCPI/libscpi/src/units.o ./Core/SCPI/libscpi/src/units.su ./Core/SCPI/libscpi/src/utils.cyclo ./Core/SCPI/libscpi/src/utils.d ./Core/SCPI/libscpi/src/utils.o ./Core/SCPI/libscpi/src/utils.su

.PHONY: clean-Core-2f-SCPI-2f-libscpi-2f-src

