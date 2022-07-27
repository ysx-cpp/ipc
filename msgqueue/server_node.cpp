#include "messages_factory.h"
#include <iostream>
#include <sys/stat.h>
#include <glog/logging.h>

uint32_t set_task_priority(const pthread_t pid, const int policy, uint32_t priority){
    int retcode = 0;
    struct sched_param param;
    param.sched_priority = priority;
    retcode = pthread_setschedparam(pid, policy, &param);
    return retcode;
}

void log_init(const char* program_name) {
    google::LogSeverity glog_level = google::GLOG_WARNING;
    struct stat stat_buf;
    bool  create_dir = false;
    if (stat("./log", &stat_buf) != 0) {
        create_dir = true;
    }
    if ((stat_buf.st_mode & S_IFDIR) != S_IFDIR) {

        remove("./log");
        create_dir = true;

    }
    if (create_dir) {

        mode_t mode = umask(0);

        int res = mkdir("./log", 0777);

        if (res != 0) {
            std::cout << "MAIN create ./log dir fail!" << std::endl;
        }
        umask(mode);

    }
    char *cur_dir = getcwd(NULL, 0);

    std::string temp(cur_dir);
    free(cur_dir);

    std::string log_dir = temp + "/log/";

    FLAGS_log_dir = log_dir.c_str();

    google::InitGoogleLogging(program_name);
    //ipc::pavaro::Module::instance()->init(log_dir, program_name);

    ::google::InstallFailureSignalHandler();
    FLAGS_stop_logging_if_full_disk = true;
    FLAGS_minloglevel = glog_level;
    FLAGS_alsologtostderr = true;

}

void log_uninit() {
    google::ShutdownGoogleLogging();
}
int main(int argc, char* argv[]) {
    std::string type = "zmq";
    uint16_t port    = 6000;
    std::string name = "ServerNode";
    if (argc == 1){
        port = 6000;
    }
    else if (argc == 3){
        type = argv[1];
        port = atoi(argv[2]);
    } else{
        std::cerr << argv[0] << " type " << " port " << std::endl;
        return -1;
    }

    log_init(name.c_str());


    pthread_t pid = pthread_self();
    uint32_t priority = 30;
    int policy = SCHED_RR;
    set_task_priority(pid, policy, priority);

    ipc::messages::MessageFactory factory(type);

    ipc::messages::IServerNode* server = factory.create_server(name, port);

    LOG_FIRST_N(ERROR, 2) << name << " start begin";

    server->start();

    LOG_FIRST_N(ERROR, 2) << name << " start end";
    LOG_FIRST_N(ERROR, 2) << name << " spin begin";
    server->spin();
    LOG_FIRST_N(ERROR, 2) << name << " spin end";

    log_uninit();
}
