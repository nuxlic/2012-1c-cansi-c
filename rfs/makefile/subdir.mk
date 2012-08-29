################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../bitmap.c \
../ext2_operations.c \
../fs_handler.c \
../groupDescriptor.c \
../inode.c \
../memcachear.c \
../multiThreading.c \
../procesadorDePedidos.c \
../rfs.c \
../rfsCommon.c \
../socketHandler.c \
../superbloque.c \
../synchronizer.c 

OBJS += \
./bitmap.o \
./ext2_operations.o \
./fs_handler.o \
./groupDescriptor.o \
./inode.o \
./memcachear.o \
./multiThreading.o \
./procesadorDePedidos.o \
./rfs.o \
./rfsCommon.o \
./socketHandler.o \
./superbloque.o \
./synchronizer.o 

C_DEPS += \
./bitmap.d \
./ext2_operations.d \
./fs_handler.d \
./groupDescriptor.d \
./inode.d \
./memcachear.d \
./multiThreading.d \
./procesadorDePedidos.d \
./rfs.d \
./rfsCommon.d \
./socketHandler.d \
./superbloque.d \
./synchronizer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -D_FILE_OFFSET_BITS=64 -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


