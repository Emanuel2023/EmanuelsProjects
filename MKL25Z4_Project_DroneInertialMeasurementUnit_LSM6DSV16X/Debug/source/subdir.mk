################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/IMU.c \
../source/lsm6dsv16x_reg.c \
../source/mtb.c \
../source/semihost_hardfault.c \
../source/sfe_lsm6dsv16x.c \
../source/sfe_lsm_shim.c 

C_DEPS += \
./source/IMU.d \
./source/lsm6dsv16x_reg.d \
./source/mtb.d \
./source/semihost_hardfault.d \
./source/sfe_lsm6dsv16x.d \
./source/sfe_lsm_shim.d 

OBJS += \
./source/IMU.o \
./source/lsm6dsv16x_reg.o \
./source/mtb.o \
./source/semihost_hardfault.o \
./source/sfe_lsm6dsv16x.o \
./source/sfe_lsm_shim.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_LSM6DSV16X\board" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_LSM6DSV16X\source" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_LSM6DSV16X" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_LSM6DSV16X\drivers" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_LSM6DSV16X\startup" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_LSM6DSV16X\utilities" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_LSM6DSV16X\CMSIS" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/IMU.d ./source/IMU.o ./source/lsm6dsv16x_reg.d ./source/lsm6dsv16x_reg.o ./source/mtb.d ./source/mtb.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o ./source/sfe_lsm6dsv16x.d ./source/sfe_lsm6dsv16x.o ./source/sfe_lsm_shim.d ./source/sfe_lsm_shim.o

.PHONY: clean-source

