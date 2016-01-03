TARGET = qtgeoservices_ors
QT += location-private positioning-private network

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryOrs
load(qt_plugin)

HEADERS += \
    qgeoserviceproviderpluginors.h \
    qgeocodingmanagerengineors.h \
    qgeocodereplyors.h \
    qgeoroutingmanagerengineors.h \
    qgeoroutereplyors.h \
    qplacemanagerengineors.h \
    qplacesearchreplyors.h \
    qplacecategoriesreplyors.h

SOURCES += \
    qgeoserviceproviderpluginors.cpp \
    qgeocodingmanagerengineors.cpp \
    qgeocodereplyors.cpp \
    qgeoroutingmanagerengineors.cpp \
    qgeoroutereplyors.cpp \
    qplacemanagerengineors.cpp \
    qplacesearchreplyors.cpp \
    qplacecategoriesreplyors.cpp


OTHER_FILES += \
    ors_plugin.json

