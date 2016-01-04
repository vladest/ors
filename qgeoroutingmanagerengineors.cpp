/****************************************************************************
**
** Copyright (C) 2013 Aaron McCarthy <mccarthy.aaron@gmail.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgeoroutingmanagerengineors.h"
#include "qgeoroutereplyors.h"

#include <QtCore/QUrlQuery>

#include <QtCore/QDebug>

QGeoRoutingManagerEngineOrs::QGeoRoutingManagerEngineOrs(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString)
:   QGeoRoutingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("ors.useragent")))
        m_userAgent = parameters.value(QStringLiteral("ors.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    if (parameters.contains(QStringLiteral("ors.routing.host")))
        m_urlPrefix = parameters.value(QStringLiteral("ors.routing.host")).toString().toLatin1();
    else
        m_urlPrefix = QStringLiteral("http://openls.geog.uni-heidelberg.de/route");

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoRoutingManagerEngineOrs::~QGeoRoutingManagerEngineOrs()
{
}

QGeoRouteReply* QGeoRoutingManagerEngineOrs::calculateRoute(const QGeoRouteRequest &request)
{
    QNetworkRequest networkRequest;
    networkRequest.setRawHeader("User-Agent", m_userAgent);

    QUrl url(m_urlPrefix);
    QUrlQuery query;

    foreach (const QGeoCoordinate &c, request.waypoints()) {
        if (c == request.waypoints().first())
            query.addQueryItem(QStringLiteral("start"), QString::number(c.longitude()) + QLatin1Char(',') +
                               QString::number(c.latitude()));
        else if (c == request.waypoints().last())
            query.addQueryItem(QStringLiteral("end"), QString::number(c.longitude()) + QLatin1Char(',') +
                               QString::number(c.latitude()));
        else
            query.addQueryItem(QStringLiteral("via"), QString::number(c.longitude()) + QLatin1Char(',') +
                               QString::number(c.latitude()));
    }
    if (request.waypoints().count() == 2) { //need empty via
        query.addQueryItem(QStringLiteral("via"), "");
    }

    if (request.travelModes() & QGeoRouteRequest::CarTravel)
        query.addQueryItem(QStringLiteral("routepref"), QStringLiteral("Car"));
    if (request.travelModes() & QGeoRouteRequest::PedestrianTravel)
        query.addQueryItem(QStringLiteral("routepref"), QStringLiteral("Pedestrian"));
    if (request.travelModes() & QGeoRouteRequest::BicycleTravel)
        query.addQueryItem(QStringLiteral("routepref"), QStringLiteral("Bicycle"));
    if (request.travelModes() & QGeoRouteRequest::TruckTravel)
        query.addQueryItem(QStringLiteral("routepref"), QStringLiteral("HeavyVehicle"));

    if (request.routeOptimization() == QGeoRouteRequest::ShortestRoute)
        query.addQueryItem(QStringLiteral("weighting"), QStringLiteral("Shortest"));
    if (request.routeOptimization() == QGeoRouteRequest::FastestRoute)
        query.addQueryItem(QStringLiteral("weighting"), QStringLiteral("Fastest"));
    if (request.routeOptimization() == QGeoRouteRequest::MostEconomicRoute ||
            request.routeOptimization() == QGeoRouteRequest::MostScenicRoute)
        query.addQueryItem(QStringLiteral("weighting"), QStringLiteral("Recommended"));

    foreach (QGeoRouteRequest::FeatureType routeFeature, request.featureTypes()) {
        QGeoRouteRequest::FeatureWeight weigth = request.featureWeight(routeFeature);
        QString osrFeature;
        QString enabled = QStringLiteral("false");

        if (routeFeature == QGeoRouteRequest::TollFeature)
            osrFeature = QStringLiteral("noTollways");
        if (routeFeature == QGeoRouteRequest::HighwayFeature)
            osrFeature = QStringLiteral("noMotorways");
        if (routeFeature == QGeoRouteRequest::FerryFeature)
            osrFeature = QStringLiteral("noFerries");
        if (routeFeature == QGeoRouteRequest::DirtRoadFeature)
            osrFeature = QStringLiteral("noUnpavedroads");

        if (weigth == QGeoRouteRequest::AvoidFeatureWeight
                || weigth == QGeoRouteRequest::DisallowFeatureWeight)
            enabled = QStringLiteral("true");

        query.addQueryItem(osrFeature, enabled);
    }
    //ORS requires all features to befilled up. So check what was not yet filled
    if (!request.featureTypes().contains(QGeoRouteRequest::TollFeature))
        query.addQueryItem(QStringLiteral("noTollways"), QStringLiteral("false"));
    if (!request.featureTypes().contains(QGeoRouteRequest::HighwayFeature))
        query.addQueryItem(QStringLiteral("noMotorways"), QStringLiteral("false"));
    if (!request.featureTypes().contains(QGeoRouteRequest::FerryFeature))
        query.addQueryItem(QStringLiteral("noFerries"), QStringLiteral("false"));
    if (!request.featureTypes().contains(QGeoRouteRequest::DirtRoadFeature))
        query.addQueryItem(QStringLiteral("noUnpavedroads"), QStringLiteral("false"));

    //always try to add steps
    query.addQueryItem(QStringLiteral("noSteps"), QStringLiteral("false"));

    //TODO: check Locale
    query.addQueryItem(QStringLiteral("lang"), QStringLiteral("en"));

    //TODO: check Locale
    query.addQueryItem(QStringLiteral("distunit"), QStringLiteral("M"));

    if (request.maneuverDetail() == QGeoRouteRequest::NoManeuvers)
        query.addQueryItem(QStringLiteral("instructions"), QStringLiteral("false"));
    else
        query.addQueryItem(QStringLiteral("instructions"), QStringLiteral("true"));

    url.setQuery(query);
    networkRequest.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(networkRequest);

    QGeoRouteReplyOrs *routeReply = new QGeoRouteReplyOrs(reply, request, this);

    connect(routeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(routeReply, SIGNAL(error(QGeoRouteReply::Error,QString)),
            this, SLOT(replyError(QGeoRouteReply::Error,QString)));

    return routeReply;
}

void QGeoRoutingManagerEngineOrs::replyFinished()
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoRoutingManagerEngineOrs::replyError(QGeoRouteReply::Error errorCode,
                                             const QString &errorString)
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}
