#include "mainwindow.h"
#include <QApplication>
#include <iostream>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (argc < 2)
    {
        std::cerr << "Enter a valid device to work with as parameter!";
        return 0;
    }

    MainWindow w(argv[1]);
    w.show();

    return a.exec();
}
