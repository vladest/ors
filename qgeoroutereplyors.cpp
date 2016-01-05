#include "qgeoroutereplyors.h"

#include <QGeoRectangle>
#include <QtLocation/QGeoRouteSegment>
#include <QtLocation/QGeoManeuver>
#include <QtCore/QXmlStreamReader>

static int parseTime(const QStringRef& stime) {
    int sec_ = 0;

    if (stime.startsWith("PT")) {
        int start_  = 2;
        int indH = stime.indexOf("H");
        int indM = stime.indexOf("M");
        int indS = stime.indexOf("S");
        if (indH >= 0) {
            sec_ += stime.mid(start_, indH - start_).toInt()*3600;
            start_ = indH + 1;
        }
        if (indM >= 0) {
            sec_ += stime.mid(start_, indM - start_).toInt()*60;
            start_ = indM + 1;
        }
        if (indS >= 0) {
            sec_ += stime.mid(start_, indS - start_).toInt();
        }
    }
    return sec_;
}

static QGeoCoordinate parsePos(const QStringRef& spos) {
    QGeoCoordinate coord(0, 0);
    QVector<QStringRef> vec_coord = spos.split(" ");
    if (vec_coord.size() == 2) {
        coord.setLongitude(vec_coord.at(0).toDouble());
        coord.setLatitude(vec_coord.at(1).toDouble());
    }
    return coord;
}

static QGeoRectangle parseBounds(QXmlStreamReader *xml) {
    QList<QGeoCoordinate> coordinates;
    while (!(xml->isEndElement() && xml->name() == "BoundingBox")) {
        if (xml->isStartElement() && xml->name() == "pos") {
            xml->readNext();
            coordinates.append(parsePos(xml->text()));
        }
        xml->readNext();
    }

    QGeoRectangle bounds = QGeoRectangle(coordinates);
    return bounds;
}

static QList<QGeoCoordinate> parsePolyline(QXmlStreamReader *xml, const QString &element)
{
    QList<QGeoCoordinate> path;
    while (!(xml->isEndElement() && xml->name() == element)) {
        if (xml->isStartElement() && xml->name() == "pos") {
            xml->readNext();
            path.append(parsePos(xml->text()));
        }
        xml->readNext();
    }

    return path;
}

static QGeoManeuver::InstructionDirection osrmInstructionDirection(int instructionCode)
{
    if (instructionCode == 0)
        return QGeoManeuver::DirectionForward;
    else if (instructionCode == 1)
        return QGeoManeuver::DirectionBearRight;
    else if (instructionCode == 2)
        return QGeoManeuver::DirectionRight;
    else if (instructionCode == 3)
        return QGeoManeuver::DirectionHardRight;
    else if (instructionCode == -3)
        return QGeoManeuver::DirectionHardLeft;
    else if (instructionCode == -2)
        return QGeoManeuver::DirectionLeft;
    else if (instructionCode == -1)
        return QGeoManeuver::DirectionBearLeft;
    else
        return QGeoManeuver::NoDirection;
}

static QGeoRouteSegment parseInstructions(QXmlStreamReader *xml)
{
    QGeoRouteSegment firstSegment;
    QGeoRouteSegment prevSegment;
    while (!(xml->isEndElement() && xml->name() == "RouteInstructionsList")) {
        if (xml->isStartElement() && xml->name() == "RouteInstruction") {
            qreal segmentTime = parseTime(xml->attributes().value("duration"));
            QString description = xml->attributes().value("description").toString();

            QGeoRouteSegment segment;
            QGeoManeuver maneuver;
            int directionCode = -1;
            QString instructionText;
            qreal distance = 0;
            QList<QGeoCoordinate> path;
            while (!(xml->isEndElement() && xml->name() == "RouteInstruction")) {
                if (xml->isStartElement() && xml->name() == "DirectionCode") {
                    xml->readNext();
                    directionCode = xml->text().toInt();
                }
                if (xml->isStartElement() && xml->name() == "Instruction") {
                    xml->readNext();
                    instructionText = xml->text().toString();
                }
                if (xml->isStartElement() && xml->name() == "Distance") {
                    distance = xml->attributes().value("value").toFloat();
                }
                if (xml->isStartElement() && xml->name() == "RouteInstructionGeometry") {
                    path = parsePolyline(xml, "RouteInstructionGeometry");
                }
                xml->readNext();
            }
            segment.setDistance(distance);

            maneuver.setDirection(osrmInstructionDirection(directionCode));
            maneuver.setDistanceToNextInstruction(distance);
            maneuver.setInstructionText(instructionText);
            if (path.size() > 0)
                maneuver.setPosition(path.at(0));
            maneuver.setTimeToNextInstruction(segmentTime);

            segment.setManeuver(maneuver);
            segment.setPath(path);
            segment.setTravelTime(segmentTime);

            if (!firstSegment.isValid()) {
                firstSegment = segment;
            }
            if (prevSegment.isValid())
                prevSegment.setNextRouteSegment(segment);
            prevSegment = segment;
        }
        xml->readNext();
    }
    return firstSegment;
}

QGeoRouteReplyOrs::QGeoRouteReplyOrs(QNetworkReply *reply, const QGeoRouteRequest &request,
                                     QObject *parent)
    :   QGeoRouteReply(request, parent), m_reply(reply)
{
    connect(m_reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
}

QGeoRouteReplyOrs::~QGeoRouteReplyOrs()
{
    if (m_reply)
        m_reply->deleteLater();
}

void QGeoRouteReplyOrs::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = 0;
}

static QGeoRoute constructRoute(QXmlStreamReader *xml)
{
    QGeoRoute route;

    QList<QGeoCoordinate> path;
    QGeoRouteSegment firstSegment;
    QString instrictionsLang;

    while (!xml->atEnd()) {
        xml->readNext();
        if (xml->isStartElement() && xml->name() == "TotalTime") {
            xml->readNext();
            int totalTime = parseTime(xml->text());
            route.setTravelTime(totalTime);
        }
        if (xml->isStartElement() && xml->name() == "TotalDistance") {
            qreal totalDist = xml->attributes().value("value").toFloat();
            route.setDistance(totalDist);
        }
        if (xml->isStartElement() && xml->name() == "BoundingBox") {
            QGeoRectangle bounds = parseBounds(xml);
            route.setBounds(bounds);
        }
        if (xml->isStartElement() && xml->name() == "RouteGeometry") {
            path = parsePolyline(xml, "RouteGeometry");
        }
        if (xml->isStartElement() && xml->name() == "RouteInstructionsList") {
            instrictionsLang = xml->attributes().value("lang").toString();
            firstSegment = parseInstructions(xml);
        }
    }

    route.setFirstRouteSegment(firstSegment);
    route.setPath(path);
    return route;
}

void QGeoRouteReplyOrs::networkReplyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError) {
        setError(QGeoRouteReply::CommunicationError, m_reply->errorString());
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }
    QList<QGeoRoute> routes;
    QXmlStreamReader xml;
    xml.addData(m_reply->readAll());

    int routesNum = 0;
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == "Response") {
            routesNum = xml.attributes().value("numberOfResponses").toInt();
            if (routesNum > 0) {
                QGeoRoute route = constructRoute(&xml);
                routes.append(route);
            }
        }
    }
    if (xml.hasError()) {
        setError(QGeoRouteReply::ParseError, QStringLiteral("Error parsing OpenRouteService xml response:") + xml.errorString() + " at line: " + xml.lineNumber());
    } else {
        setRoutes(routes);
        setFinished(true);
    }

    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoRouteReplyOrs::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoRouteReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = 0;
}
