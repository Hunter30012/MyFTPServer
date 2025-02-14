#include "server_controller.h"

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");

    ServerController serverController(argc, argv);
    return serverController.init();
}
