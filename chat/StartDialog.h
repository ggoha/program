#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QtGui>
#include <QDialog>

class StartDialog : public QDialog
{
Q_OBJECT
private:
    QLineEdit* m_pname;
    QSpinBox* m_pport;
public:
    StartDialog(QWidget* = 0);
    QString name() const;
    quint16 port() const;
//?? прочитать
//    ~StartDialog();
};

#endif // STARTDIALOG_H
