#include "renamefiledialog.h"
#include "ui_renamefiledialog.h"

RenameFileDialog::RenameFileDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::RenameFileDialog) {
    m_ui->setupUi(this);    
}

void RenameFileDialog::setText(QString initialText) {
    m_initialValue = initialText;
    m_ui->lineEdit->setText(initialText);
}

void RenameFileDialog::getData(QString & from, QString & to) {
    from = m_initialValue;
    to = m_ui->lineEdit->text();
}

RenameFileDialog::~RenameFileDialog() {
    delete m_ui;
}
