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
#include <QInputDialog>


#define FIT_BLOCK 3
#define FIT_BLOCK_SIZE 1
#define FDT_BLOCK 260
#define FDT_BLOCK_SIZE 256


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
                children->fileType = 2; // ardunix::filesystem::filetype::file; // I forgot to teach adxFS what files and directories are... temp!!
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

bool MainWindow::findFreeINode(int &freeINode)
{
    for (int i = 0; i < 256; i++)
    {
        ubyte1 parent = this->readByte(this->finalPositionFIT(i, 0));
        ubyte1 firstFilenameChar = this->readByte(this->finalPositionFIT(i, 1));

        if (!parent && !firstFilenameChar) // if both are 0, then inode is free
        {
            freeINode = i;
            return true;
        }
    }

    return false;
}

int MainWindow::finalPositionFIT(int inode, int offset)
{
    int fit_block = (inode * FIT_BLOCK_SIZE) + FIT_BLOCK;
    return finalPosition(fit_block, offset);
}

int MainWindow::finalPositionFDT(int inode, int offset)
{
    int fdt_block = (inode * FDT_BLOCK_SIZE /* 1 FTD has 256 Sectors (blocks) */) + FDT_BLOCK; // get first ftd block
    return finalPosition(fdt_block, offset);
}

int MainWindow::finalPosition(int block, int offset)
{
    // TODO: comment this better

    int _block = block;
    //Serial.print("  fsBlockSize = "); Serial.println((long)fsBlockSize);
    int skipFSBlocks = offset / fsBlockSize; // get the block we really want by respecting the position and the filesystem block size    Byte 56 wanted > 56 / 16 = 3,5 > Block 3
    _block = _block + skipFSBlocks; // get the real block we want to read by respecting the position
    int rest = offset % fsBlockSize; // get the rest(/offset) in block
    int bytes = (_block - 1) * devBlockSize + rest; // get the real position in bytes

    /*Serial.print("  Block = "); Serial.print((long)ftd_block);
    Serial.print("  Rest = "); Serial.print((long)rest);
    Serial.print("  in Bytes: "); Serial.println((long)bytes);*/

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
        int inode = 0;

        if (vfi)
            inode = vfi->iNode;

        int finalPos = finalPositionFDT(inode, i);
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

    if (vfiFile.size() >= 256*16) // file too big
    {
        qDebug() << "file too big, only 256*16 bytes (4.096 Bytes) to be written !";
    }

    QByteArray ba = vfiFile.read(256*16);

    qDebug() << "WRITING !";
    for (int i = 0; /*i < (256*16) && because read has maxlen given*/ i < ba.size(); i++) // 0 - 255 = 256*256 bytes
    {
        int inode = 0;

        if (vfi)
            inode = vfi->iNode;

        int finalPos = finalPositionFDT(inode, i);
        //qDebug() << "w byte: " << finalPos;
        writeByte(finalPos, ba[i]);
    }

    qDebug() << "D0NE!";
}

void MainWindow::slotDeleteVFI(VFI* vfi)
{
    if (!vfi)
    {
        qDebug() << "Invalid VFI, cannot delete invalid file!";
        return;
    }

    // clear fit entry of file
    for (int i = 0; i < 16; i++)
    {
        this->writeByte(this->finalPositionFIT(vfi->iNode, i), 0);
    }

    // clear fdt entry of file (for safety and clearity)
    for (int i = 0; i < (FDT_BLOCK_SIZE * fsBlockSize); i++)
    {
        this->writeByte(this->finalPositionFDT(vfi->iNode, i), 0);
    }

    qDebug() << "File deleted!";
}

void MainWindow::slotFormatDisk()
{
    QString partitionNameString = QInputDialog::getText(this, "Format disk with adxFS", "Enter new Partition name:");

    if (partitionNameString.size() > 16) // can only carry 16 bytes
    {
        qDebug() << "Partition name > 16 bytes !";
        slotFormatDisk();
        return;
    }
    else
        if (partitionNameString.size() < 1)
        {
            qDebug() << "aborted.";
            return;
        }


    const char* partitionName = partitionNameString.toStdString().c_str();

    // write adxFS magic
    rawBlockDeviceAccessor->setByte(0, 0, 'a');
    rawBlockDeviceAccessor->setByte(0, 1, 'd');
    rawBlockDeviceAccessor->setByte(0, 2, 'x');
    rawBlockDeviceAccessor->setByte(0, 3, 'F');
    rawBlockDeviceAccessor->setByte(0, 4, 'S');
    rawBlockDeviceAccessor->setByte(0, 5, 1); // version

    // write partition name
    int i = 0;
    for (; i < partitionNameString.count(); i++)
    {
        rawBlockDeviceAccessor->setByte(0, devBlockSize * 1 + i, partitionName[i]); // partition name on sector 1
    }

    // write rest with 0 to cover old, longer names
    for (; i < 16; i++)
    {
        rawBlockDeviceAccessor->setByte(0, devBlockSize * 1 + i, 0); // partition name on sector 1
    }

    // clear all fits with 0
    for (int i = 0; i < 256; i++) // fit (n / 255)
    {
        for (int k = 0; k < 16; k++) // byte in fit (n / 16)
        {
            this->writeByte(this->finalPositionFIT(i, k), 0); // fit: clear with 0
        }
    }

    // create root directory
    this->writeByte(this->finalPositionFIT(0, 0), 0); // create fit 0 (root dir)
    this->writeByte(this->finalPositionFIT(0, 1), 'r');
    this->writeByte(this->finalPositionFIT(0, 2), 'o');
    this->writeByte(this->finalPositionFIT(0, 3), 'o');
    this->writeByte(this->finalPositionFIT(0, 4), 't');
    this->writeByte(this->finalPositionFIT(0, 5), 0); // for safety reason a zero

    qDebug() << "Done formatting!";
}

MainWindow::MainWindow(QString device, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    rawBlockDeviceAccessor = new RDBAFake(device);
    vfiList = 0;

    ui->setupUi(this);

    l = new QVBoxLayout();
    this->centralWidget()->setLayout(l);
    l->setSpacing(1);

    QPushButton* pbFormat = new QPushButton("Format Disk with adxFS");
    connect(pbFormat, SIGNAL(clicked()), this, SLOT(slotFormatDisk()));
    l->addWidget(pbFormat);

    // check if this is a adxFS filesystem disk
    char magic1 = rawBlockDeviceAccessor->getByte(0, 0);
    char magic2 = rawBlockDeviceAccessor->getByte(0, 1);
    char magic3 = rawBlockDeviceAccessor->getByte(0, 2);
    char magic4 = rawBlockDeviceAccessor->getByte(0, 3);
    char magic5 = rawBlockDeviceAccessor->getByte(0, 4);

    if (   magic1 == 'a' // 0x61 &&
        && magic2 == 'd' // 0x64 &&
        && magic3 == 'x' // 0x78 &&
        && magic4 == 'F' // 0x46 &&
        && magic5 == 'S' // 0x53 &&
        //&& header_version <= ADXFS_MINVER)
       )
    {
        qDebug() << "Disk has adxFS on it";
    }
    else
    {
        qDebug() << "Disk not formatted with adxFS";
        return;
    }


    char partitionName[16+1]; // escape

    int i = 0;
    for (; i <= 16; i++)
    {
        partitionName[i] = rawBlockDeviceAccessor->getByte(0, 512+i); // sector 2
    }

    partitionName[i+1] = 0; // escape

    QString partitionNameConvert = QString(partitionName);

    QString partitionNameString = "Partition name: %1";
    partitionNameString = partitionNameString.arg(partitionNameConvert);

    QLabel* lPartitionName = new QLabel(partitionNameString);
    l->addWidget(lPartitionName);


    vfiList = this->getRootVFI(40)->getChildrenList(); // 40 = sdcard, but does rly not matter
    VFI* vfi = 0;

    if (!vfiList)
    {
        qDebug() << "No files on this disk.";
    }
    else // vfiList exists (there are files)
    {
        vfi = vfiList->getFirstEntry();
        l->addWidget(new QLabel("Files: "));
    }

    while (vfi)
    {
        VFIEntryWidget* vfiEW = new VFIEntryWidget(vfi);

        connect(vfiEW, SIGNAL(signalOpen(VFI*)), this, SLOT(slotOpenVFI(VFI*)));
        connect(vfiEW, SIGNAL(signalReplace(VFI*)), this, SLOT(slotReplaceVFI(VFI*)));
        connect(vfiEW, SIGNAL(signalDelete(VFI*)), this, SLOT(slotDeleteVFI(VFI*)));

        //QPushButton* vfiEntry = new QPushButton(QString((const char*)vfi->nodeName));
        //connect(vfiEntry, SIGNAL(clicked()), this, SLOT(slotVFInfo()));

        l->addWidget(vfiEW, 0, Qt::AlignTop);

        vfi = vfiList->getNextEntry();
    }    

    l->addSpacing(1);
    QPushButton* pbAddFile = new QPushButton("Add file ..");
    connect(pbAddFile, SIGNAL(clicked()), this, SLOT(slotAddFile()));
    l->addWidget(pbAddFile, 1, Qt::AlignBottom);
}

void MainWindow::slotAddFile()
{
    int freeINode = 0;

    if (findFreeINode(freeINode))
    {
        QString filenameString = QInputDialog::getText(this, "Add file", "Enter filename:");

        if (filenameString.size() > 15) // can only carry 15 bytes
        {
            qDebug() << "File name > 15 bytes !";
            slotAddFile();
            return;
        }
        else
            if (filenameString.size() < 1)
            {
                qDebug() << "aborted.";
                return;
            }

        const char* filename = filenameString.toStdString().c_str();

        // set root directory (as there are no directories in adxFS v1, it can be manually written (root 0)
        this->writeByte(this->finalPositionFIT(freeINode, 0), 0);

        // write filename
        int i = 1; // filename starts @ offset 1
        for (; i < filenameString.count()+1; i++) // +1 because we start @ i = 1
        {
            this->writeByte(this->finalPositionFIT(freeINode, i), filename[i-1]);
        }

        // write rest with 0 to cover old, longer names
        for (; i < 15; i++)
        {
            this->writeByte(this->finalPositionFIT(freeINode, i), 0);
        }

        // clear fdt entry for file
        for (int k = 0; k < (FDT_BLOCK_SIZE * fsBlockSize); k++) // 256 * 16 bytes (4KB)
        {
             this->writeByte(this->finalPositionFDT(freeINode, k), 0);
        }

        qDebug() << "File created!";
    }
    else
    {
        qDebug() << "Device full!";
        return;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete rawBlockDeviceAccessor;
    delete vfiList;
}
