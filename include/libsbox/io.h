/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#ifndef LIBSBOX_IO_H
#define LIBSBOX_IO_H

#include <string>

namespace libsbox {
    class io_stream {
    public:
        int fd = -1;
        std::string filename;
    public:
        void freopen(const std::string &filename);
        void reset();
        friend class execution_context;
    };

    class in_stream : public io_stream {};
    class out_stream : public io_stream {};

    struct io_pipe {
        out_stream *write_end;
        in_stream *read_end;
        int extra_flags;
    };
} // namespace libsbox

#endif //LIBSBOX_IO_H
