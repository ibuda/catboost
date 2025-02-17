RESOURCES_LIBRARY()



IF (NOT QT_REQUIRED)
    MESSAGE(FATAL_ERROR "No QT Toolkit for your build")
ELSE()
    IF (OS_LINUX)
        IF (QT_VERSION STREQUAL "5.6.1")
            DECLARE_EXTERNAL_RESOURCE(QT sbr:1165641810)
        ELSE()
            ENABLE(QT_NOT_FOUND)
        ENDIF()
    ELSEIF (OS_DARWIN)
        IF (QT_VERSION STREQUAL "5.6.1")
            DECLARE_EXTERNAL_RESOURCE(QT sbr:1167023706)
            CFLAGS(GLOBAL "-F$QT_RESOURCE_GLOBAL/lib")
            LDFLAGS("-F$QT_RESOURCE_GLOBAL/lib")
        ELSE()
            ENABLE(QT_NOT_FOUND)
        ENDIF()
    ELSE()
        ENABLE(QT_NOT_FOUND)
    ENDIF()

    IF (QT_NOT_FOUND)
        MESSAGE(FATAL_ERROR "No QT Toolkit for the selected platform")
    ELSE()
        CFLAGS(GLOBAL "-I$QT_RESOURCE_GLOBAL/include")
        LDFLAGS("-L$QT_RESOURCE_GLOBAL/lib")
    ENDIF()
ENDIF()

END()
