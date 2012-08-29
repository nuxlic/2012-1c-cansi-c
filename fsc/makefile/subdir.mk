################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Client.c \
../connectionPool.c \
../fscCommon.c \
../fuse.c \
../memcachear.c \
../serializadores.c \
../sockear.c 

OBJS += \
./Client.o \
./connectionPool.o \
./fscCommon.o \
./fuse.o \
./memcachear.o \
./serializadores.o \
./sockear.o 

C_DEPS += \
./Client.d \
./connectionPool.d \
./fscCommon.d \
./fuse.d \
./memcachear.d \
./serializadores.d \
./sockear.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


