#include <unistd.h>

#include "ProcessManager.h"

int main(/*int argc, char *argv[]*/)
{
    Process::ProcessScheduler p;

    p.LaunchProcess("/home/mark/tmp/cpp/main", "", "main");
    p.LaunchProcess("/home/mark/tmp/cpp/main2", "", "main2");
    p.LaunchProcess("/home/mark/tmp/cpp/main3", "", "main3");
    p.LaunchProcess("/home/mark/tmp/cpp/main4", "", "main4");

    sleep(500);

    return 0;
}
