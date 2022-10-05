#include <hal/system.h>
#include <hal/delay.h>
#include <hal/display.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <seatest/seatest.h>
#include "test_suite_vfs.h"
#include "test_suite_crypto.h"
#include "test_suite_db.h"

static void all_tests(void)
{
    test_fixture_vfs();
    test_suite_crypto();
    test_suite_db();
}

int __attribute__((noinline, used)) main()
{
    // System initialize
    system_initialize();
    // Eink welcome message
    eink_clear_log();
    eink_log_printf("Unit test updater");
    eink_log_printf("Date %s %s", __DATE__, __TIME__);
    eink_log("Please wait...", false);
    eink_log_refresh();

    mount_all();

    int err = run_tests(all_tests);
    eink_log_printf("Finished with code %i", err);
    eink_log_refresh();
    
    umount_all();

    system_deinitialize();
    // Do not reset
    return -1;
}
