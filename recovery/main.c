#include <hal/system.h>
#include <hal/delay.h>
#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>

#include <database.h>
#include <log.h>
#include <mount_points.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <lsqlite3/lsqlite3.h>
#include <luafilesystem/src/src/lfs.h>
#include <ltar/src/lmicrotar.h>
#include <lrecovery.h>

#include <fcntl.h>
#include <string.h>
#include <errno.h>

static const char *working_dir = "/system/scripts";
static const char *entry_point = "entry.lua";

static lua_State *prepare_lua_context() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "lsqlite3complete", luaopen_lsqlite3, 1);
    luaL_requiref(L, "lfs", luaopen_lfs, 1);
    luaL_requiref(L, "lmicrotar", luaopen_lmicrotar, 1);
    luaL_requiref(L, "lrecovery", luaopen_lrecovery, 1);
    return L;
}

static int invoke_entry_point(lua_State *L, const char *entry) {
    char path[256] = {};
    chdir(working_dir);
    snprintf(path, sizeof(path), "%s/%s", working_dir, entry);
    return luaL_dofile(L, path);
}

static int mount_filesystem() {
    const vfs_mount_point_desc_t initial_fstab[] = {
            {.disk = blkdev_emmc_user, .partition = 3, .type = vfs_fs_auto, .mount_point = get_user_mount_point()},
    };

    int err = vfs_mount_init(initial_fstab, sizeof initial_fstab);
    if (err != 0) {
        debug_log("Unable to init vfs: %d", err);
        return err;
    }

    vfs_mount_point_desc_t fstab[3] = {};
    err = get_mount_points(fstab);
    if (err != 0) {
        vfs_unmount_deinit();
        return err;
    }

    /// Re-mount a filesystem with the corrected mount points
    err = vfs_unmount_deinit();
    if (err != 0) {
        debug_log("Unable to unmount vfs: %d", err);
        return err;
    }
    return vfs_mount_init(fstab, sizeof fstab);
}

static void create_log_directory(const char *dir) {
    struct stat st = {};
    const int ret = stat(dir, &st);
    if (ret == -1 && errno == ENOENT) {
        mkdir(dir, 0666);
    }
}

static int prepare_environment() {
    system_initialize();

    int err = mount_filesystem();
    if (err != 0) {
        debug_log("Unable to mount filesystem: %d", err);
        return err;
    }

    create_log_directory(get_log_directory());

    redirect_logs_to_file(get_log_filename());

    database_initialize();

    return 0;
}

int destroy_environment() {
    flush_logs();
    database_deinitialize();
    int err = vfs_unmount_deinit();
    system_deinitialize();
    return err;
}

int __attribute__((noinline, used)) main() {
    int err = prepare_environment();
    if (err != 0) {
        return err;
    }

    lua_State *L = prepare_lua_context();
    err = invoke_entry_point(L, entry_point);
    if (err) {
        debug_log("Error occurs when executing entry point, Hint Machine 0x%x", err);
        debug_log("Error: %s", lua_tostring(L, -1));
    }

    debug_log("PureRecovery finished, exiting...");
    destroy_environment();
    return err;
}
