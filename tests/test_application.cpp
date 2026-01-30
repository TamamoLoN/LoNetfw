#include "lonetfw/lonetfw.h"

int main(int argc, char *argv[])
{
    if (lon::system::Application::Instance().init(argc, argv))
    {
        return lon::system::Application::Instance().run();
    }
    return 0;
}
