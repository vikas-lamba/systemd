/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

/***
  This file is part of systemd.

  Copyright 2010-2012 Lennart Poettering
***/

#include <alloca.h>
#include <stdbool.h>
#include <stddef.h>

#include "macro.h"
#include "string-util.h"
#include "time-util.h"

#define PATH_SPLIT_SBIN_BIN(x) x "sbin:" x "bin"
#define PATH_SPLIT_SBIN_BIN_NULSTR(x) x "sbin\0" x "bin\0"

#define PATH_NORMAL_SBIN_BIN(x) x "bin"
#define PATH_NORMAL_SBIN_BIN_NULSTR(x) x "bin\0"

#if HAVE_SPLIT_BIN
#  define PATH_SBIN_BIN(x) PATH_SPLIT_SBIN_BIN(x)
#  define PATH_SBIN_BIN_NULSTR(x) PATH_SPLIT_SBIN_BIN_NULSTR(x)
#else
#  define PATH_SBIN_BIN(x) PATH_NORMAL_SBIN_BIN(x)
#  define PATH_SBIN_BIN_NULSTR(x) PATH_NORMAL_SBIN_BIN_NULSTR(x)
#endif

#define DEFAULT_PATH_NORMAL PATH_SBIN_BIN("/usr/local/") ":" PATH_SBIN_BIN("/usr/")
#define DEFAULT_PATH_NORMAL_NULSTR PATH_SBIN_BIN_NULSTR("/usr/local/") PATH_SBIN_BIN_NULSTR("/usr/")
#define DEFAULT_PATH_SPLIT_USR DEFAULT_PATH_NORMAL ":" PATH_SBIN_BIN("/")
#define DEFAULT_PATH_SPLIT_USR_NULSTR DEFAULT_PATH_NORMAL_NULSTR PATH_SBIN_BIN_NULSTR("/")
#define DEFAULT_PATH_COMPAT PATH_SPLIT_SBIN_BIN("/usr/local/") ":" PATH_SPLIT_SBIN_BIN("/usr/") ":" PATH_SPLIT_SBIN_BIN("/")

#if HAVE_SPLIT_USR
#  define DEFAULT_PATH DEFAULT_PATH_SPLIT_USR
#  define DEFAULT_PATH_NULSTR DEFAULT_PATH_SPLIT_USR_NULSTR
#else
#  define DEFAULT_PATH DEFAULT_PATH_NORMAL
#  define DEFAULT_PATH_NULSTR DEFAULT_PATH_NORMAL_NULSTR
#endif

bool is_path(const char *p) _pure_;
int path_split_and_make_absolute(const char *p, char ***ret);
bool path_is_absolute(const char *p) _pure_;
char* path_make_absolute(const char *p, const char *prefix);
int safe_getcwd(char **ret);
int path_make_absolute_cwd(const char *p, char **ret);
int path_make_relative(const char *from_dir, const char *to_path, char **_r);
char* path_kill_slashes(char *path);
char* path_startswith(const char *path, const char *prefix) _pure_;
int path_compare(const char *a, const char *b) _pure_;
bool path_equal(const char *a, const char *b) _pure_;
bool path_equal_or_files_same(const char *a, const char *b, int flags);
char* path_join(const char *root, const char *path, const char *rest);

static inline bool path_equal_ptr(const char *a, const char *b) {
        return !!a == !!b && (!a || path_equal(a, b));
}

/* Note: the search terminates on the first NULL item. */
#define PATH_IN_SET(p, ...)                                     \
        ({                                                      \
                char **s;                                       \
                bool _found = false;                            \
                STRV_FOREACH(s, STRV_MAKE(__VA_ARGS__))         \
                        if (path_equal(p, *s)) {                \
                               _found = true;                   \
                               break;                           \
                        }                                       \
                _found;                                         \
        })

#define PATH_STARTSWITH_SET(p, ...)                             \
        ({                                                      \
                char **s;                                       \
                bool _found = false;                            \
                STRV_FOREACH(s, STRV_MAKE(__VA_ARGS__))         \
                        if (path_startswith(p, *s)) {           \
                               _found = true;                   \
                               break;                           \
                        }                                       \
                _found;                                         \
        })

int path_strv_make_absolute_cwd(char **l);
char** path_strv_resolve(char **l, const char *root);
char** path_strv_resolve_uniq(char **l, const char *root);

int find_binary(const char *name, char **filename);

bool paths_check_timestamp(const char* const* paths, usec_t *paths_ts_usec, bool update);

int fsck_exists(const char *fstype);
int mkfs_exists(const char *fstype);

/* Iterates through the path prefixes of the specified path, going up
 * the tree, to root. Also returns "" (and not "/"!) for the root
 * directory. Excludes the specified directory itself */
#define PATH_FOREACH_PREFIX(prefix, path) \
        for (char *_slash = ({ path_kill_slashes(strcpy(prefix, path)); streq(prefix, "/") ? NULL : strrchr(prefix, '/'); }); _slash && ((*_slash = 0), true); _slash = strrchr((prefix), '/'))

/* Same as PATH_FOREACH_PREFIX but also includes the specified path itself */
#define PATH_FOREACH_PREFIX_MORE(prefix, path) \
        for (char *_slash = ({ path_kill_slashes(strcpy(prefix, path)); if (streq(prefix, "/")) prefix[0] = 0; strrchr(prefix, 0); }); _slash && ((*_slash = 0), true); _slash = strrchr((prefix), '/'))

char *prefix_root(const char *root, const char *path);

/* Similar to prefix_root(), but returns an alloca() buffer, or
 * possibly a const pointer into the path parameter */
#define prefix_roota(root, path)                                        \
        ({                                                              \
                const char* _path = (path), *_root = (root), *_ret;     \
                char *_p, *_n;                                          \
                size_t _l;                                              \
                while (_path[0] == '/' && _path[1] == '/')              \
                        _path ++;                                       \
                if (empty_or_root(_root))                               \
                        _ret = _path;                                   \
                else {                                                  \
                        _l = strlen(_root) + 1 + strlen(_path) + 1;     \
                        _n = alloca(_l);                                \
                        _p = stpcpy(_n, _root);                         \
                        while (_p > _n && _p[-1] == '/')                \
                                _p--;                                   \
                        if (_path[0] != '/')                            \
                                *(_p++) = '/';                          \
                        strcpy(_p, _path);                              \
                        _ret = _n;                                      \
                }                                                       \
                _ret;                                                   \
        })

int parse_path_argument_and_warn(const char *path, bool suppress_root, char **arg);

char* dirname_malloc(const char *path);
const char *last_path_component(const char *path);

bool filename_is_valid(const char *p) _pure_;
bool path_is_normalized(const char *p) _pure_;

char *file_in_same_dir(const char *path, const char *filename);

bool hidden_or_backup_file(const char *filename) _pure_;

bool is_device_path(const char *path);
bool is_deviceallow_pattern(const char *path);

int systemd_installation_has_version(const char *root, unsigned minimal_version);

bool dot_or_dot_dot(const char *path);

static inline const char *skip_dev_prefix(const char *p) {
        const char *e;

        /* Drop any /dev prefix if there is any */

        e = path_startswith(p, "/dev/");

        return e ?: p;
}

bool empty_or_root(const char *root);
static inline const char *empty_to_root(const char *path) {
        return isempty(path) ? "/" : path;
}
