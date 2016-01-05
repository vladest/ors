#include "qgeoserviceproviderpluginors.h"
#include "qgeocodingmanagerengineors.h"
#include "qgeoroutingmanagerengineors.h"
#include "qplacemanagerengineors.h"

QGeoCodingManagerEngine *QGeoServiceProviderFactoryOrs::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoCodingManagerEngineOrs(parameters, error, errorString);
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryOrs::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoRoutingManagerEngineOrs(parameters, error, errorString);
}

