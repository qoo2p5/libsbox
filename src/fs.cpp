/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#include <libsbox/fs.h>
#include <libsbox/die.h>

#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <ftw.h>
#include <random>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sendfile.h>

int libsbox::get_file_type(const std::string &path) {
    struct stat st = {};
    if (stat(path.c_str(), &st) < 0) {
        if (errno == ENOENT) return 0;
        die("Cannot stat %s (%s)", path.c_str(), strerror(errno));
    }
    return (st.st_mode & S_IFMT);
}

void libsbox::make_file(std::string path, int rules, int file_rules) {
    if (path.empty()) die("make_path() with empty path");

    auto iter = path.begin();
    while (*iter == '/') iter++;

    while (iter < path.end()) {
        iter = std::find(iter, path.end(), '/');
        if (iter != path.end()) {
            *iter = 0;
            if (mkdir(path.c_str(), rules) < 0 && errno != EEXIST) {
                die("Cannot create directory %s (%s)", path.c_str(), strerror(errno));
            }
            *iter = '/';
            iter++;
        } else {
            int fd = creat(path.c_str(), file_rules);
            if (fd < 0) {
                die("Cannot create file %s (%s)", path.c_str(), strerror(errno));
            }
            if (close(fd) != 0) {
                die("Cannot create file %s: failed to close fd (%s)", path.c_str(), strerror(errno));
            }
            break;
        }
    }

    int file_type = get_file_type(path);
    if (!S_ISREG(file_type)) {
        die("Cannot create file %s: file not created", path.c_str());
    }
}

void libsbox::make_path(std::string path, int rules) {
    if (path.empty()) die("make_path() with empty path");

    auto iter = path.begin();
    while (*iter == '/') iter++;

    while (iter < path.end()) {
        iter = std::find(iter, path.end(), '/');
        if (iter != path.end()) *iter = 0;

        if (mkdir(path.c_str(), rules) < 0 && errno != EEXIST) {
            die("Cannot create directory %s (%s)", path.c_str(), strerror(errno));
        }

        if (iter == path.end()) break;
        *iter = '/';
        iter++;
    }

    int file_type = get_file_type(path);
    if (!S_ISDIR(file_type)) {
        die("Cannot create directory %s: directory not created", path.c_str());
    }
}

const int NAME_LEN = 15;
const int NUM_TRIES = 10;
const char ALPHA[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const int ALPHA_LEN = strlen(ALPHA);

/*
 * Returns created dir name
 */
std::string libsbox::make_temp_dir(std::string prefix, int rules) {
    if (prefix.empty()) {
        prefix = ".";
    }

    static std::random_device rd;
    static std::mt19937 mt(rd());

    int tries = 0;
    std::string name;
    while (tries < NUM_TRIES) {
        tries++;
        name.resize(NAME_LEN);
        for (int i = 0; i < NAME_LEN; ++i) {
            name[i] = ALPHA[mt() % ALPHA_LEN];
        }
        std::string path = join_path(prefix, name);

        int file_type = get_file_type(path);
        if (file_type) continue;

        make_path(path, rules);
        return name;
    }

    die("Failed to find free id within %d tries", NUM_TRIES);
}

void libsbox::copy_file(const std::string &source, const std::string &dest, int rules) {
    int fd_source = open(source.c_str(), O_RDONLY, 0);
    if (fd_source < 0) {
        libsbox::die("Copy %s -> %s failed: cannot open source file (%s)", source.c_str(), dest.c_str(), strerror(errno));
    }

    int fd_dest = creat(dest.c_str(), rules);
    if (fd_dest < 0) {
        libsbox::die("Copy %s -> %s failed: cannot create destination file (%s)", source.c_str(), dest.c_str(), strerror(errno));
    }

    struct stat stat_source = {};
    fstat(fd_source, &stat_source);

    sendfile(fd_dest, fd_source, nullptr, stat_source.st_size);

    if (close(fd_source) != 0) {
        libsbox::die("Copy %s -> %s failed: cannot close source file (%s)", source.c_str(), dest.c_str(), strerror(errno));
    }

    if (close(fd_dest) != 0) {
        libsbox::die("Copy %s -> %s failed: cannot close destination file (%s)", source.c_str(), dest.c_str(), strerror(errno));
    }
}
std::string libsbox::find_executable(std::string name) {
    if (name[0] == '.') return name;
    if (name[0] == '/') return name;
    char *path = strdup(getenv("PATH"));
    if (path != nullptr) {
        const char *dir;
        for (dir = strtok(path, ";"); dir != nullptr; dir = strtok(nullptr, ";")) {
            std::string result = join_path(dir, name);
            if (get_file_type(result)) {
                free(path);
                return result;
            }
        }
    }

    free(path);
    libsbox::die("Executable %s not found in PATH or PATH not set", name.c_str());
}
