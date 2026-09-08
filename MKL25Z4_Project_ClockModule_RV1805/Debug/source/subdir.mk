################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/MKL25Z4_Project_RV1805clockmodule.c \
../source/mtb.c \
../source/semihost_hardfault.c 

C_DEPS += \
./source/MKL25Z4_Project_RV1805clockmodule.d \
./source/mtb.d \
./source/semihost_hardfault.d 

OBJS += \
./source/MKL25Z4_Project_RV1805clockmodule.o \
./source/mtb.o \
./source/semihost_hardfault.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_RV1805clockmodule\board" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_RV1805clockmodule\source" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_RV1805clockmodule" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_RV1805clockmodule\drivers" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_RV1805clockmodule\startup" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_RV1805clockmodule\utilities" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_RV1805clockmodule\CMSIS" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/MKL25Z4_Project_RV1805clockmodule.d ./source/MKL25Z4_Project_RV1805clockmodule.o ./source/mtb.d ./source/mtb.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o

.PHONY: clean-source

