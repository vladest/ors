#ifndef QPLACEMANAGERENGINEORS_H
#define QPLACEMANAGERENGINEORS_H

#include <QtLocation/QPlaceManagerEngine>
#include <QtLocation/QGeoServiceProvider>

class QNetworkAccessManager;
class QNetworkReply;
class QPlaceCategoriesReplyOrs;

class QPlaceManagerEngineOrs : public QPlaceManagerEngine
{
    Q_OBJECT

public:
    QPlaceManagerEngineOrs(const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                           QString *errorString);
    ~QPlaceManagerEngineOrs();

    QPlaceSearchReply *search(const QPlaceSearchRequest &request) Q_DECL_OVERRIDE;

    QPlaceReply *initializeCategories() Q_DECL_OVERRIDE;
    QString parentCategoryId(const QString &categoryId) const Q_DECL_OVERRIDE;
    QStringList childCategoryIds(const QString &categoryId) const Q_DECL_OVERRIDE;
    QPlaceCategory category(const QString &categoryId) const Q_DECL_OVERRIDE;

    QList<QPlaceCategory> childCategories(const QString &parentId) const Q_DECL_OVERRIDE;

    QList<QLocale> locales() const Q_DECL_OVERRIDE;
    void setLocales(const QList<QLocale> &locales) Q_DECL_OVERRIDE;

private slots:
    void categoryReplyFinished();
    void categoryReplyError();
    void replyFinished();
    void replyError(QPlaceReply::Error errorCode, const QString &errorString);

private:
    void fetchNextCategoryLocale();

    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_urlPrefix;
    QList<QLocale> m_locales;

    QNetworkReply *m_categoriesReply;
    QList<QPlaceCategoriesReplyOrs *> m_pendingCategoriesReply;
    QHash<QString, QPlaceCategory> m_categories;
    QHash<QString, QStringList> m_subcategories;

    QList<QLocale> m_categoryLocales;
};

#endif // QPLACEMANAGERENGINEOrs_H
