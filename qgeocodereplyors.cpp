#include "qgeocodereplyors.h"

#include <QXmlStreamReader>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoLocation>
#include <QtPositioning/QGeoRectangle>

QGeoCodeReplyOrs::QGeoCodeReplyOrs(QNetworkReply *reply, QObject *parent)
:   QGeoCodeReply(parent), m_reply(reply)
{
    connect(m_reply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));

    setLimit(1);
    setOffset(0);
}

QGeoCodeReplyOrs::~QGeoCodeReplyOrs()
{
    if (m_reply)
        m_reply->deleteLater();
}

void QGeoCodeReplyOrs::abort()
{
    if (!m_reply)
        return;

    m_reply->abort();

    m_reply->deleteLater();
    m_reply = 0;
}

static QGeoCoordinate parsePoint(QXmlStreamReader *xml) {
    QGeoCoordinate coord(0, 0);
    do {
        if (xml->isStartElement() && xml->name() == "pos") {
            xml->readNext();
            QVector<QStringRef> vec_coord = xml->text().split(" ");
            if (vec_coord.size() == 2) {
                coord.setLongitude(vec_coord.at(0).toDouble());
                coord.setLatitude(vec_coord.at(1).toDouble());
            }
        }
        xml->readNext();
    } while (!(xml->isEndElement() && xml->name() == "pos"));
    return coord;
}

static QGeoAddress parseAddress(QXmlStreamReader *xml) {
    QGeoAddress address;
    QString countryCode = xml->attributes().value("countryCode").toString();
    QString street;
    QString building;
    address.setCountryCode(countryCode);
    do {
        if (xml->isStartElement() && xml->name() == "PostalCode") {
            xml->readNext();
            address.setPostalCode(xml->text().toString());
        }
        if (xml->isStartElement() && xml->name() == "Street") {
            street = xml->attributes().value("officialName").toString();
        }
        if (xml->isStartElement() && xml->name() == "Building") {
            building = xml->attributes().value("number").toString();
        }
        if (xml->isStartElement() && xml->name() == "Place") {
            QString attrType = xml->attributes().value("type").toString();
            xml->readNext();
            if (attrType == "Country") {
                address.setCountry(xml->text().toString());
            }
            if (attrType == "Municipality") {
                address.setCity(xml->text().toString());
            }
            if (attrType == "MunicipalitySubdivision") {
                address.setDistrict(xml->text().toString());
            }
            if (attrType == "CountrySubdivision") {
                address.setState(xml->text().toString());
            }
            if (attrType == "CountrySecondarySubdivision") {
                address.setCounty(xml->text().toString());
            }
        }
        xml->readNext();
    } while (!(xml->isEndElement() && xml->name() == "Address"));
    if (!building.isEmpty())
        street += ", " + building;
    address.setStreet(street);
    return address;
}

static QList<QGeoLocation> parseLocations(QXmlStreamReader *xml)
{
    QList<QGeoLocation> locations;

    while (!(xml->isEndElement() && xml->name() == "Response")) {
        if (xml->isStartElement() &&
                (xml->name() == "GeocodedAddress" ||
                 xml->name() == "ReverseGeocodedLocation")) {
            QGeoLocation location;
            while (!(xml->isEndElement() &&
                    (xml->name() == "GeocodedAddress" ||
                     xml->name() == "ReverseGeocodedLocation"))) {
                if (xml->isStartElement() && xml->name() == "Point") {
                    QGeoCoordinate point = parsePoint(xml);
                    location.setCoordinate(point);
                }
                if (xml->isStartElement() && xml->name() == "Address") {
                    QGeoAddress address = parseAddress(xml);
                    location.setAddress(address);
                }
                if (xml->isStartElement() && xml->name() == "SearchCentreDistance") {
                }
                xml->readNext();
            }
            locations.append(location);
        }
        xml->readNext();
    }

    return locations;
}

void QGeoCodeReplyOrs::networkReplyFinished()
{
    if (!m_reply)
        return;

    if (m_reply->error() != QNetworkReply::NoError)
        return;

    QList<QGeoLocation> locations;
    QXmlStreamReader xml;
    QByteArray data = m_reply->readAll();
    xml.addData(data);

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == "Response") {
            int responsesNum = xml.attributes().value("numberOfResponses").toInt();
            if (responsesNum > 0) {
                locations = parseLocations(&xml);
            }
        }
    }
    if (xml.hasError()) {
        setError(QGeoCodeReply::ParseError, QStringLiteral("Error parsing OpenRouteService xml response:") + xml.errorString() + " at line: " + xml.lineNumber());
    } else {
        setLocations(locations);
        setFinished(true);
    }
    m_reply->deleteLater();
    m_reply = 0;
}

void QGeoCodeReplyOrs::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)

    if (!m_reply)
        return;

    setError(QGeoCodeReply::CommunicationError, m_reply->errorString());

    m_reply->deleteLater();
    m_reply = 0;
}
