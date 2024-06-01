################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/VXI11/Src/portmap.c \
../Core/VXI11/Src/rpc.c \
../Core/VXI11/Src/vxi11.c \
../Core/VXI11/Src/vxi11_create_link.c \
../Core/VXI11/Src/vxi11_destroy_link.c \
../Core/VXI11/Src/vxi11_device_read.c \
../Core/VXI11/Src/vxi11_device_write.c 

OBJS += \
./Core/VXI11/Src/portmap.o \
./Core/VXI11/Src/rpc.o \
./Core/VXI11/Src/vxi11.o \
./Core/VXI11/Src/vxi11_create_link.o \
./Core/VXI11/Src/vxi11_destroy_link.o \
./Core/VXI11/Src/vxi11_device_read.o \
./Core/VXI11/Src/vxi11_device_write.o 

C_DEPS += \
./Core/VXI11/Src/portmap.d \
./Core/VXI11/Src/rpc.d \
./Core/VXI11/Src/vxi11.d \
./Core/VXI11/Src/vxi11_create_link.d \
./Core/VXI11/Src/vxi11_destroy_link.d \
./Core/VXI11/Src/vxi11_device_read.d \
./Core/VXI11/Src/vxi11_device_write.d 


# Each subdirectory must supply rules for building sources it contributes
Core/VXI11/Src/%.o Core/VXI11/Src/%.su Core/VXI11/Src/%.cyclo: ../Core/VXI11/Src/%.c Core/VXI11/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g -DDEBUG -DDATA_IN_D2_SRAM -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"/home/grzegorz/git/VXI11/SOFTWARE/Core/VXI11/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-VXI11-2f-Src

clean-Core-2f-VXI11-2f-Src:
	-$(RM) ./Core/VXI11/Src/portmap.cyclo ./Core/VXI11/Src/portmap.d ./Core/VXI11/Src/portmap.o ./Core/VXI11/Src/portmap.su ./Core/VXI11/Src/rpc.cyclo ./Core/VXI11/Src/rpc.d ./Core/VXI11/Src/rpc.o ./Core/VXI11/Src/rpc.su ./Core/VXI11/Src/vxi11.cyclo ./Core/VXI11/Src/vxi11.d ./Core/VXI11/Src/vxi11.o ./Core/VXI11/Src/vxi11.su ./Core/VXI11/Src/vxi11_create_link.cyclo ./Core/VXI11/Src/vxi11_create_link.d ./Core/VXI11/Src/vxi11_create_link.o ./Core/VXI11/Src/vxi11_create_link.su ./Core/VXI11/Src/vxi11_destroy_link.cyclo ./Core/VXI11/Src/vxi11_destroy_link.d ./Core/VXI11/Src/vxi11_destroy_link.o ./Core/VXI11/Src/vxi11_destroy_link.su ./Core/VXI11/Src/vxi11_device_read.cyclo ./Core/VXI11/Src/vxi11_device_read.d ./Core/VXI11/Src/vxi11_device_read.o ./Core/VXI11/Src/vxi11_device_read.su ./Core/VXI11/Src/vxi11_device_write.cyclo ./Core/VXI11/Src/vxi11_device_write.d ./Core/VXI11/Src/vxi11_device_write.o ./Core/VXI11/Src/vxi11_device_write.su

.PHONY: clean-Core-2f-VXI11-2f-Src

