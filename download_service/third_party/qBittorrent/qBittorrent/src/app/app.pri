INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/application.h \
    $$PWD/applicationinstancemanager.h \
    $$PWD/cmdoptions.h \
    $$PWD/filelogger.h \
    $$PWD/magnet_download_task_impl.h \
    $$PWD/qtlocalpeer/qtlocalpeer.h \
    $$PWD/signalhandler.h \
    $$PWD/subscibe_event.h \
    $$PWD/torrent_file_download_task_impl.h \
    $$PWD/torrent_helper.h \
    $$PWD/torrent_settings.h \
    $$PWD/torrent_task_public_define.h \
    $$PWD/upgrade.h

SOURCES += \
    $$PWD/application.cpp \
    $$PWD/applicationinstancemanager.cpp \
    $$PWD/cmdoptions.cpp \
    $$PWD/filelogger.cpp \
    $$PWD/magnet_download_task_impl.cpp \
    $$PWD/main.cpp \
    $$PWD/qtlocalpeer/qtlocalpeer.cpp \
    $$PWD/signalhandler.cpp \
    $$PWD/subscibe_event.cpp \
    $$PWD/torrent_file_download_task_impl.cpp \
    $$PWD/torrent_helper.cpp \
    $$PWD/torrent_settings.cpp \
    $$PWD/upgrade.cpp

stacktrace {
    HEADERS += $$PWD/stacktrace.h
    SOURCES += $$PWD/stacktrace.cpp
}
