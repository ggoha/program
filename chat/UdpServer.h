#ifndef UDPSERVER_H
#define UDPSERVER_H
#include <QtGui>
#include <QtNetwork>
#include <QtSql>

class UdpServer : public QWidget
{
Q_OBJECT
private:
    QUdpSocket* m_pudp;
    QLineEdit* m_ple;
    QTableView* view;
    QSqlTableModel* m_pstm;
    QPushButton* m_ppb;
    QLabel* m_pl;

    QListWidget* m_plw;

    qint16 start_port;
    QString my_name;

    QSqlDatabase dbMessage;

    void SendDatagramPresence(const QHostAddress& host, quint16 port);

    bool ConnectionDB();
    bool CreateDB();
    bool InsertMessageIntoDB(QHostAddress* datagram_host, QByteArray baDatagram);

    void CreateDesign();
public:
    UdpServer(QWidget* pwgt = 0);

private slots:
    void FindOnline();
    void ReadPendingDatagram();
    void SendUserDatagram();
    void ReactionItemCliced(QListWidgetItem*);
};


#endif // UDPSERVER_H
