#ifndef RENAMEFILEDIALOG_H
#define RENAMEFILEDIALOG_H

#include <QDialog>

namespace Ui {
class RenameFileDialog;
}

class RenameFileDialog : public QDialog {
    Q_OBJECT

public:
    explicit RenameFileDialog(QWidget *parent = 0);
    ~RenameFileDialog();
    void setText(QString initialText);
    void getData(QString & from, QString & to);

private:
    Ui::RenameFileDialog *m_ui;
    QString m_initialValue;
};

#endif // RENAMEFILEDIALOG_H
