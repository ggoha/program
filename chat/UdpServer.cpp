#include "UdpServer.h"
#include "StartDialog.h"


#define EXITDIALOG -1
#define EXITCONNECTIONDB -2
// ----------------------------------------------------------------------
UdpServer::UdpServer(QWidget* pwgt)
{
    //Первичный диалог
    //Вынести в функцию?
    StartDialog* pInputDialog = new StartDialog;
    if (!pInputDialog->exec()==QDialog::Accepted)
    {
        qDebug()<<"Error Dialog";
        exit (EXITDIALOG);
    }
    start_port=pInputDialog->port();
    my_name=pInputDialog->name();
    delete pInputDialog;

    //Подключение базы данных
    if (!ConnectionDB())
    {
        qDebug()<<"Error connection db";
        exit(EXITCONNECTIONDB);
    }

    //Создание дизайна
    CreateDesign();
}

// ----------------------------------------------------------------------
void UdpServer::FindOnline()
{
    m_pudp->writeDatagram("Who_online?", QHostAddress::Broadcast, start_port);
    m_plw->clear();
}

// ----------------------------------------------------------------------
void UdpServer::SendUserDatagram()
{
    m_pudp->writeDatagram("#"+m_ple->text().toAscii(), QHostAddress::Broadcast, start_port);
}

// ----------------------------------------------------------------------
void UdpServer::SendDatagramPresence(const QHostAddress& host, quint16 port)
{
    m_pudp->writeDatagram(my_name.toAscii(), host, port);
}

// ----------------------------------------------------------------------
void UdpServer::ReadPendingDatagram()

{
    QByteArray baDatagram;
    QHostAddress* datagram_host = new QHostAddress;
    quint16 datagram_port;
    do
    {
        baDatagram.resize(m_pudp->pendingDatagramSize());
        m_pudp->readDatagram(baDatagram.data(), baDatagram.size(), datagram_host, &datagram_port);
    }
    while (m_pudp->hasPendingDatagrams());
    if (baDatagram.at(0)!='#')
        if (baDatagram=="Who_online?")
        {
            SendDatagramPresence(*datagram_host, datagram_port);
        }
        else
        {
            //добавление пользователя онлайн
            //отдельная функция?
            m_plw->addItem(baDatagram+" ("+datagram_host->toString()+')');
        }
    else
    {
        if (!InsertMessageIntoDB(datagram_host, baDatagram))
        {
            qDebug()<<"Message cann't insert";
        }

    }
    delete datagram_host;
}

// ----------------------------------------------------------------------
void UdpServer::ReactionItemCliced(QListWidgetItem* m_plwi)
{
    m_pl->setText(m_plwi->text());
}

bool UdpServer::ConnectionDB()
{
    dbMessage = QSqlDatabase::addDatabase("QSQLITE");
    dbMessage.setDatabaseName("bd_name.sqlite");
    if (!dbMessage.open())
    {
        qDebug()<<"Cann't connection db"<<dbMessage.lastError().text();
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

// ----------------------------------------------------------------------
bool UdpServer::CreateDB()
{
//Проверять на существование
    QSqlQuery a_query;
    QString SqlRequest = "CREATE TABLE messages( \
                                number INTEGER PRIMARY KEY NOT NULL,\
                                sender TEXT,\
                                recipient TEXT,\
                                message TEXT,\
                                datetime TEXT\
                                )";
    bool SqlRequestResult = a_query.exec(SqlRequest);
    if (!SqlRequestResult)
    {
        qDebug()<<"Don't create db"<<dbMessage.lastError().text();
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

// ----------------------------------------------------------------------
bool UdpServer::InsertMessageIntoDB(QHostAddress* datagram_host, QByteArray baDatagram)
{
    int CurrentRow = m_pstm->rowCount();
    if (!m_pstm->insertRow(CurrentRow))
    {
        qDebug()<<"Cann't insert row"<<m_pstm->lastError().text();
        return FALSE;
    }
    m_pstm->setData(m_pstm->index(CurrentRow, 0), CurrentRow);
    m_pstm->setData(m_pstm->index(CurrentRow, 1), datagram_host->toString());
    m_pstm->setData(m_pstm->index(CurrentRow, 2), "ALL");
    m_pstm->setData(m_pstm->index(CurrentRow, 3), baDatagram);
    m_pstm->setData(m_pstm->index(CurrentRow, 4), QDateTime::currentDateTime());

   if (!m_pstm->submitAll())
   {
       qDebug()<<"Error insert data"<<m_pstm->lastError().text();
       return FALSE;
   }
   else
   {
       return TRUE;
   }
}

// ----------------------------------------------------------------------
void UdpServer::CreateDesign()
{
    setWindowTitle("UdpServer");

    m_pudp = new QUdpSocket(this);
    m_pudp->bind(start_port);

    QTimer* ptimer = new QTimer(this);
    ptimer->start(1000);
    connect(ptimer, SIGNAL(timeout()), SLOT(FindOnline()));
    connect(m_pudp, SIGNAL(readyRead()), SLOT(ReadPendingDatagram()));

    m_plw = new QListWidget;
    connect(m_plw, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(ReactionItemCliced(QListWidgetItem*)));

    m_pl = new QLabel(this);
    m_ple = new QLineEdit(this);

    m_pstm = new QSqlTableModel;
    m_pstm->setTable("messages");
    m_pstm->select();
    m_pstm->setEditStrategy(QSqlTableModel::OnManualSubmit);

    view = new QTableView;
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setModel(m_pstm);

    m_ppb = new QPushButton(this);
    m_ppb->setText("Send");
    connect(m_ppb, SIGNAL(clicked()), SLOT(SendUserDatagram()));

    QVBoxLayout* VBoxLayout = new QVBoxLayout;
    QHBoxLayout* HBoxLayout1 = new QHBoxLayout;
    QHBoxLayout* HBoxLayout2 = new QHBoxLayout;
    HBoxLayout1->addWidget(m_ple);
    HBoxLayout1->addWidget(m_ppb);
    VBoxLayout->addWidget(m_pl);
    VBoxLayout->addWidget(view);
    VBoxLayout->addLayout(HBoxLayout1);
    HBoxLayout2->addWidget(m_plw);
    HBoxLayout2->addLayout(VBoxLayout);
    this->setLayout(HBoxLayout2);
}
