#include <unistd.h>

#include "LinuxProcessManager.h"

int main(/*int argc, char *argv[]*/)
{
    Process::LinuxProcessManager p;

    p.LaunchProcess("/home/mark/tmp/cpp/main", "", "main");
    p.LaunchProcess("/home/mark/tmp/cpp/main2", "", "main2");

    sleep(3);

    p.LaunchProcess("/home/mark/tmp/cpp/main3", "", "main3");
    p.LaunchProcess("/home/mark/tmp/cpp/main4", "", "main4");

    p.Wait();

    return 0;
}
