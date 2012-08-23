################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/driver_firmware_v1_alpha.c \
../src/fifo.c \
../src/general.c \
../src/main.c \
../src/motors.c \
../src/sensors.c \
../src/serialcom.c \
../src/twi_master.c \
../src/usart.c 

OBJS += \
./src/driver_firmware_v1_alpha.o \
./src/fifo.o \
./src/general.o \
./src/main.o \
./src/motors.o \
./src/sensors.o \
./src/serialcom.o \
./src/twi_master.o \
./src/usart.o 

C_DEPS += \
./src/driver_firmware_v1_alpha.d \
./src/fifo.d \
./src/general.d \
./src/main.d \
./src/motors.d \
./src/sensors.d \
./src/serialcom.d \
./src/twi_master.d \
./src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega8535 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


