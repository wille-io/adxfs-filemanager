#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    RDBAFake* rawBlockDeviceAccessor;
    VFI* getRootVFI(ubyte1 deviceID);

    int finalPosition(VFI* vfi, int offset, bool bullshit = true);
    ubyte1 readByte(int finalPosition);
    bool writeByte(int finalPosition, ubyte1 value);

    List<VFI*>* vfiList;
    QVBoxLayout* l;



public slots:
    void slotOpenVFI(VFI* vfi);
    void slotReplaceVFI(VFI* vfi);
    //void slotDeleteVFI(VFI* vfi);


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
