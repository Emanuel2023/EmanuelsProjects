################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/HyperDisplay_ILI9163C.c \
../source/HyperDisplay_KWH018ST01_4WSPI.c \
../source/MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure.c \
../source/compass.c \
../source/fast_hsv2rgb_32bit.c \
../source/fast_hsv2rgb_8bit.c \
../source/hyperdisplay.c \
../source/mtb.c \
../source/semihost_hardfault.c 

C_DEPS += \
./source/HyperDisplay_ILI9163C.d \
./source/HyperDisplay_KWH018ST01_4WSPI.d \
./source/MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure.d \
./source/compass.d \
./source/fast_hsv2rgb_32bit.d \
./source/fast_hsv2rgb_8bit.d \
./source/hyperdisplay.d \
./source/mtb.d \
./source/semihost_hardfault.d 

OBJS += \
./source/HyperDisplay_ILI9163C.o \
./source/HyperDisplay_KWH018ST01_4WSPI.o \
./source/MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure.o \
./source/compass.o \
./source/fast_hsv2rgb_32bit.o \
./source/fast_hsv2rgb_8bit.o \
./source/hyperdisplay.o \
./source/mtb.o \
./source/semihost_hardfault.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MKL25Z128VLK4 -DCPU_MKL25Z128VLK4_cm0plus -DSDK_OS_BAREMETAL -DFSL_RTOS_BM -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure\drivers" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure\CMSIS" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure\utilities" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure\board" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure\source" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure" -I"C:\Users\KC\Documents\MCUXpressoIDE_11.9.0_2144\workspace\MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure\startup" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/HyperDisplay_ILI9163C.d ./source/HyperDisplay_ILI9163C.o ./source/HyperDisplay_KWH018ST01_4WSPI.d ./source/HyperDisplay_KWH018ST01_4WSPI.o ./source/MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure.d ./source/MKL25Z4_Project_SparkfunTFTHyperDisplayConfigure.o ./source/compass.d ./source/compass.o ./source/fast_hsv2rgb_32bit.d ./source/fast_hsv2rgb_32bit.o ./source/fast_hsv2rgb_8bit.d ./source/fast_hsv2rgb_8bit.o ./source/hyperdisplay.d ./source/hyperdisplay.o ./source/mtb.d ./source/mtb.o ./source/semihost_hardfault.d ./source/semihost_hardfault.o

.PHONY: clean-source

