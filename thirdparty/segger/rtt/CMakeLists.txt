add_library(rtt SEGGER_RTT.c)
add_library(segger::rtt ALIAS rtt)

target_include_directories(rtt PUBLIC .)

set_property(SOURCE SEGGER_RTT_ASM_ARMv7M.S PROPERTY LANGUAGE C)

