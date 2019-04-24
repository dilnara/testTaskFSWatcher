#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "renamefiledialog.h"

#include <QFileDialog>
#include <QPalette>
#include <QDateTime>
#include <QMessageBox>

const QString c_strListingFiles = "Indexing files...";
const QString c_strTotal = "Total: ";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_rfd(new RenameFileDialog(this)),
    m_fsWatcher(createFSWatcher()),
    m_pathToWatch("")
{
    m_ui->setupUi(this);
    m_fsWatcher->setListener(this);

    QStringList horzHeaders;
    horzHeaders << "name" << "modified" << "size";
    m_ui->tableWidget->setHorizontalHeaderLabels(horzHeaders);
    m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    QPalette palette = m_ui->tableWidget->palette();
    QPixmap px( ":/images/piano_keys.png" );
    palette.setBrush( QPalette::Base, QBrush( px ) );
    m_ui->tableWidget->setPalette( palette );

    //draw count of raws
    updateTotal();

    qRegisterMetaType<FSFileInfo>("FSFileInfo");
    qRegisterMetaType<std::wstring>("std::wstring");
    qRegisterMetaType<std::list<FSFileInfo>>("std::list<FSFileInfo>");

    connect(m_ui->choseFolderButton, SIGNAL (released()), this, SLOT (handleChoseFolderButton()));
    connect(m_ui->toolButton, SIGNAL (released()), this, SLOT (handleRewatchButton()));
    connect(m_ui->tableWidget, SIGNAL(clicked(const QModelIndex &)), this, SLOT(handleTableClicked(const QModelIndex &)));
    connect(m_rfd, SIGNAL(accepted()), this, SLOT(handleRenameFileDialogAccepted()));
    connect(this, SIGNAL(signalOnAdd(const FSFileInfo &)), this, SLOT(handleOnAdd(const FSFileInfo &)));
    connect(this, SIGNAL(signalOnRemove(const std::wstring &)), this, SLOT(handleOnRemove(const std::wstring &)));
    connect(this, SIGNAL(signalOnModified(const FSFileInfo &)), this, SLOT(handleOnModified(const FSFileInfo &)));
    connect(this, SIGNAL(signalOnRenamed(const std::wstring &, const std::wstring &)), this, SLOT(handleOnRenamed(const std::wstring &, const std::wstring &)));
    connect(this, SIGNAL(signalOnError(const std::wstring &)), this, SLOT(handleOnError(const std::wstring &)));
    connect(this, SIGNAL(signalOnFilesListed(std::list<FSFileInfo>)), this, SLOT(handleOnFilesListed(std::list<FSFileInfo>)));
}

MainWindow::~MainWindow() {
    delete m_fsWatcher;
    delete m_ui;
    delete m_rfd;
}

void MainWindow::updateTotal() {
    m_ui->labelTotal->setText(c_strTotal + QString::number(m_ui->tableWidget->rowCount()));
}

//call RenameFileDialogue on name click
void MainWindow::handleTableClicked(const QModelIndex &index) {
    if (index.isValid() && index.column() == 0) {
        QString cellText = index.data().toString();
        m_rfd->setText(cellText);
        m_rfd->exec();
    }
}

void MainWindow::handleRenameFileDialogAccepted() {
    QString newFileName;
    QString oldFileName;
    m_rfd->getData(oldFileName, newFileName);
    if (!m_fsWatcher->renameFile(oldFileName.toStdWString(), newFileName.toStdWString())) {
        handleOnError(L"Couldn't rename, possibly either no access or file has been removed");
    }
}

void MainWindow::setTableRow(int row, const FSFileInfo & fi) {
    QTableWidgetItem *items[3] = {
        new QTableWidgetItem(QString::fromWCharArray(fi.name.c_str())),
        new QTableWidgetItem(QDateTime::fromTime_t(fi.modified).toString()),
        new QTableWidgetItem(QString::fromWCharArray(std::to_wstring(fi.size).c_str()))
    };

    for (int i = 0; i < 3; ++i) {
        items[i]->setFlags(items[i]->flags() ^ Qt::ItemIsEditable);
        m_ui->tableWidget->setItem(row, i, items[i]);
    }
}

void MainWindow::startWatch() {
    m_ui->labelTotal->setText(c_strListingFiles);
    m_ui->tableWidget->setRowCount(0);
    m_fsWatcher->startWatch(m_pathToWatch.toStdWString());
}

void MainWindow::handleRewatchButton() {
    if (m_pathToWatch.isEmpty() || m_pathToWatch.isNull())
        return;
    this->startWatch();
}

void MainWindow::requestDirectory() {
    QString chosenDirectory = QFileDialog::getExistingDirectory(
                                                 this,
                                                 tr("Chose Directory For Watching"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if (!chosenDirectory.isNull()) {
        m_pathToWatch = chosenDirectory;
        m_ui->folderNameLabel->setText(m_pathToWatch);
        this->startWatch();
    }
}

void MainWindow::handleChoseFolderButton() {
    requestDirectory();
}

void MainWindow::handleOnError(const std::wstring & errorMessage) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Warning");
    msgBox.setText(QString::fromWCharArray(errorMessage.c_str()));
    msgBox.exec();
}

void MainWindow::handleOnAdd(const FSFileInfo & newFile) {
    m_ui->tableWidget->insertRow(m_ui->tableWidget->rowCount());
    this->setTableRow(m_ui->tableWidget->rowCount() - 1, newFile);

    updateTotal();
}

void MainWindow::handleOnRemove(const std::wstring & removedFileOrFolderName) {
    for (int r = m_ui->tableWidget->rowCount() - 1; r >= 0; --r) {
        std::wstring fName = m_ui->tableWidget->item(r, 0)->text().toStdWString();
        if (fName.find(removedFileOrFolderName) == 0) {
            m_ui->tableWidget->removeRow(r);
        }
    }

    updateTotal();
}

void MainWindow::handleOnModified(const FSFileInfo & modifiedFile) {
    for (int r = m_ui->tableWidget->rowCount() - 1; r >= 0; --r) {
        std::wstring fName = m_ui->tableWidget->item(r, 0)->text().toStdWString();
        if (fName == modifiedFile.name) {
            this->setTableRow(r, modifiedFile);
        }
    }
}

void MainWindow::handleOnRenamed(const std::wstring & oldName, const std::wstring & newName) {
    for (int r = m_ui->tableWidget->rowCount() - 1; r >= 0; --r) {
        std::wstring fName = m_ui->tableWidget->item(r, 0)->text().toStdWString();
        if (fName.find(oldName) == 0) {
            fName.replace(0, oldName.length(), newName);
            QTableWidgetItem * newItem = new QTableWidgetItem(QString::fromWCharArray(fName.c_str()));
            m_ui->tableWidget->setItem(r, 0, newItem);
        }
    }
}

void MainWindow::handleOnFilesListed(const std::list<FSFileInfo> & filesList) {
    size_t total = filesList.size();
    m_ui->tableWidget->setRowCount(total);
    int row = 0;
    std::list<FSFileInfo>::const_iterator c_it;
    for (c_it = filesList.begin(); c_it != filesList.end(); ++c_it) {
        this->setTableRow(row, *c_it);
        ++row;
    }

    updateTotal();
}

void MainWindow::onError(const std::wstring & errorMessage) {
    emit signalOnError(errorMessage);
}

void MainWindow::onAdd(const FSFileInfo & newFile) {
    emit signalOnAdd(newFile);
}

void MainWindow::onRemove(const std::wstring & removedFile) {
    emit signalOnRemove(removedFile);
}

void MainWindow::onModified(const FSFileInfo & modifiedFile) {
    emit signalOnModified(modifiedFile);
}

void MainWindow::onRenamed(const std::wstring & oldName, const std::wstring & newName) {
    emit signalOnRenamed(oldName, newName);
}

void MainWindow::onFilesListed(const std::list<FSFileInfo> & filesList) {
    emit signalOnFilesListed(filesList);
}
