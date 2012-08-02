#include <QApplication>
#include "ZBarReaderTest.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ZBarReaderTest zbarTest;
    zbarTest.show();
    
    return app.exec();
}
