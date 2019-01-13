#pragma once

#include <QMainWindow>
#include "header.h"
#include "RDBAFake.h"
#include "VFI.h"
#include <QLayout>
#include <QVBoxLayout>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString device, QWidget *parent = 0);
    ~MainWindow();
    RDBAFake* rawBlockDeviceAccessor;
    VFI* getRootVFI(ubyte1 deviceID);

    bool findFreeINode(int &freeINode);

    int finalPositionFIT(int inode, int offset);
    int finalPositionFDT(int inode, int offset);

    /* PRIVATE, ONLY CLASS USES THIS: */ int finalPosition(int block, int offset);


    //int finalPositionFDT(VFI* vfi, int offset);


    ubyte1 readByte(int finalPosition);
    bool writeByte(int finalPosition, ubyte1 value);

    List<VFI*>* vfiList;
    QVBoxLayout* l;



public slots:
    void slotOpenVFI(VFI* vfi);
    void slotReplaceVFI(VFI* vfi);
    void slotDeleteVFI(VFI* vfi);
    void slotFormatDisk();
    void slotAddFile();


private:
    Ui::MainWindow *ui;
};
