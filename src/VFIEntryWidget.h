#pragma once

#include "VFI.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>


class VFIEntryWidget : public QWidget
{
    Q_OBJECT

public:
    VFIEntryWidget(VFI *vfi);
    ~VFIEntryWidget();

    QHBoxLayout* lLayout;
    VFI* vfi;

    QPushButton* pbOpen;
    QPushButton* pbReplace;
    //QPushButton* pbDelete;

signals:
    void signalOpen(VFI* vfi);
    void signalReplace(VFI* vfi);
    //void signalDelete(VFI* vfi);

public slots:
    void slotOpen();
    void slotReplace();
};
