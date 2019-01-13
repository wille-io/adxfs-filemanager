#pragma once

#include <QFile>
#include "header.h"


class RDBAFake
{
public:
    RDBAFake(QString file);
    ~RDBAFake();

    QString filename;
    QFile* file;
    ubyte1 getByte(int ingnoredDeviceID, int position);
    void setByte(ubyte1 deviceType, int position, ubyte1 val);
    QByteArray ba;
};
