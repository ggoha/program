#include <QtGui>
#include "UdpServer.h"

// ----------------------------------------------------------------------
int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    UdpServer    server;

    server.show();

    return app.exec();
}
