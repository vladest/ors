#include "qplacecategoriesreplyors.h"

QPlaceCategoriesReplyOrs::QPlaceCategoriesReplyOrs(QObject *parent)
:   QPlaceReply(parent)
{
}

QPlaceCategoriesReplyOrs::~QPlaceCategoriesReplyOrs()
{
}

void QPlaceCategoriesReplyOrs::emitFinished()
{
    setFinished(true);
    emit finished();
}

void QPlaceCategoriesReplyOrs::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
    emit error(errorCode, errorString);
}
