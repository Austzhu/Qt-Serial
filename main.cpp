#include "Singleup.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");
    //a.setStyle("macintosh");
    SingleUp w;
    w.show();

    return a.exec();
}
