#ifndef QGEOCODINGMANAGERENGINEORS_H
#define QGEOCODINGMANAGERENGINEORS_H

#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QGeoCodingManagerEngine>
#include <QtLocation/QGeoCodeReply>

class QNetworkAccessManager;

class QGeoCodingManagerEngineOrs : public QGeoCodingManagerEngine
{
    Q_OBJECT

public:
    QGeoCodingManagerEngineOrs(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                               QString *errorString);
    ~QGeoCodingManagerEngineOrs();

    QGeoCodeReply *geocode(const QGeoAddress &address, const QGeoShape &bounds) Q_DECL_OVERRIDE;
    QGeoCodeReply *geocode(const QString &address, int limit, int offset,
                           const QGeoShape &bounds) Q_DECL_OVERRIDE;
    QGeoCodeReply *reverseGeocode(const QGeoCoordinate &coordinate,
                                  const QGeoShape &bounds) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void replyFinished();
    void replyError(QGeoCodeReply::Error errorCode, const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_urlPrefix;
};

#endif // QGEOCODINGMANAGERENGINEOrs_H
