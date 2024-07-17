################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/msp430-gcc/bin/msp430-elf-gcc-9.3.1.exe" -c -mmcu=msp430fr5994 -mhwmult=f5series -I"C:/ti/ccs1250/ccs/ccs_base/msp430/include_gcc" -I"C:/ti/msp/MSP430Ware_3_80_14_01/driverlib/driverlib/MSP430FR5xx_6xx" -I"C:/Users/User/workspace_v12/alpaca/alpaca" -I"C:/ti/msp430-gcc/msp430-elf/include" -Og -g -gdwarf-3 -gstrict-dwarf -Wall -mlarge -mcode-region=none -mdata-region=lower -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


