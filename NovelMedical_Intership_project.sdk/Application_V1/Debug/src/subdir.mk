################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/lscript.ld 

C_SRCS += \
../src/i2c_access.c \
../src/iic_phyreset.c \
../src/main.c \
../src/oled.c \
../src/platform.c \
../src/platform_mb.c \
../src/platform_zynq.c \
../src/platform_zynqmp.c \
../src/sfp.c \
../src/si5324.c \
../src/tcp_server_func.c \
../src/udp_server_func.c \
../src/user_xadc.c 

OBJS += \
./src/i2c_access.o \
./src/iic_phyreset.o \
./src/main.o \
./src/oled.o \
./src/platform.o \
./src/platform_mb.o \
./src/platform_zynq.o \
./src/platform_zynqmp.o \
./src/sfp.o \
./src/si5324.o \
./src/tcp_server_func.o \
./src/udp_server_func.o \
./src/user_xadc.o 

C_DEPS += \
./src/i2c_access.d \
./src/iic_phyreset.d \
./src/main.d \
./src/oled.d \
./src/platform.d \
./src/platform_mb.d \
./src/platform_zynq.d \
./src/platform_zynqmp.d \
./src/sfp.d \
./src/si5324.d \
./src/tcp_server_func.d \
./src/udp_server_func.d \
./src/user_xadc.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM v7 gcc compiler'
	arm-none-eabi-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -I../../Application_BSP_V1/ps7_cortexa9_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

