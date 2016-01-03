#ifndef QGEOCODEREPLYORS_H
#define QGEOCODEREPLYORS_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoCodeReply>

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE

#endif // QGEOCODEREPLYORS_H
