#pragma once

//! Error codes
enum ecoboot_update_errors {
    error_eco_update_ok,
    // With errno
    error_eco_vfs,    //! Errno related to vfs
    error_eco_blkdev, //! Errno related to blkdev
    error_eco_system, //! Generic system error
    //Without errno
    error_eco_update_no_reg_file, //! Not a regular file
    error_eco_too_small,          //! Bootloader too small
    error_eco_verify              //! Verification failed
};

/** Update the ecoboot from the selected file
 * @param[in] path Path to the bootloader image
 * @return 0 if success -errno on failure
 */
int flash_bootloader(const char *path);
