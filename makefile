-include ../makefile.init

USER_OBJS :=
LIBS := 


O_SRCS := 
C_SRCS := src/transport src/log
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
OBJS :=
C_DEPS := 
EXECUTABLES :=
BIN_DIR := ./bin


C_SRCS += \
./src/main_a.c ./src/main_b.c ./src/main_c.c 

OBJS += \
./bin/main_a.o ./bin/main_b.o ./bin/main_c.o 

C_DEPS += \
./bin/main_a.d ./bin/main_b.d ./bin/main_c.d 

# Every subdirectory with source files must be described here
SUBDIRS := \
. src/transport src/log

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: clean allprocesses

# Tool invocations
allprocesses: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "$(BIN_DIR)/processa" ./bin/main_a.o $(USER_OBJS) $(LIBS)
	gcc  -o "$(BIN_DIR)/processb" ./bin/main_b.o $(USER_OBJS) $(LIBS)
	gcc  -o "$(BIN_DIR)/processc" ./bin/main_c.o $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

processa: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "$(BIN_DIR)/processa" ./bin/main_a.o $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

processb: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "$(BIN_DIR)/processb" ./bin/main_b.o $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' ':

processc: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o "$(BIN_DIR)/processc" ./bin/main_c.o $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
processc:

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) "$(BIN_DIR)/processa" "$(BIN_DIR)/processb" "$(BIN_DIR)/processc"


.PHONY: all clean dependents
.SECONDARY:


# Each subdirectory must supply rules for building sources it contributes
bin/%.o: src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


-include ../makefile.targets
