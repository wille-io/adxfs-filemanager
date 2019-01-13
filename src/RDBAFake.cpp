#include "RDBAFake.h"
#include "QDebug"
#include <iostream>


RDBAFake::RDBAFake(QString filename)
    : filename(filename)
{
    file = new QFile(filename);

    if(!file->open(QFile::ReadOnly))
    {
        qDebug() <<"noep!" << file->errorString();
        exit(1);
    }
    else
    {
        ba = file->read(20*1024*1024); // read maximum of 20 MB
        // as a sd card is a sequential device, which is totally hardcore mega bullshit, we need to buffer a shitload of space. ram = fucked
    }
}

ubyte1 RDBAFake::getByte(int ignoredDeviceID, int position)
{
    (void)ignoredDeviceID; // ingore
    return ba[position];
}

void RDBAFake::setByte(ubyte1 deviceType, int position, ubyte1 val)
{
    (void)deviceType; // ignore
    ba[position] = val;
}

RDBAFake::~RDBAFake()
{
    file->close(); // this is a sequential device, hahahahahhhahah, so we must reopen it
    delete file;

    file = new QFile(filename);
    if (!file->open(QFile::ReadWrite | QFile::Unbuffered))
    {
        qDebug() << "Could not save changes!" << file->errorString();
    }


    // TODO: look if there are changes (reget sd values and operator= with both bas)

    qDebug() << "wrote" << file->write(ba) << "bytes! do not forget to sync before removing device!"; // write all changes to sd card

    file->flush();
    file->close();
    delete file;
}
