#include "StartDialog.h"

StartDialog::StartDialog(QWidget* pwgt) : QDialog(pwgt, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    m_pname = new QLineEdit;
    m_pport = new QSpinBox;
    m_pport->setRange(1000, 65535);

    QLabel* p_mlname = new QLabel("Name");
    QLabel* p_mlport = new QLabel("Port");

    QPushButton* p_mpbok = new QPushButton("Ok");
    QPushButton* p_mpbcancel = new QPushButton("Cancel");

    connect(p_mpbok, SIGNAL(clicked()), SLOT(accept()));
    connect(p_mpbcancel, SIGNAL(clicked()), SLOT(reject()));

    QGridLayout* p_mgl = new QGridLayout;
    p_mgl->addWidget(m_pname, 0, 1);
    p_mgl->addWidget(p_mlname, 0, 0);
    p_mgl->addWidget(m_pport, 1, 1);
    p_mgl->addWidget(p_mlport, 1, 0);
    p_mgl->addWidget(p_mpbok, 2, 0);
    p_mgl->addWidget(p_mpbcancel, 2, 1);

    setLayout(p_mgl);

}
QString StartDialog::name() const
{
    return m_pname->text();
}

quint16 StartDialog::port() const
{
    return m_pport->value();
}

//StartDialog::~StartDialog()
//{
//    delete m_pname;
//    delete m_pport
//}
