#ifndef QGEOCODEREPLYORS_H
#define QGEOCODEREPLYORS_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoCodeReply>

class QGeoCodeReplyOrs : public QGeoCodeReply
{
    Q_OBJECT

public:
    explicit QGeoCodeReplyOrs(QNetworkReply *reply, QObject *parent = 0);
    ~QGeoCodeReplyOrs();

    void abort();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkReply *m_reply;
};

#endif // QGEOCODEREPLYORS_H
