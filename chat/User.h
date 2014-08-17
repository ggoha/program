#ifndef USER_H
#define USER_H

#include <QString>
#include <QtNetwork>
#include <QStringList>
class User
{
private:
    QString name;
    QHostAddress* address;
    quint16 port;
    //    QStringList messages;
    //    QTcpSocket();
    bool online;
public:
    User();
};

#endif // USER_H
