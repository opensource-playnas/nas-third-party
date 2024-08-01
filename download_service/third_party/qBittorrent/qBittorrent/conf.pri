# Adapt these paths
# Point these to the include folders
INCLUDEPATH += $$quote($$PWD/../../qBittorrent/boost_1_81_0)
INCLUDEPATH += $$quote($$PWD/../../qBittorrent/install_msvc32/base/include)
INCLUDEPATH += $$quote($$PWD/../../qBittorrent/install_msvc32/qt5_15_11/include)
INCLUDEPATH += $$quote($$PWD/../../)
# Point these to the lib folders
LIBS += $$quote(-L$$PWD/../../qBittorrent/boost_1_81_0/stage/lib)
LIBS += $$quote(-L$$PWD/../../qBittorrent/install_msvc32/base/lib)
LIBS += $$quote(-L$$PWD/../../qBittorrent/install_msvc32/qt5_15_11/lib)

# Adapt the lib names/versions accordingly
# If you want to use Boost auto-linking then disable
# BOOST_ALL_NO_LIB below and omit Boost libraries here
LIBS += libcrypto.lib libssl.lib zlib.lib
CONFIG(debug, debug|release) {
    LIBS += libboost_system-vc141-mt-sgd-x32-1_81.lib libtorrentd.lib libtry_signald.lib
}
else {
    LIBS += libboost_system-vc141-mt-s-x32-1_81.lib libtorrent.lib libtry_signal.lib
}

# ...or if you use MinGW
#LIBS += libcrypto libssl libz
#CONFIG(debug, debug|release) {
#    LIBS += libtorrent-rasterbar \
#            libboost_system-mt
#}
#else {
#    LIBS += libtorrent-rasterbar \
#            libboost_system-mt
#}

# Disable to use Boost auto-linking
DEFINES += BOOST_ALL_NO_LIB
# Use one of the following options
DEFINES += BOOST_SYSTEM_STATIC_LINK
#DEFINES += BOOST_SYSTEM_DYN_LINK
# Enable it if compiling with libtorrent 3.x
#DEFINES += BOOST_SYSTEM_USE_UTF8

# Enable if libtorrent was built with this flag defined
#DEFINES += TORRENT_NO_DEPRECATE
# Enable if linking dynamically against libtorrent
#DEFINES += TORRENT_LINKING_SHARED

# Enable this if compiling with libtorrent 2.x
#DEFINES += QBT_USES_LIBTORRENT2

# Enable stack trace support
CONFIG += stacktrace

win32-msvc* {
    QMAKE_CXXFLAGS += "/guard:cf"
    QMAKE_LFLAGS += "/guard:cf"
    QMAKE_LFLAGS_RELEASE += "/OPT:REF /OPT:ICF"
}
