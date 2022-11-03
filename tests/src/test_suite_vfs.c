#include "test_suite_vfs.h"
#include <seatest/seatest.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/statvfs.h>
#include <hal/delay.h>
#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define AUTO_BUF(var) char *var __attribute__((__cleanup__(_free_clean_up_buf)))

static void _free_clean_up_buf(char **str)
{
    free((void *)*str);
    *str = NULL;
}

/** Tests for the basic read functionality
 * using the FILE stdio access interface
 */
static void test_basic_file_read_api()
{
    // Open the boot json on master partition
    FILE *file = fopen("/os/.boot.json", "r");
    if (!file)
    {
        assert_fail("Fopen error skip other tests for fat");
    }
    else
    {
        assert_int_equal(0, ftell(file));
        assert_int_equal(0, fseek(file, 0, SEEK_END));
        assert_true(ftell(file) > 100);
        assert_int_equal(0, fclose(file));
    }
    // Try the second littlefs partition
    file = fopen("/user/db/contacts_001.sql", "r");
    if (!file)
    {

        assert_fail("Fopen error skip other tests for lfs");
    }
    else
    {
        assert_int_equal(0, ftell(file));
        assert_int_equal(0, fseek(file, 0, SEEK_END));
        assert_true(ftell(file) > 100);
        assert_int_equal(0, fclose(file));
    }
}

void mount_all()
{
    // fstab filesystem mounts
    static const vfs_mount_point_desc_t fstab[] = {
        {.disk = blkdev_emmc_user, .partition = 1, .type = vfs_fs_fat, "/os"},
        {.disk = blkdev_emmc_user, .partition = 3, .type = vfs_fs_auto, "/user"},
    };
    printf("Initializing VFS subsystem...\n");
    int err = vfs_mount_init(fstab, sizeof fstab);
    if (err)
    {
        printf("Failed to initialize VFS errno %i\n", err);
    }
}

void umount_all()
{
    int err = vfs_unmount_deinit();
    if (err)
    {
        printf("Failed to umount VFS data errno %i\n", err);
    }
}

/** Test for write data
 */
static void test_basic_write_files(void)
{
    const char *files_to_write[] = {"/user/test001.bin", "/os/test002.bin"};
    int arr_to_wr[100];
    int arr_to_rd[100];
    for (size_t n = 0; n < ARRAY_SIZE(arr_to_wr); ++n)
    {
        arr_to_wr[n] = n;
    }
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_write); ++fno)
    {
        const char *fname = files_to_write[fno];
        FILE *file = fopen(fname, "w");
        if (!file)
        {
            assert_fail("Unable to open file for write");
        }
        else
        {
            assert_ulong_equal(ARRAY_SIZE(arr_to_wr),
                               fwrite(arr_to_wr, sizeof(arr_to_wr[0]), ARRAY_SIZE(arr_to_wr), file));
            assert_int_equal(sizeof(arr_to_wr), ftell(file));
            assert_int_equal(0, fclose(file));
        }
    }
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_write); ++fno)
    {
        const char *fname = files_to_write[fno];
        FILE *file = fopen(fname, "r");
        if (!file)
        {
            assert_fail("Unable to open file for write");
        }
        else
        {
            memset(arr_to_rd, 0, sizeof arr_to_rd);
            assert_ulong_equal(ARRAY_SIZE(arr_to_rd),
                               fread(arr_to_rd, sizeof(arr_to_rd[0]), ARRAY_SIZE(arr_to_rd), file));
            assert_int_equal(sizeof(arr_to_rd), ftell(file));
            assert_int_equal(0, fclose(file));
            assert_n_array_equal(arr_to_wr, arr_to_rd, ARRAY_SIZE(arr_to_rd));
        }
    }
}

// Failed to open file test
static void test_relative_open(void)
{
    const char *dirs_to_create[] = {"dir1"};
    const char *files_to_open[] = {"test101.bin", "dir1/test002.bin"};

    for(size_t dno = 0; dno< ARRAY_SIZE(dirs_to_create);dno++){
        assert_true(mkdir(dirs_to_create[dno], 0666) == 0);
        assert_true(rmdir(dirs_to_create[dno]) == 0);
    }

    for(size_t fno = 0; fno< ARRAY_SIZE(files_to_open);fno++){
        FILE* fd = fopen(files_to_open[fno],"w");
        assert_true(fd != NULL);
        fclose(fd);
        unlink(files_to_open[fno]);
    }
}

// Failed to open file test
static void test_failed_to_open_files(void)
{
    assert_true(fopen(NULL, "r") == NULL);
    assert_int_equal(EINVAL, errno);
    assert_true(fopen("", "r") == NULL);
    assert_int_equal(EINVAL, errno);
    assert_true(fopen("/nonexist_mountpoint/xxx.txt", "r") == NULL);
    assert_int_equal(ENOENT, errno);
    assert_true(fopen("/user/xxxx.txt", "r") == NULL);
    assert_int_equal(ENOENT, errno);
    assert_true(fopen("/os/xxxx.txt", "r") == NULL);
    assert_int_equal(ENOENT, errno);
}

static ssize_t file_length(const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file)
    {
        return -1;
    }
    if (fseek(file, 0, SEEK_END) < 0)
    {
        fclose(file);
        return -1;
    }
    ssize_t len = ftell(file);
    fclose(file);
    return len;
}

// Create and remove files
static void test_create_and_remove_files(void)
{
    const char *files_to_check[] = {"/user/test003.bin", "/os/test004.bin"};
    static const size_t trunc_size = 256 * 1024;
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_check); ++fno)
    {
        const char *fname = files_to_check[fno];
        assert_int_equal(0, truncate(fname, trunc_size));
    }
    // Open and check for sizes
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_check); ++fno)
    {
        const char *fname = files_to_check[fno];
        assert_int_equal(trunc_size, file_length(fname));
    }
    // Unlink
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_check); ++fno)
    {
        const char *fname = files_to_check[fno];
        assert_int_equal(0, unlink(fname));
    }
    // Check for sizes again
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_check); ++fno)
    {
        const char *fname = files_to_check[fno];
        assert_int_equal(-1, file_length(fname));
        assert_int_equal(ENOENT, errno);
    }
    // Unlink unexistient
    assert_int_equal(-1, unlink("/os/non_existientfile"));
    assert_int_equal(ENOENT, errno);
}

// Tests for directory create removal and stat
static void test_directory_create_remove_stat_base(const char *basedir)
{
    printf("base dir %s\n", basedir);
    char path[96];
    snprintf(path, sizeof path, "%s/dirtest", basedir);
    assert_int_equal(0, mkdir(path, 0755));
    for (size_t d = 0; d < 20; ++d)
    {
        snprintf(path, sizeof path, "%s/dirtest/dir%i", basedir, d);
        assert_int_equal(0, mkdir(path, 0755));
    }
    snprintf(path, sizeof path, "%s/dirtest/dir2/filx", basedir);
    assert_int_equal(0, truncate(path, 16384));
    // Check stat for file and dir
    struct stat st;
    assert_int_equal(0, stat(basedir, &st));
    assert_true(S_ISDIR(st.st_mode));

    assert_int_equal(0, stat(path, &st));
    assert_true(S_ISREG(st.st_mode));
    assert_int_equal(16384, st.st_size);

    assert_int_equal(0, unlink(path));

    for (size_t d = 0; d < 20; ++d)
    {
        struct stat st;
        snprintf(path, sizeof path, "%s/dirtest/dir%i", basedir, d);
        assert_int_equal(0, stat(path, &st));
        assert_true(S_ISDIR(st.st_mode));
    }
    // Try to remove the directories
    for (size_t d = 0; d < 20; ++d)
    {
        snprintf(path, sizeof path, "%s/dirtest/dir%i", basedir, d);
        assert_int_equal(0, rmdir(path));
    }
    // Last dir remove
    snprintf(path, sizeof path, "%s/dirtest", basedir);
    assert_int_equal(0, rmdir(path));

    assert_int_equal(-1, rmdir(path));
    assert_int_equal(ENOENT, errno);
}

static void test_root_directory_stat_tests(const char* base)
{
    struct stat st;
    // Copy base path
    char path[128];
    strncpy(path, base, sizeof path);

    //Check if it is a directory
    assert_int_equal(0, stat(path, &st));
    assert_true(S_ISDIR(st.st_mode));

    // Append tailing backspace
    strncat(path, "/", sizeof path);

    // Check if it is a directory once again
    assert_int_equal(0, stat(path, &st));
    assert_true(S_ISDIR(st.st_mode));
}

static void test_advanced_stats(const char* base)
{
    // Create directory and file in that dir
    char sub_dir[128];
    char file_in_dir[128];
    struct stat st;
    // Prepare name for directory
    strncpy(sub_dir, base, sizeof sub_dir);
    strncat(sub_dir, "/advdir", sizeof sub_dir);
    // Prepare name for file
    strncpy(file_in_dir, sub_dir, sizeof sub_dir);
    strncat(file_in_dir , "/advfile", sizeof sub_dir);
    printf("%s: base: %s, subdir: %s, subfile: %s\n",
            __PRETTY_FUNCTION__, base, sub_dir, file_in_dir);
    // Create directory and truncated file
    assert_int_equal(0, mkdir(sub_dir, 0755));
    assert_int_equal(0, truncate(file_in_dir, 124567));

    // Now check for stat for directories and sub dirs
    // Root dir
    assert_int_equal(0, stat(base, &st));
    assert_false(S_ISFIFO(st.st_mode));
    assert_false(S_ISCHR(st.st_mode));
    assert_true(S_ISDIR(st.st_mode));
    assert_false(S_ISBLK(st.st_mode));
    assert_false(S_ISLNK(st.st_mode));
    assert_false(S_ISSOCK(st.st_mode));
    assert_false(S_ISREG(st.st_mode));

    // Sub dir
    assert_int_equal(0, stat(sub_dir, &st));
    assert_false(S_ISFIFO(st.st_mode));
    assert_false(S_ISCHR(st.st_mode));
    assert_true(S_ISDIR(st.st_mode));
    assert_false(S_ISBLK(st.st_mode));
    assert_false(S_ISLNK(st.st_mode));
    assert_false(S_ISSOCK(st.st_mode));
    assert_false(S_ISREG(st.st_mode));

    // Check for file
    assert_int_equal(0, stat(file_in_dir, &st));
    assert_int_equal(124567, st.st_size);
    assert_false(S_ISFIFO(st.st_mode));
    assert_false(S_ISCHR(st.st_mode));
    assert_false(S_ISDIR(st.st_mode));
    assert_false(S_ISBLK(st.st_mode));
    assert_false(S_ISLNK(st.st_mode));
    assert_false(S_ISSOCK(st.st_mode));
    assert_true(S_ISREG(st.st_mode));

    // Final cleanup
    assert_int_equal(0, unlink(file_in_dir));
    assert_int_equal(0, rmdir(sub_dir));

}

static void test_directory_create_remove_stat_user(void)
{
    test_directory_create_remove_stat_base("/user");
    test_root_directory_stat_tests("/user");
    test_advanced_stats("/user");
}

static void test_directory_create_remove_stat_os(void)
{
    test_directory_create_remove_stat_base("/os");
    test_root_directory_stat_tests("/os");
    test_advanced_stats("/os");
}

static void test_dir_travesal_intervfs(void)
{
    assert_true(opendir("/kupa") == NULL);
    assert_int_equal(ENOENT, errno);
    assert_true(opendir("/os/xxx") == NULL);
    assert_int_equal(ENOENT, errno);
    assert_true(opendir("/user/xxx") == NULL);
    assert_int_equal(ENOENT, errno);
    // Traverse inter mount point
    DIR *dir = opendir("/");
    if (!dir)
    {
        assert_fail("opendir error skip other tests");
    }
    else
    {
        struct dirent *dent;
        int count = 0;
        while ((dent = readdir(dir)) != NULL)
        {
            count++;
        }
        // Two mount points
        assert_int_equal(2, count);
        assert_int_equal(0, closedir(dir));
    }
}

static void test_dir_traversal(const char *basedir)
{
    char path[96];

    // Create resources
    snprintf(path, sizeof path, "%s/dirtest", basedir);
    assert_int_equal(0, mkdir(path, 0755));
    for (size_t d = 0; d < 20; ++d)
    {
        snprintf(path, sizeof path, "%s/dirtest/dir%u", basedir, d);
        assert_int_equal(0, mkdir(path, 0755));
    }
    snprintf(path, sizeof path, "%s/dirtest/filx", basedir);
    assert_int_equal(0, truncate(path, 16384));

    // Major tests
    snprintf(path, sizeof path, "%s/dirtest", basedir);
    DIR *dirh = opendir(path);
    assert_true(dirh != NULL);
    if (dirh)
    {
        struct dirent *dent;
        int dcnt = 0;
        int fcnt = 0;
        while ((dent = readdir(dirh)) != NULL)
        {
            if (dent->d_type == DT_REG)
            {
                fcnt++;
                assert_string_equal("filx", dent->d_name);
            }
            if (dent->d_type == DT_DIR)
            {
                dcnt++;
            }
        }
        assert_int_equal(20, dcnt);
        assert_int_equal(1, fcnt);
        assert_int_equal(0, closedir(dirh));
    }
    // Delete resources
    for (size_t d = 0; d < 20; ++d)
    {
        snprintf(path, sizeof path, "%s/dirtest/dir%u", basedir, d);
        assert_int_equal(0, rmdir(path));
    }
    snprintf(path, sizeof path, "%s/dirtest/filx", basedir);
    assert_int_equal(0, unlink(path));
    // Last dir remove
    snprintf(path, sizeof path, "%s/dirtest", basedir);
    assert_int_equal(0, rmdir(path));
    // Test for opening base directory
    dirh = opendir(basedir);
    assert_true(dirh != NULL);
    assert_int_equal(0, closedir(dirh));
}

static void test_dir_traversal_user(void)
{
    test_dir_traversal("/user");
}

static void test_dir_traversal_os(void)
{
    test_dir_traversal("/os");
}

static void test_statvfs(void)
{
    struct statvfs svfs;
    assert_int_equal(-1, statvfs("/xyz", &svfs));
    assert_int_equal(ENOENT, errno);
    assert_int_equal(0, statvfs("/os/test", &svfs));
    assert_int_equal(512, svfs.f_bsize);
    assert_true(svfs.f_frsize > 128);
    assert_true(svfs.f_blocks > 100);
    assert_true(svfs.f_bfree > 10);
    memset(&svfs, 0, sizeof svfs);
    assert_int_equal(0, statvfs("/user/test", &svfs));
    assert_true(svfs.f_bsize > 0);
    assert_true(svfs.f_frsize > 0);
    assert_true(svfs.f_blocks > 100);
    assert_true(svfs.f_bfree > 10);
}

static void test_rename_with_base(const char *basedir)
{
    char oldf[PATH_MAX];
    char newf[PATH_MAX];
    snprintf(oldf, sizeof oldf, "%s/file_to_rename", basedir);
    snprintf(newf, sizeof newf, "%s/file_renamed", basedir);
    assert_int_equal(-1, rename("/os/not_exists", "/os/new_notexists"));
    assert_int_equal(ENOENT, errno);
    assert_int_equal(0, truncate(oldf, 16384));
    assert_int_equal(0, rename(oldf, newf));
    // Now new file should exists and can be deleted
    assert_int_equal(0, unlink(newf));
}

static void test_rename_os(void)
{
    test_rename_with_base("/os");
}

static void test_rename_user(void)
{
    test_rename_with_base("/user");
}

//////////////////// Performance tests //////////////////////////////////////

static int num = 0;
static void initFilePathGenerator(int seed)
{
    num = seed;
}
static void generateFilePath(const char *const base_dir, size_t max_path_length, char *path)
{
    snprintf(path, max_path_length, "%s/speed_file%d", base_dir, num++);
}

static uint32_t calculateSpeed(size_t file_size, uint32_t tdiff)
{
    return (uint32_t)((((uint64_t)file_size * 1000) / tdiff) / 1024U);
}

struct Stats
{
    uint32_t min;
    uint32_t max;
    uint32_t avg;
};

/// initally min should be set to max uint32_t
static uint32_t calcMin(uint32_t current_min, uint32_t new_val)
{
    return new_val < current_min ? new_val : current_min;
}

/// initally max should be set to 0
static uint32_t calcMax(uint32_t current_max, uint32_t new_val)
{
    return new_val > current_max ? new_val : current_max;
}

static uint32_t calcAvg(uint32_t current_avg, uint32_t sample_count, uint32_t new_val)
{
    int64_t temp = current_avg;
    temp += ((int64_t)new_val - temp) / (int64_t)sample_count;
    return (uint32_t)temp;
}

static void calcStats(struct Stats *stats, uint32_t sample_count, uint32_t new_value)
{
    if (stats == NULL)
        return;

    stats->min = calcMin(stats->min, new_value);
    stats->max = calcMax(stats->max, new_value);
    stats->avg = calcAvg(stats->avg, sample_count, new_value);
}

static uint32_t test_write_speed(const char *path, size_t file_size, size_t chunk_size, size_t iobuf_size)
{
    AUTO_BUF(buf) = malloc(chunk_size);
    AUTO_BUF(iobuf) = malloc(iobuf_size);

    FILE *fil = fopen(path, "w");
    assert_true(fil != NULL);
    if (!fil)
    {
        printf("Fatal unable to open file skip speed write tests\n");
        return 0;
    }
    assert_int_equal(0, setvbuf(fil, iobuf, _IOFBF, iobuf_size));
    uint32_t t1 = get_jiffiess();
    for (size_t ch = 0; ch < file_size / chunk_size; ++ch)
    {
        assert_int_equal(1, fwrite(buf, chunk_size, 1, fil));
    }
    fclose(fil);
    uint32_t t2 = get_jiffiess();
    uint32_t tdiff = jiffiess_timer_diff(t1, t2);
    uint32_t writeSpeed = calculateSpeed(file_size, tdiff);

    printf("    %s - write speed %lu kB/s time %lu ms iobuf_size %u kB\n", path, writeSpeed, tdiff, iobuf_size / 1024u);

    return writeSpeed;
}

static uint32_t test_read_speed(const char *path, size_t chunk_size, size_t iobuf_size)
{
    AUTO_BUF(buf) = malloc(chunk_size);
    AUTO_BUF(iobuf) = malloc(iobuf_size);

    ssize_t file_size = file_length(path);

    FILE *fil = fopen(path, "r");

    // Read test
    assert_true(fil != NULL);
    if (!fil)
    {
        printf("Fatal unable to open file skip speed read tests\n");
        unlink(path);
        return 0;
    }
    assert_int_equal(0, setvbuf(fil, iobuf, _IOFBF, iobuf_size));
    uint32_t t1 = get_jiffiess();
    for (size_t ch = 0; ch < file_size / chunk_size; ++ch)
    {
        assert_int_equal(1, fread(buf, chunk_size, 1, fil));
    }
    fclose(fil);
    uint32_t t2 = get_jiffiess();
    uint32_t tdiff = jiffiess_timer_diff(t1, t2);
    uint32_t readSpeed = calculateSpeed(file_size, tdiff);

    printf("    %s - read speed %lu kB/s time %lu ms iobuf_size %u kB\n", path, readSpeed, tdiff, iobuf_size / 1024u);

    return readSpeed;
}

static void test_seq_write_and_read_speed_os(
    char *base_dir, uint32_t number_of_files, size_t file_size, size_t chunk_size, size_t iobuf_size)
{
    struct Stats write_stats = {0xFFFFFFFF, 0, 0};
    struct Stats read_stats = {0xFFFFFFFF, 0, 0};

    initFilePathGenerator(0);
    char path[PATH_MAX];

    printf("  Performance test number of files %lu file size %u chunk size %u iobuf size %u\n",
           number_of_files,
           file_size,
           chunk_size,
           iobuf_size);

    for (unsigned i = 0; i < number_of_files; i++)
    {
        generateFilePath(base_dir, sizeof(path), path);
        uint32_t speed = test_write_speed(path, file_size, chunk_size, iobuf_size);
        calcStats(&write_stats, i + 1, speed);
    }

    printf("   write speed - min %lu kB/s, max %lu kB/s, avg %lu kB/s\n",
           write_stats.min,
           write_stats.max,
           write_stats.avg);

    initFilePathGenerator(0);
    for (unsigned i = 0; i < number_of_files; i++)
    {
        generateFilePath(base_dir, sizeof(path), path);
        uint32_t speed = test_read_speed(path, chunk_size, iobuf_size);
        calcStats(&read_stats, i + 1, speed);
    }

    printf(
        "   read speed - min %lu kB/s, max %lu kB/s, avg %lu kB/s\n", read_stats.min, read_stats.max, read_stats.avg);

    initFilePathGenerator(0);
    for (unsigned i = 0; i < number_of_files; i++)
    {
        generateFilePath(base_dir, sizeof(path), path);
        unlink(path);
    }
}

static void test_seq_write_after_umount(char *base_dir,
                                        uint32_t number_of_serires,
                                        uint32_t number_of_files,
                                        size_t file_size,
                                        size_t chunk_size,
                                        size_t iobuf_size)
{
    struct Stats write_stats = {0xFFFFFFFF, 0, 0};

    initFilePathGenerator(0);
    char path[PATH_MAX];

    for (uint32_t i = 0; i < number_of_serires; i++)
    {
        umount_all();
        mount_all();

        printf("  Performance test number of files %lu file size %u chunk size %u iobuf size %u\n",
               number_of_files,
               file_size,
               chunk_size,
               iobuf_size);

        for (unsigned i = 0; i < number_of_files; i++)
        {
            generateFilePath(base_dir, sizeof(path), path);
            uint32_t speed = test_write_speed(path, file_size, chunk_size, iobuf_size);
            calcStats(&write_stats, i + 1, speed);
        }

        printf("   write speed - min %lu kB/s, max %lu kB/s, avg %lu kB/s\n",
               write_stats.min,
               write_stats.max,
               write_stats.avg);

        struct statvfs stat;

        uint32_t t1 = get_jiffiess();
        vfs_statvfs(base_dir, &stat);
        uint32_t t2 = get_jiffiess();
        uint32_t tdiff = jiffiess_timer_diff(t1, t2);
        printf("   statvfs time %lu ms\n", tdiff);
    }

    initFilePathGenerator(0);
    for (unsigned i = 0; i < number_of_files * number_of_serires; i++)
    {
        generateFilePath(base_dir, sizeof(path), path);
        unlink(path);
    }
}

static void test_speed_os()
{
    printf("Performance test os - iobuffer size\n");
    for (unsigned i = 1; i <= 512; i = i * 2)
    {
        test_seq_write_and_read_speed_os("/os", 5, 4 * 1024 * 1024, 16 * 1024, i * 1024);
    }
    printf("Performance test os - stability test\n");
    test_seq_write_and_read_speed_os("/os", 100, 4 * 1024 * 1024, 16 * 1024, 64 * 1024);
}

static void test_speed_user(void)
{
    printf("Performance test user - iobuffer size\n");
    for (unsigned i = 1; i <= 512; i = i * 2)
    {
        test_seq_write_and_read_speed_os("/user", 5, 4 * 1024 * 1024, 16 * 1024, i * 1024);
    }
    printf("Performance test user - stability test\n");
    test_seq_write_and_read_speed_os("/user", 100, 4 * 1024 * 1024, 16 * 1024, 64 * 1024);

    printf("Performance test user - stability test with umounts\n");
    test_seq_write_after_umount("/user", 10, 100, 4 * 1024 * 1024, 16 * 1024, 64 * 1024);
}

static void test_getcwd(void){
    char cwd[PATH_MAX] = {};
    getcwd(cwd,sizeof(cwd));
    assert_string_equal(cwd,"/user");
}

static void test_chdir(void){
    char cwd[PATH_MAX] = {};
    getcwd(cwd,sizeof(cwd));
    assert_string_equal(cwd,"/user");

    chdir("/os");
    getcwd(cwd,sizeof(cwd));
    assert_string_equal(cwd,"/os");

    chdir("current");
    getcwd(cwd,sizeof(cwd));
    assert_string_equal(cwd,"/os/current");

    chdir("/user/db");
    getcwd(cwd,sizeof(cwd));
    assert_string_equal(cwd,"/user/db");
}

// VFS test fixutre
void test_fixture_vfs()
{
    test_fixture_start();
    run_test(test_basic_file_read_api);
    run_test(test_basic_write_files);
    run_test(test_failed_to_open_files);
    run_test(test_create_and_remove_files);
    run_test(test_directory_create_remove_stat_user);
    run_test(test_directory_create_remove_stat_os);
    run_test(test_dir_travesal_intervfs);
    run_test(test_dir_traversal_user);
    run_test(test_dir_traversal_os);
    run_test(test_statvfs);
    run_test(test_rename_os);
    run_test(test_rename_user);
    // run_test(test_speed_os);
    // run_test(test_speed_user);
    run_test(test_getcwd);
    run_test(test_chdir);
    run_test(test_relative_open);
    test_fixture_end();
}
