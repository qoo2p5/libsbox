/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#ifndef LIBSBOX_FS_H
#define LIBSBOX_FS_H

#include <string>

namespace libsbox {
    inline std::string join_path(const std::string &path) {
        return path;
    }

    template<typename... types>
    std::string join_path(const std::string &path1, types... paths) {
        std::string path2 = join_path(paths...);
        if (path1.back() == '/' || path2[0] == '/') return path1 + path2;
        return path1 + "/" + path2;
    }

    int get_file_type(const std::string &path);
    void make_path(std::string, int rules = 0755);
    void make_file(std::string, int rules = 0755, int file_rules = 0644);
    std::string make_temp_dir(std::string, int rules = 0755);
    void copy_file(const std::string &, const std::string &, int rules = 0644);
    std::string find_executable(std::string name);
} // namespace libsbox

#endif //LIBSBOX_FS_H
