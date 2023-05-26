add_subdirectory(common)

add_library(recovery STATIC
        flash_bootloader.c
        flash_secure_fuses.c
        convert_fs.c
        security/pgmkeys.c

        #USB VCOM
                usb/vcom/usb_device_descriptor.c
                usb/vcom/virtual_com.c

        #USB MTP
#        usb/mtp/mtp_file_system_adapter.c
#        usb/mtp/mtp_object_handle.c
#        usb/mtp/mtp_operation.c
#        usb/mtp/mtp.c
#        usb/mtp/usb_device_descriptor.c
        )

target_link_libraries(recovery PRIVATE hal-common md5 common system pure-core)
target_include_directories(recovery PUBLIC .)