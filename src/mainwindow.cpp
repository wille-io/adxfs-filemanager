#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QDebug>
#include <iostream>
#include <QPushButton>
#include <QLabel>
#include "VFIEntryWidget.h"
#include <QProcess>
#include <QFileDialog>


#define FIT_BLOCK_POS 2
#define FDT_BLOCK_POS 260


int devBlockSize = 512;
int fsBlockSize = 16;

VFI* MainWindow::getRootVFI(ubyte1 deviceID) // for initializers
{
    /*if (!initialized || !valid)
    {
        return 0;
    }
    else*/
    {
        /* the root vfi's nodename is always '/'
         * so create a vfi with nodename slashstring
         *
         * read all FITs and see which FITs have 0 (root) as parent and add them to the rootVFI
         * but begin at sektor 3. as we must ignore root (which starts at sektor 2) */

        VFI* rootVFI = new VFI;
        rootVFI->deviceID = deviceID;
        rootVFI->fileSize = 0;
        rootVFI->fileSystemType = 123; // ardunix::filesystem::filesystemtype::adxFS;
        rootVFI->fileType = 2; // ardunix::filesystem::filetype::directory;
        rootVFI->iNode = 0; // 0 = root
        rootVFI->lba = 0; // don't care
        rootVFI->nodeName = "/"; // ArdunixHeader::slashstring; // THIS STRING HAS TO BE REPLACED BY THE CALLER !! e.g. "sdcard"
        rootVFI->parent = 0; // no parent for now, the caller of getRootVFI has to set the parent !!!


        // ADD CHILDREN [CAUTION! " < 255" <= okay!    " <= 255" <= loop]
        for (ubyte1 fitBlock = 1; fitBlock < 255; fitBlock++) // ubyte1 enough, as there are only 256*256 FITs and ubyte1 can count from 0 - 255
        {
            int sdSector = fitBlock + 2; // get sector on sd card
            int pos = sdSector * devBlockSize; // convert sector to position
            ubyte1 fitParent = rawBlockDeviceAccessor->getByte(deviceID, pos); // byte 0 of current fit

            if (fitParent == 0) // only get FITs with parent 0 (root) [uhm.. y?]
            {
                ubyte1 filenameSize = 0; // if filenameSize == 0 then this is an empty FIT block

                for (ubyte1 i = 0; i <= 15; i++) // get filename size (max size = 15)
                {
                    ubyte1 filenameChar = rawBlockDeviceAccessor->getByte(deviceID, pos + 1 + i); // byte i of current fit in section "filename"

                    if (!filenameChar)
                    {
                        //Serial.print("filenameChar = noep!");
                        //Serial.println("pos = "); Serial.println((long)pos);
                        break;
                    }

                    filenameSize = i+1; // as size does never start with 0 if more than 0 chars are available ;)
                    //Serial.print("filenamesize = ");
                    //Serial.println((int)filenameSize);
                }

                if (!filenameSize)
                    continue;

                ubyte1* filename = (ubyte1*)malloc(filenameSize+1); // no memset, I trust this method. +1 for terminating string

                for (ubyte1 i = 0; i < filenameSize; i++) // get filename
                {
                    ubyte1 c = rawBlockDeviceAccessor->getByte(deviceID, pos + 1 + i); // byte i of current fit in section "filename"
                    //Serial.print((char)c); Serial.print("("); Serial.print((int)c); Serial.print(") ");
                    filename[i] = c;
                }

                filename[filenameSize] = 0; // terminate string !!


                // create children now
                VFI* children = new VFI;
                children->deviceID = deviceID;
                children->fileSize = 0; // temp
                children->fileSystemType = 123; //ardunix::filesystem::filesystemtype::adxFS;
                children->fileType = 2; // ardunix::filesystem::filetype::file; // FUCK... I forgot to teach adxFS what files and directories are... temp!!
                children->iNode = fitBlock; // as the iNode is the fitBlock
                //Serial.print("fit block = "); Serial.println((long)fitBlock);
                children->lba = 0; // don't care
                children->nodeName = (char*)filename;
                children->parent = rootVFI;

                rootVFI->addChild(children);
            }
            /*else
                Serial.println("fitParent != 1");*/
        }

        return rootVFI;
    }
}

int MainWindow::finalPosition(VFI *vfi, int offset, bool bullshit)
{
    // TODO: comment this better

    int inode = 0;

    if (vfi)
        inode = vfi->iNode; // get inode

    /*if (bullshit)
        inode += 1;*/

    //qDebug() << "inode: " << inode;

    // ftd_block_pos ist eine POSITION --- aber!!!! ftd_block_pos wird skipBlocks addiert. skipBlocks ist KEINE POSITION, sondern EIN BLOCK !!!!
    // Berechnung KANN NICHT funktionieren! Aber wieso hat es zuvor so gut funktioniert?

    int ftd_block_pos = (inode * 256*256 /* = 65.536 */) + FDT_BLOCK_POS; // get first ftd block position
    //Serial.print("  fsBlockSize = "); Serial.println((long)fsBlockSize);
    int skipBlocks = offset / fsBlockSize; // get the block we really want by respecting the position and the filesystem block size    Byte 56 wanted > 56 / 16 = 3,5 > Block 3
    ftd_block_pos = ftd_block_pos + skipBlocks; // get the real block we want to read by respecting the position
    int rest = offset % fsBlockSize; // get the rest(/offset) in block
    int bytes = (ftd_block_pos - 1) * devBlockSize + rest; // get the real position in bytes

    /*Serial.print("  Block = "); Serial.print((long)ftd_block);
    Serial.print("  Rest = "); Serial.print((long)rest);
    Serial.print("  in Bytes: "); Serial.println((long)bytes);*/

    //qDebug() << "byte: " << bytes;

    return bytes;
}

ubyte1 MainWindow::readByte(int finalPosition) // must convert pos to real position by respecting dev blocksize and fs blocksize
{
    return rawBlockDeviceAccessor->getByte(0, finalPosition);
}

bool MainWindow::writeByte(int finalPosition, ubyte1 value)
{
    rawBlockDeviceAccessor->setByte(0, finalPosition, value);
    return true;
}

void MainWindow::slotOpenVFI(VFI* vfi)
{
    if (!vfi)
        return;

    QString tempFileName = "./temp_" + QString(vfi->nodeName);

    system(QString("rm " + tempFileName).toStdString().c_str()); // delete if already existing

    QFile vfiFile;
    vfiFile.setFileName(tempFileName);
    if (!vfiFile.open(QFile::ReadWrite))
    {
        qDebug() << "NOEP, not possble to create temp file!";
        return;
    }

    QByteArray ba;

    qDebug() << "READING !";
    for (int i = 0; i < (256*256); i++) // 256*256 = current size in bytes of a ftd ( but must be 256*256 but dunno.. )
    {
        int finalPos = finalPosition(vfi, i);
        //qDebug() << "r byte: " << finalPos;
        ubyte1 byte = readByte(finalPos);
        ba.append((char)byte);
    }

    vfiFile.write(ba);

    // at end
    vfiFile.flush();
    vfiFile.close();

    QProcess* proc = new QProcess();
    proc->start(QString("wxHexEditor"), QStringList() << tempFileName);
}

void MainWindow::slotReplaceVFI(VFI* vfi)
{
    if (!vfi)
        return;

    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Replace file with...");

    QFile vfiFile;
    vfiFile.setFileName(filename);
    if (!vfiFile.open(QFile::ReadOnly))
    {
        qDebug() << "NOEP, not possble to create temp file! '" << vfiFile.fileName() << "'";
        return;
    }

    if (vfiFile.size() >= 256*256) // file too big
    {
        qDebug() << "file to big, only 256*256 bytes to be written !";
    }

    QByteArray ba = vfiFile.read(256*256);

    qDebug() << "WRITING !";
    for (int i = 0; i < (256*256) /*ba.size()*/; i++) // 0 - 255 = 256*256 bytes
    {
        int finalPos = finalPosition(vfi, i);
        //qDebug() << "w byte: " << finalPos;
        writeByte(finalPos, ba[i]);
    }

#warning fill rest if there is one with null

    qDebug() << "D0NE!";
}

/*void MainWindow::slotDeleteVFI(VFI* vfi)
{

}*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    rawBlockDeviceAccessor = new RDBAFake();
    vfiList = 0;


    ui->setupUi(this);

    char partitionName[16+1]; // escape

    std::cerr << "Partition Name: \"";

    int i = 0;
    for (; i <= 16; i++)
    {
        partitionName[i] = rawBlockDeviceAccessor->getByte(0, 512+i); // sector 2
        std::cerr << (char)partitionName[i];
    }

    partitionName[i+1] = 0; // escape

    std::cerr << "\"" << std::endl;


    vfiList = this->getRootVFI(40)->getChildrenList(); // sdcard, but does rly not matter

    qDebug() << "getRootVFI:";


    VFI* vfi = vfiList->getFirstEntry();

    l = new QVBoxLayout();
    this->centralWidget()->setLayout(l);

    l->setSpacing(1);

    QString partitionNameConvert = QString(partitionName);

    QString partitionNameString = "Partition name: %1";
    partitionNameString = partitionNameString.arg(partitionNameConvert);

    QLabel* lPartitionName = new QLabel(partitionNameString);
    l->addWidget(lPartitionName);
    l->addWidget(new QLabel("Files: "));

    while (vfi)
    {
        VFIEntryWidget* vfiEW = new VFIEntryWidget(vfi);

        connect(vfiEW, SIGNAL(signalOpen(VFI*)), this, SLOT(slotOpenVFI(VFI*)));
        connect(vfiEW, SIGNAL(signalReplace(VFI*)), this, SLOT(slotReplaceVFI(VFI*)));
        //connect(vfiEW, SIGNAL(signalOpen(VFI*)), this, SLOT(slotOpenVFI(VFI*)));

        //QPushButton* vfiEntry = new QPushButton(QString((const char*)vfi->nodeName));
        //connect(vfiEntry, SIGNAL(clicked()), this, SLOT(slotVFInfo()));

        l->addWidget(vfiEW, 0, Qt::AlignTop);




        std::cerr << "VFI: \n";
        std::cerr << " > name: " << vfi->nodeName << std::endl;
        std::cerr << " > text: \"";

        for (int i = 0; i < (256*256); i++) // 256*256 TEST!
        {
            std::cerr << this->readByte(finalPosition(vfi, i));
            /*std::cerr
                    << (int)this->readByte(finalPosition(vfi, i))
                    << (int)this->readByte(finalPosition(vfi, i+1))
                    << " ";*/
        }

        std::cerr << "\"" << std::endl << std::endl;


        vfi = vfiList->getNextEntry();
    }

    l->addSpacing(1);
    l->addWidget(new QPushButton("Add file .."), 1, Qt::AlignBottom);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete rawBlockDeviceAccessor;
    delete vfiList;
}
