/*
 * Copyright (c) 2019 Andrei Odintsov <forestryks1@gmail.com>
 */

#ifndef LIBSBOX_EXECUTION_TARGET_H
#define LIBSBOX_EXECUTION_TARGET_H

#include <libsbox/io.h>
#include <libsbox/bind.h>
#include <libsbox/cgroup.h>

#include <vector>
#include <string>
#include <map>
#include <unistd.h>

namespace libsbox {
    class execution_target;

    extern execution_target *current_target;
} // namespace libsbox

class libsbox::execution_target {
public:
    long time_limit = -1;
    long memory_limit = -1;
    long fsize_limit = -1;
    int max_files = 64;
    int max_threads = 1;

    in_stream stream_stdin;
    out_stream stream_stdout, stream_stderr;

    // stats
    long time_usage = 0;
    long time_usage_sys = 0;
    long time_usage_user = 0;
    long wall_time_usage = 0;
    long memory_usage = 0;

    bool time_limit_exceeded = false;
    bool wall_time_limit_exceeded = false;
    bool exited = false;
    int exit_code = -1;
    bool signaled = false;
    int term_signal = -1;
    bool oom_killed = false;

    void add_standard_rules();
    void add_bind_rule(const std::string &, const std::string &, int);

    execution_target(int, char **);
    explicit execution_target(const std::vector<std::string> &);
    ~execution_target();
private:
    char **argv = nullptr;
    char **env = nullptr;

    std::vector<bind_rule> bind_rules;

    cgroup_controller *cpuacct_controller = nullptr, *memory_controller = nullptr;

    uid_t uid = 0;

    pid_t proxy_pid = 0;
    pid_t slave_pid = 0;
    int status_pipe[2] = {-1, -1};
    std::string id;
    bool inside_box = false;
    int exec_fd = -1;
    bool proxy_killed = false;
    bool running = false;

    void init();
    void die();

    void prepare();
    void cleanup();

    int proxy();
    void start_proxy();
    void prepare_root();
    void destroy_root();

    [[noreturn]]
    void slave();
    void freopen_fds();
    void dup2_fds();
    void close_all_fds();
    void setup_rlimits();
    void setup_credentials();

    long get_time_usage();
    long get_time_usage_sys();
    long get_time_usage_user();
    long get_memory_usage();
    bool get_oom_status();

    friend int clone_callback(void *);
    friend void die(const char *msg, ...);
    friend class execution_context;
};

#endif //LIBSBOX_EXECUTION_TARGET_H
