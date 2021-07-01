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

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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
        assert_int_equal(ftell(file), 0);
        assert_int_equal(fseek(file, 0, SEEK_END), 0);
        assert_true(ftell(file) > 100);
        assert_int_equal(fclose(file), 0);
    }
    // Try the second littlefs partition
    file = fopen("/user/db/contacts_001.sql", "r");
    if (!file)
    {

        assert_fail("Fopen error skip other tests for lfs");
    }
    else
    {
        assert_int_equal(ftell(file), 0);
        assert_int_equal(fseek(file, 0, SEEK_END), 0);
        assert_true(ftell(file) > 100);
        assert_int_equal(fclose(file), 0);
    }
}

/** Test for write data
 */
static void test_basic_write_files(void)
{
    const char *files_to_write[] = {
        "/user/test001.bin",
        "/os/test002.bin"};
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
            assert_ulong_equal(
                fwrite(arr_to_wr, sizeof(arr_to_wr[0]), ARRAY_SIZE(arr_to_wr), file),
                ARRAY_SIZE(arr_to_wr));
            assert_int_equal(ftell(file), sizeof(arr_to_wr));
            fclose(file);
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
            assert_ulong_equal(
                fread(arr_to_rd, sizeof(arr_to_rd[0]), ARRAY_SIZE(arr_to_rd), file),
                ARRAY_SIZE(arr_to_rd));
            assert_int_equal(ftell(file), sizeof(arr_to_rd));
            fclose(file);
            assert_n_array_equal(arr_to_wr, arr_to_rd, ARRAY_SIZE(arr_to_rd));
        }
    }
}

// Failed to open file test
static void test_failed_to_open_files(void)
{
    assert_true(fopen(NULL, "r") == NULL);
    assert_int_equal(errno, EINVAL);
    assert_true(fopen("", "r") == NULL);
    assert_int_equal(errno, EINVAL);
    assert_true(fopen("/nonexist_mountpoint/xxx.txt", "r") == NULL);
    assert_int_equal(errno, ENOENT);
    assert_true(fopen("/user/xxxx.txt", "r") == NULL);
    assert_int_equal(errno, ENOENT);
    assert_true(fopen("/os/xxxx.txt", "r") == NULL);
    assert_int_equal(errno, ENOENT);
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
    const char *files_to_check[] = {
        "/user/test003.bin",
        "/os/test004.bin"};
    static const size_t trunc_size = 256 * 1024;
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_check); ++fno)
    {
        const char *fname = files_to_check[fno];
        assert_int_equal(truncate(fname, trunc_size), 0);
    }
    //Open and check for sizes
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_check); ++fno)
    {
        const char *fname = files_to_check[fno];
        assert_int_equal(file_length(fname), trunc_size);
    }
    // Unlink
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_check); ++fno)
    {
        const char *fname = files_to_check[fno];
        assert_int_equal(unlink(fname), 0);
    }
    //Check for sizes again
    for (size_t fno = 0; fno < ARRAY_SIZE(files_to_check); ++fno)
    {
        const char *fname = files_to_check[fno];
        assert_int_equal(file_length(fname), -1);
        assert_int_equal(errno, ENOENT);
    }
    //Unlink unexistient
    assert_int_equal(unlink("/os/non_existientfile"), -1);
    assert_int_equal(errno, ENOENT);
}

// Tests for directory create removal and stat
static void test_directory_create_remove_stat_base(const char *basedir)
{
    char path[96];
    snprintf(path, sizeof path, "%s/dirtest", basedir);
    assert_int_equal(mkdir(path, 0755), 0);
    for (size_t d = 0; d < 20; ++d)
    {
        snprintf(path, sizeof path, "%s/dirtest/dir%i", basedir, d);
        assert_int_equal(mkdir(path, 0755), 0);
    }
    snprintf(path, sizeof path, "%s/dirtest/dir2/filx", basedir);
    assert_int_equal(truncate(path, 16384), 0);
    // Check stat for file and dir
    struct stat st;
    assert_int_equal(stat(path, &st), 0);
    assert_true(S_ISREG(st.st_mode));
    assert_int_equal(st.st_size, 16384);
    assert_int_equal(unlink(path), 0);
    for (size_t d = 0; d < 20; ++d)
    {
        snprintf(path, sizeof path, "%s/dirtest/dir%i", basedir, d);
        assert_int_equal(stat(path, &st), 0);
        assert_true(S_ISDIR(st.st_mode));
    }
    //Try to remove the directories
    for (size_t d = 0; d < 20; ++d)
    {
        snprintf(path, sizeof path, "%s/dirtest/dir%i", basedir, d);
        assert_int_equal(unlink(path), 0);
    }
    // Last dir remove
    snprintf(path, sizeof path, "%s/dirtest", basedir);
    assert_int_equal(unlink(path), 0);
}

static void test_directory_create_remove_stat_lfs(void)
{
    test_directory_create_remove_stat_base("/user");
}

static void test_directory_create_remove_stat_vfat(void)
{
    test_directory_create_remove_stat_base("/os");
}

static void test_dir_travesal_intervfs(void)
{
    assert_true(opendir("/kupa") == NULL);
    assert_int_equal(ENOENT, errno);
    assert_true(opendir("/os/xxx") == NULL);
    assert_int_equal(ENOENT, errno);
    assert_true(opendir("/user/xxx") == NULL);
    assert_int_equal(ENOENT, errno);
    //Traverse inter mount point
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
    assert_int_equal(mkdir(path, 0755), 0);
    for (size_t d = 0; d < 20; ++d)
    {
        snprintf(path, sizeof path, "%s/dirtest/dir%i", basedir, d);
        assert_int_equal(mkdir(path, 0755), 0);
    }
    snprintf(path, sizeof path, "%s/dirtest/filx", basedir);
    assert_int_equal(truncate(path, 16384), 0);

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
                assert_ulong_equal(16384, dent->d_size);
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
        snprintf(path, sizeof path, "%s/dirtest/dir%i", basedir, d);
        assert_int_equal(unlink(path), 0);
    }
    snprintf(path, sizeof path, "%s/dirtest/filx", basedir);
    assert_int_equal(unlink(path), 0);
    // Last dir remove
    snprintf(path, sizeof path, "%s/dirtest", basedir);
    assert_int_equal(unlink(path), 0);
}

void test_dir_traversal_lfs(void)
{
    test_dir_traversal("/user");
}

void test_dir_traversal_vfat(void)
{
    test_dir_traversal("/os");
}

// VFS test fixutre
void test_fixture_vfs()
{
    test_fixture_start();
    run_test(test_basic_file_read_api);
    run_test(test_basic_write_files);
    run_test(test_failed_to_open_files);
    run_test(test_create_and_remove_files);
    run_test(test_directory_create_remove_stat_lfs);
    run_test(test_directory_create_remove_stat_vfat);
    run_test(test_dir_travesal_intervfs);
    run_test(test_dir_traversal_lfs);
    run_test(test_dir_traversal_vfat);
    test_fixture_end();
}
