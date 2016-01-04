#include "qgeoroutereplyors.h"

#include <QGeoRectangle>
#include <QtLocation/QGeoRouteSegment>
#include <QtLocation/QGeoManeuver>
#include <QtCore/QXmlStreamReader>

QT_BEGIN_NAMESPACE

static int parseTime(const QStringRef& stime) {
    //
    int sec_ = 0;
    //qDebug() << stime.toString();

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
    qDebug()  << "bounding box" << bounds.topLeft() << bounds.bottomRight();
    return bounds;
}

static QList<QGeoCoordinate> parsePolyline(QXmlStreamReader *xml)
{
    QList<QGeoCoordinate> path;
    while (!(xml->isEndElement() && xml->name() == "RouteGeometry")) {
        if (xml->isStartElement() && xml->name() == "pos") {
            xml->readNext();
            path.append(parsePos(xml->text()));
        }
        xml->readNext();
    }

    return path;
}

static QGeoManeuver::InstructionDirection osrmInstructionDirection(const QString &instructionCode)
{
    if (instructionCode == QLatin1String("0"))
        return QGeoManeuver::NoDirection;
    else if (instructionCode == QLatin1String("1"))
        return QGeoManeuver::DirectionForward;
    else if (instructionCode == QLatin1String("2"))
        return QGeoManeuver::DirectionBearRight;
    else if (instructionCode == QLatin1String("3"))
        return QGeoManeuver::DirectionRight;
    else if (instructionCode == QLatin1String("4"))
        return QGeoManeuver::DirectionHardRight;
    else if (instructionCode == QLatin1String("5"))
        return QGeoManeuver::DirectionUTurnLeft;
    else if (instructionCode == QLatin1String("6"))
        return QGeoManeuver::DirectionHardLeft;
    else if (instructionCode == QLatin1String("7"))
        return QGeoManeuver::DirectionLeft;
    else if (instructionCode == QLatin1String("8"))
        return QGeoManeuver::DirectionBearLeft;
    else if (instructionCode == QLatin1String("9"))
        return QGeoManeuver::NoDirection;
    else if (instructionCode == QLatin1String("10"))
        return QGeoManeuver::DirectionForward;
    else if (instructionCode == QLatin1String("11"))
        return QGeoManeuver::NoDirection;
    else if (instructionCode == QLatin1String("12"))
        return QGeoManeuver::NoDirection;
    else if (instructionCode == QLatin1String("13"))
        return QGeoManeuver::NoDirection;
    else if (instructionCode == QLatin1String("14"))
        return QGeoManeuver::NoDirection;
    else if (instructionCode == QLatin1String("15"))
        return QGeoManeuver::NoDirection;
    else
        return QGeoManeuver::NoDirection;
}

const QString osrmInstructionText(const QString &instructionCode, const QString &wayname)
{
    if (instructionCode == QLatin1String("0")) {
        return QString();
    } else if (instructionCode == QLatin1String("1")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Go straight.");
        else
            return QGeoRouteReplyOrs::tr("Go straight onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("2")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Turn slightly right.");
        else
            return QGeoRouteReplyOrs::tr("Turn slightly right onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("3")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Turn right.");
        else
            return QGeoRouteReplyOrs::tr("Turn right onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("4")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Make a sharp right.");
        else
            return QGeoRouteReplyOrs::tr("Make a sharp right onto %1.").arg(wayname);
    }
    else if (instructionCode == QLatin1String("5")) {
        return QGeoRouteReplyOrs::tr("When it is safe to do so, perform a U-turn.");
    } else if (instructionCode == QLatin1String("6")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Make a sharp left.");
        else
            return QGeoRouteReplyOrs::tr("Make a sharp left onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("7")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Turn left.");
        else
            return QGeoRouteReplyOrs::tr("Turn left onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("8")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Turn slightly left.");
        else
            return QGeoRouteReplyOrs::tr("Turn slightly left onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("9")) {
        return QGeoRouteReplyOrs::tr("Reached waypoint.");
    } else if (instructionCode == QLatin1String("10")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Head on.");
        else
            return QGeoRouteReplyOrs::tr("Head onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11")) {
        return QGeoRouteReplyOrs::tr("Enter the roundabout.");
    } else if (instructionCode == QLatin1String("11-1")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the first exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the first exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11-2")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the second exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the second exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11-3")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the third exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the third exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11-4")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the fourth exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the fourth exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11-5")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the fifth exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the fifth exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11-6")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the sixth exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the sixth exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11-7")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the seventh exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the seventh exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11-8")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the eighth exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the eighth exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("11-9")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("At the roundabout take the ninth exit.");
        else
            return QGeoRouteReplyOrs::tr("At the roundabout take the ninth exit onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("12")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Leave the roundabout.");
        else
            return QGeoRouteReplyOrs::tr("Leave the roundabout onto %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("13")) {
        return QGeoRouteReplyOrs::tr("Stay on the roundabout.");
    } else if (instructionCode == QLatin1String("14")) {
        if (wayname.isEmpty())
            return QGeoRouteReplyOrs::tr("Start at the end of the street.");
        else
            return QGeoRouteReplyOrs::tr("Start at the end of %1.").arg(wayname);
    } else if (instructionCode == QLatin1String("15")) {
        return QGeoRouteReplyOrs::tr("You have reached your destination.");
    } else {
        return QGeoRouteReplyOrs::tr("Don't know what to say for '%1'").arg(instructionCode);
    }
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
//    QGeoRouteSegment firstSegment;
//    int firstPosition = -1;
//    int segmentPathLengthCount = 0;
    while (!xml->atEnd()) {
        xml->readNext();
        if (xml->isStartElement() && xml->name() == "TotalTime") {
            xml->readNext();
            int totalTime = parseTime(xml->text());
            route.setTravelTime(totalTime);
            qDebug() << "total time" << totalTime;
        }
        if (xml->isStartElement() && xml->name() == "TotalDistance") {
            qreal totalDist = xml->attributes().value("value").toFloat();
            route.setDistance(totalDist);
            qDebug() << "total distance" << totalDist;
        }
        if (xml->isStartElement() && xml->name() == "BoundingBox") {
            QGeoRectangle bounds = parseBounds(xml);
            route.setBounds(bounds);
        }
        if (xml->isStartElement() && xml->name() == "RouteGeometry") {
            path = parsePolyline(xml);
        }
    }

/*
    for (int i = instructions.count() - 1; i >= 0; --i) {
        QJsonArray instruction = instructions.at(i).toArray();

        if (instruction.count() < 8) {
            qWarning("Instruction does not contain enough fields.");
            continue;
        }

        const QString instructionCode = instruction.at(0).toString();
        const QString wayname = instruction.at(1).toString();
        double segmentLength = instruction.at(2).toDouble();
        int position = instruction.at(3).toDouble();
        int time = instruction.at(4).toDouble();
        //const QString segmentLengthString = instruction.at(5).toString();
        //const QString direction = instruction.at(6).toString();
        //double azimuth = instruction.at(7).toDouble();

        QGeoRouteSegment segment;
        segment.setDistance(segmentLength);

        QGeoManeuver maneuver;
        maneuver.setDirection(osrmInstructionDirection(instructionCode));
        maneuver.setDistanceToNextInstruction(segmentLength);
        maneuver.setInstructionText(osrmInstructionText(instructionCode, wayname));
        maneuver.setPosition(path.at(position));
        maneuver.setTimeToNextInstruction(time);

        segment.setManeuver(maneuver);

        if (firstPosition == -1)
            segment.setPath(path.mid(position));
        else
            segment.setPath(path.mid(position, firstPosition - position));

        segmentPathLengthCount += segment.path().length();

        segment.setTravelTime(time);

        segment.setNextRouteSegment(firstSegment);

        firstSegment = segment;
        firstPosition = position;
    }

    route.setFirstRouteSegment(firstSegment);

*/
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
    QByteArray data = m_reply->readAll();
    //qDebug() << "data:" << data;
    xml.addData(data);

    int routesNum = 0;
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == "Response") {
            routesNum = xml.attributes().value("numberOfResponses").toInt();
            qDebug() << "routes" << routesNum;
            if (routesNum > 0) {
                QGeoRoute route = constructRoute(&xml);
                routes.append(route);
            }
        }
    }
    if (xml.hasError()) {
        setError(QGeoRouteReply::ParseError, QStringLiteral("Couldn't parse xml."));
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

QT_END_NAMESPACE
