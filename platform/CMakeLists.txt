# System library for all targets
set( LIB_NAME system )

add_library( ${LIB_NAME} STATIC )

set(LIB_RUNTIME_OBJS ${LIB_NAME}-runtime-objs)
add_library( ${LIB_RUNTIME_OBJS} OBJECT)
target_link_libraries( ${LIB_RUNTIME_OBJS} PUBLIC ${LIB_NAME} )


target_sources( ${LIB_NAME} PRIVATE
        drivers/hab/hab.c

        SDK_2_13_0_MIMXRT1051xxxxB-2/components/osa/fsl_os_abstraction_bm.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/components/lists/fsl_component_generic_list.c

        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/sdmmc/mmc/fsl_mmc.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/sdmmc/host/usdhc/non_blocking/fsl_sdmmc_host.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/sdmmc/common/fsl_sdmmc_common.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/sdmmc/osa/fsl_sdmmc_osa.c

        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/dcd/usb_phydcd.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/phy/usb_phy.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/device/usb_device_ehci.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/device/usb_device_dci.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/output/source/device/usb_device_ch9.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/output/source/device/class/usb_device_class.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/output/source/device/class/usb_device_cdc_acm.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/output/source/device/class/usb_device_mtp.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/output/source/device/class/usb_device_mtp_operation.c

        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_common_arm.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_cache.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_clock.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_common.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_dcp.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_dmamux.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_edma.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_flexram.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_flexspi.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_gpio.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_gpt.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_lpi2c.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_lpspi.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_lpuart.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_lpuart_edma.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_pit.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_pmu.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_rtwdog.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_semc.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_snvs_hp.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_snvs_lp.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_src.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_trng.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_usdhc.c
        SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers/fsl_wdog.c

        )
target_link_libraries( ${LIB_NAME} PUBLIC hal-common )


target_sources( ${LIB_RUNTIME_OBJS} PRIVATE
        syscalls/direntsyscalls.c
        syscalls/exit.c
        syscalls/fdsycalls.c
        syscalls/fnsyscalls.c
        syscalls/sbrk.c
        syscalls/syscalls.c

        boot/clock_config.c
        boot/evkbimxrt1050_flexspi_nor_config.c
        boot/evkbimxrt1050_sdram_ini_dcd.c
        boot/fsl_flexspi_nor_boot.c
        boot/pin_mux.c
        boot/startup_mimxrt1051.c
        boot/system_MIMXRT1051.c
        )


# Include directories
target_include_directories( ${LIB_NAME} PUBLIC
    SDK_2_13_0_MIMXRT1051xxxxB-2/CMSIS/Core/Include
    SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051
    SDK_2_13_0_MIMXRT1051xxxxB-2/devices/MIMXRT1051/drivers
    SDK_2_13_0_MIMXRT1051xxxxB-2/components/lists
    SDK_2_13_0_MIMXRT1051xxxxB-2/components/osa

    SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/sdmmc/common
    SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/sdmmc/host/usdhc
    SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/sdmmc/mmc
    SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/sdmmc/osa

        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/dcd
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/device
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/include
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/phy
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/output/source/device
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/output/source/device/class
        SDK_2_13_0_MIMXRT1051xxxxB-2/middleware/usb/output/npw/device_config/ehci

        include
)
# User defines
target_compile_definitions(${LIB_NAME} PUBLIC
    FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1
    __STARTUP_INITIALIZE_NONCACHEDATA
    XIP_EXTERNAL_FLASH=0
    CPU_MIMXRT1051DVL6B_cm7
    CPU_MIMXRT1051DVL6B
    __FPU_PRESENT
    SKIP_SYSCLK_INIT
    _GNU_SOURCE
    DISABLE_WATCHDOG
        DATA_SECTION_IS_CACHEABLE
)


if (NOT ENABLE_SECURE_BOOT)
    # Only enable the build-time boot header in a non-secure configuration,
    # a header for Secure Boot is added post-build when signing the binary
    target_compile_definitions(${LIB_NAME} PUBLIC
        XIP_BOOT_HEADER_ENABLE=1
        XIP_BOOT_HEADER_DCD_ENABLE=0
    )
endif()



#target_compile_options(
#    ${LIB_RUNTIME_OBJS} PRIVATE "-Werror"
#)
