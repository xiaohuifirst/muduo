//
// Created by hhp on 18-6-21.
//

#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>

#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

off_t kRollSize = 1200*1000*1000;

muduo::AsyncLogging* g_asyncLog = nullptr;

void asyncOutPut(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

void bench(bool longLog)
{
    muduo::Logger::setOutput(asyncOutPut);

    const int threadNum = 1;
    std::vector<double> timeSpends(threadNum);
    // muduo库的thread类不支持拷贝或移动操作，与std::thread不同
    boost::ptr_vector<muduo::Thread> threads;

    for(int t=0; t<threadNum; ++t) {
        threads.push_back(new muduo::Thread([t, &timeSpends] {

            muduo::string longStr(100,'X');
            longStr += " ";
            muduo::Timestamp start = muduo::Timestamp::now();
            for (int i = 0; i < 1000*1000*100; ++i) {
                LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz " << longStr;
            }
            muduo::Timestamp end = muduo::Timestamp::now();
            timeSpends[t] = muduo::timeDifference(end, start);
        }));

        threads.back().start();
    }

    for(int t=0; t< threadNum; ++t){
        threads[t].join();
        printf("%f s\n", timeSpends[t]);
    }
}

int main(int argc, char* argv[])
{
    {
        // set max virtual memory to 2GB
        size_t kOneGB = 1000*1024*1024;
        rlimit rl = {2*kOneGB,2*kOneGB};
        setrlimit(RLIMIT_AS,&rl);
    }

    printf("pid = %d\n", getpid());

    char name[256];
    strncpy(name,argv[0], 256);

    muduo::AsyncLogging log(::basename(name),kRollSize);
    log.start();

    g_asyncLog = &log;
    bench(true);
}