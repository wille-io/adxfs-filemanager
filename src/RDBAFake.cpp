#include "RDBAFake.h"
#include "QDebug"
#include <iostream>


RDBAFake::RDBAFake()
{
    file = new QFile("/dev/mmcblk0");
    //file = new QFile("/dev/sdb");

    if(!file->open(QFile::ReadOnly))
    {
        qDebug() <<"FUCK! " << file->errorString();
    }
    else
    {
        qDebug() <<"nice :)   ";
        ba = file->read(20*1024*1024); // read maximum of 20 MB

        // as a sd card is a sequential device, which is totally hardcore mega bullshit, we need to buffer a shitload of space. ram = fucked

        //QByteArray
    }

}

ubyte1 RDBAFake::getByte(int ignoredDeviceID, int position)
{
    //qDebug() << "getByte" << position;
    (void)ignoredDeviceID; // ingore
    //file->seek(position);
    //qDebug() << "getByte: " << (int)ba[(int)position]; // glad that int is bigger here than on arduino
    return ba[position];
}

void RDBAFake::setByte(ubyte1 deviceType, int position, ubyte1 val)
{
    //qDebug() << "setByte" << position;
    (void)deviceType; // ignore
    ba[position] = val;
}

RDBAFake::~RDBAFake()
{
    file->close(); // this is a sequential device, hahahahahhhahah, so we must reopen it
    delete file;

    file = new QFile("/dev/mmcblk0");
    if (!file->open(QFile::ReadWrite | QFile::Unbuffered))
    {
        qDebug() << "Could not save changes!";
    }


    /*file->seek(0);

    for (int i = 0; i < 256*256*512+1000; i++)
    {
        char x = ba[i];
        file->write(&x, 1);
    }*/

    // TODO: look if there are changes (reget sd values and operator= with both bas)

    qDebug() << "Last byte of" << 33687054    << " is (int)" << (int)ba[33687054];
    qDebug() << "Last byte of" << ba.size()-1 << " is (int)" << (int)ba[ba.size()-1];

    qDebug() << "wrote" << file->write(ba) << "bytes!"; // write all changes to sd card
    file->flush();
    file->close();
    delete file;
}

