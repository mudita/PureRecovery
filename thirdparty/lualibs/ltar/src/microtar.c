/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "microtar.h"

typedef struct {
    char name[100];
    char mode[8];
    char owner[8];
    char group[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char _padding[255];
} mtar_raw_header_t;


static unsigned round_up(unsigned n, unsigned incr) {
    return n + (incr - n % incr) % incr;
}


static unsigned checksum(const mtar_raw_header_t *rh) {
    unsigned i;
    unsigned char *p = (unsigned char *) rh;
    unsigned res = 256;
    for (i = 0; i < offsetof(mtar_raw_header_t, checksum); i++) {
        res += p[i];
    }
    for (i = offsetof(mtar_raw_header_t, type); i < sizeof(*rh); i++) {
        res += p[i];
    }
    return res;
}


static int tread(mtar_t *tar, void *data, unsigned size) {
    int err = tar->read(tar, data, size);
    tar->pos += size;
    return err;
}


static int twrite(mtar_t *tar, const void *data, unsigned size) {
    int err = tar->write(tar, data, size);
    tar->pos += size;
    return err;
}


static int write_null_bytes(mtar_t *tar, int n) {
    int i, err;
    char nul = '\0';
    for (i = 0; i < n; i++) {
        err = twrite(tar, &nul, 1);
        if (err) {
            return err;
        }
    }
    return MTAR_ESUCCESS;
}

static int check_final_segment(mtar_t *tar) {
    int i, err;
    char nul;
    tar->seek(tar, -1024, SEEK_END);
    for (i = 0; i < 1024; i++) {
        err = tar->read(tar, &nul, 1);
        if (err) {
            return err;
        }
        if (nul != '\0') {
            return MTAR_ENOTFOUND;
        }
    }
    return MTAR_ESUCCESS;
}

static bool check_valid_header(mtar_t *tar) {
    mtar_header_t h = {};
    return mtar_read_header(tar, &h);
}


static int raw_to_header(mtar_header_t *h, const mtar_raw_header_t *rh) {
    unsigned chksum1, chksum2;

    /* If the checksum starts with a null byte we assume the record is NULL */
    if (*rh->checksum == '\0') {
        return MTAR_ENULLRECORD;
    }

    /* Build and compare checksum */
    chksum1 = checksum(rh);
    sscanf(rh->checksum, "%o", &chksum2);
    if (chksum1 != chksum2) {
        return MTAR_EBADCHKSUM;
    }

    /* Load raw header into header */
    sscanf(rh->mode, "%o", &h->mode);
    sscanf(rh->owner, "%o", &h->owner);
    sscanf(rh->size, "%o", &h->size);
    sscanf(rh->mtime, "%o", &h->mtime);
    h->type = rh->type;
    strcpy(h->name, rh->name);
    strcpy(h->linkname, rh->linkname);

    return MTAR_ESUCCESS;
}


static int header_to_raw(mtar_raw_header_t *rh, const mtar_header_t *h) {
    unsigned chksum;

    /* Load header into raw header */
    memset(rh, 0, sizeof(*rh));
    sprintf(rh->mode, "%o", h->mode);
    sprintf(rh->owner, "%o", h->owner);
    sprintf(rh->size, "%o", h->size);
    sprintf(rh->mtime, "%o", h->mtime);
    rh->type = h->type ? h->type : MTAR_TREG;
    strcpy(rh->name, h->name);
    strcpy(rh->linkname, h->linkname);

    /* Calculate and write checksum */
    chksum = checksum(rh);
    sprintf(rh->checksum, "%06o", chksum);
    rh->checksum[7] = ' ';

    return MTAR_ESUCCESS;
}


const char *mtar_strerror(int err) {
    switch (err) {
        case MTAR_ESUCCESS     :
            return "success";
        case MTAR_EFAILURE     :
            return "failure";
        case MTAR_EOPENFAIL    :
            return "could not open";
        case MTAR_EREADFAIL    :
            return "could not read";
        case MTAR_EWRITEFAIL   :
            return "could not write";
        case MTAR_ESEEKFAIL    :
            return "could not seek";
        case MTAR_EBADCHKSUM   :
            return "bad checksum";
        case MTAR_ENULLRECORD  :
            return "null record";
        case MTAR_ENOTFOUND    :
            return "file not found";
    }
    return "unknown error";
}


static int file_write(mtar_t *tar, const void *data, unsigned size) {
    unsigned res = fwrite(data, 1, size, tar->stream);
    return (res == size) ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
}

static int file_read(mtar_t *tar, void *data, unsigned size) {
    unsigned res = fread(data, 1, size, tar->stream);
    return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}

static int file_seek(mtar_t *tar, long offset, int mode) {
    int res = fseek(tar->stream, offset, mode);
    return (res == 0) ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
}

static long file_tell(mtar_t *tar) {
    return ftell(tar->stream);
}

static int file_close(mtar_t *tar) {
    fclose(tar->stream);
    return MTAR_ESUCCESS;
}


int mtar_open(mtar_t *tar, const char *filename, const char *mode) {
    int err = MTAR_ESUCCESS;

    memset(tar, 0, sizeof(*tar));
    tar->write = file_write;
    tar->read = file_read;
    tar->seek = file_seek;
    tar->close = file_close;
    tar->tell = file_tell;

    if ((*mode == 'a') || (*mode == 'r')) {
        tar->stream = fopen(filename, "rb");
        if (!tar->stream) {
            return MTAR_EOPENFAIL;
        }
        err = check_valid_header(tar);
        if (err != MTAR_ESUCCESS) {
            goto error;
        }
        err = check_final_segment(tar);
        if (err == MTAR_ENOTFOUND) {
            tar->close(tar);
            char n_mode[2] = {*mode, 'b'};
            tar->stream = fopen(filename, n_mode);
        } else if (err == MTAR_ESUCCESS) {
            if (*mode == 'a') {
                tar->close(tar);
                tar->stream = fopen(filename, "rb+");
                tar->seek(tar, -1024, SEEK_END);
                tar->pos = tar->tell(tar);
            } else {
                tar->seek(tar, 0, SEEK_SET);
            }
        } else {
            goto error;
        }
    } else {
        tar->stream = fopen(filename, "wb");
        if (!tar->stream) {
            goto error;
        }
    }
    return MTAR_ESUCCESS;

    error:
    if (tar->stream != NULL) {
        mtar_close(tar);
    }
    return err;
}


int mtar_close(mtar_t *tar) {
    return tar->close(tar);
}


int mtar_seek(mtar_t *tar, unsigned pos) {
    int err = tar->seek(tar, pos, SEEK_SET);
    tar->pos = pos;
    return err;
}


int mtar_rewind(mtar_t *tar) {
    tar->remaining_data = 0;
    tar->last_header = 0;
    return mtar_seek(tar, 0);
}


int mtar_next(mtar_t *tar) {
    int err, n;
    mtar_header_t h;
    /* Load header */
    err = mtar_read_header(tar, &h);
    if (err) {
        return err;
    }
    /* Seek to next record */
    n = round_up(h.size, 512) + sizeof(mtar_raw_header_t);
    return mtar_seek(tar, tar->pos + n);
}


int mtar_find(mtar_t *tar, const char *name, mtar_header_t *h) {
    int err;
    mtar_header_t header;
    /* Start at beginning */
    err = mtar_rewind(tar);
    if (err) {
        return err;
    }
    /* Iterate all files until we hit an error or find the file */
    while ((err = mtar_read_header(tar, &header)) == MTAR_ESUCCESS) {
        if (!strcmp(header.name, name)) {
            if (h) {
                *h = header;
            }
            return MTAR_ESUCCESS;
        }
        mtar_next(tar);
    }
    /* Return error */
    if (err == MTAR_ENULLRECORD) {
        err = MTAR_ENOTFOUND;
    }
    return err;
}


int mtar_read_header(mtar_t *tar, mtar_header_t *h) {
    int err;
    mtar_raw_header_t rh;
    /* Save header position */
    tar->last_header = tar->pos;
    /* Read raw header */
    err = tread(tar, &rh, sizeof(rh));
    if (err) {
        return err;
    }
    /* Seek back to start of header */
    err = mtar_seek(tar, tar->last_header);
    if (err) {
        return err;
    }
    /* Load raw header into header struct and return */
    return raw_to_header(h, &rh);
}


int mtar_read_data(mtar_t *tar, void *ptr, unsigned size) {
    int err;
    /* If we have no remaining data then this is the first read, we get the size,
     * set the remaining data and seek to the beginning of the data */
    if (tar->remaining_data == 0) {
        mtar_header_t h;
        /* Read header */
        err = mtar_read_header(tar, &h);
        if (err) {
            return err;
        }
        /* Seek past header and init remaining data */
        err = mtar_seek(tar, tar->pos + sizeof(mtar_raw_header_t));
        if (err) {
            return err;
        }
        tar->remaining_data = h.size;
    }
    /* Read data */
    err = tread(tar, ptr, size);
    if (err) {
        return err;
    }
    tar->remaining_data -= size;
    /* If there is no remaining data we've finished reading and seek back to the
     * header */
    if (tar->remaining_data == 0) {
        return mtar_seek(tar, tar->last_header);
    }
    return MTAR_ESUCCESS;
}


int mtar_write_header(mtar_t *tar, const mtar_header_t *h) {
    mtar_raw_header_t rh;
    /* Build raw header and write */
    header_to_raw(&rh, h);
    tar->remaining_data = h->size;
    return twrite(tar, &rh, sizeof(rh));
}


int mtar_write_file_header(mtar_t *tar, const char *name, unsigned size) {
    mtar_header_t h;
    /* Build header */
    memset(&h, 0, sizeof(h));
    strcpy(h.name, name);
    h.size = size;
    h.type = MTAR_TREG;
    h.mode = 0664;
    /* Write header */
    return mtar_write_header(tar, &h);
}


int mtar_write_dir_header(mtar_t *tar, const char *name) {
    mtar_header_t h;
    /* Build header */
    memset(&h, 0, sizeof(h));
    strcpy(h.name, name);
    h.type = MTAR_TDIR;
    h.mode = 0775;
    /* Write header */
    return mtar_write_header(tar, &h);
}


int mtar_write_data(mtar_t *tar, const void *data, unsigned size) {
    int err;
    /* Write data */
    err = twrite(tar, data, size);
    if (err) {
        return err;
    }
    tar->remaining_data -= size;
    /* Write padding if we've written all the data for this file */
    if (tar->remaining_data == 0) {
        return write_null_bytes(tar, round_up(tar->pos, 512) - tar->pos);
    }
    return MTAR_ESUCCESS;
}


int mtar_finalize(mtar_t *tar) {
    /* Write two NULL records */
    return write_null_bytes(tar, sizeof(mtar_raw_header_t) * 2);
}
