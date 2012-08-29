################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/cansic_engine.c \
../source/diccionario.c \
../source/diccionario_buddy.c \
../source/diccionario_dinamicas.c 

OBJS += \
./source/cansic_engine.o \
./source/diccionario.o \
./source/diccionario_buddy.o \
./source/diccionario_dinamicas.o 

C_DEPS += \
./source/cansic_engine.d \
./source/diccionario.d \
./source/diccionario_buddy.d \
./source/diccionario_dinamicas.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/home/utnso/memcached_engine_example/memcached-1.6/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


