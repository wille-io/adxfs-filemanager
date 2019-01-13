#include "VFIEntryWidget.h"
#include <QPushButton>
#include <QLabel>
#include <QDebug>


VFIEntryWidget::VFIEntryWidget(VFI* vfi)
    : vfi(vfi)
{    
    lLayout = new QHBoxLayout();

    this->setLayout(lLayout);

    pbOpen = new QPushButton("Open");
    connect(pbOpen, SIGNAL(clicked()), this, SLOT(slotOpen()));

    pbReplace = new QPushButton("Replace");
    connect(pbReplace, SIGNAL(clicked()), this, SLOT(slotReplace()));

    pbDelete = new QPushButton("Delete");
    connect(pbDelete, SIGNAL(clicked()), this, SLOT(slotDelete()));


    lLayout->addWidget(new QLabel(vfi->nodeName));
    lLayout->addWidget(pbOpen);
    lLayout->addWidget(pbReplace);
    lLayout->addWidget(pbDelete);
}

void VFIEntryWidget::slotOpen()
{
    emit signalOpen(vfi);
}

void VFIEntryWidget::slotReplace()
{
    emit signalReplace(vfi);
}

void VFIEntryWidget::slotDelete()
{
    emit signalDelete(vfi);
}

VFIEntryWidget::~VFIEntryWidget()
{

}

