#ifndef QPLACECATEGORIESREPLYORS_H
#define QPLACECATEGORIESREPLYORS_H

#include <QtLocation/QPlaceReply>

class QPlaceCategoriesReplyOrs : public QPlaceReply
{
    Q_OBJECT

public:
    explicit QPlaceCategoriesReplyOrs(QObject *parent = 0);
    ~QPlaceCategoriesReplyOrs();

    void emitFinished();
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
};

#endif // QPLACECATEGORIESREPLYORS_H
