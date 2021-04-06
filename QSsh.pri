INCLUDEPATH += $$PWD/src

HEADERS += \
        $$PWD/src/qsftpitem.h \
        $$PWD/src/qsshcommand.h \
        $$PWD/src/qsshconnection.h \
        $$PWD/src/qsshpull.h \
        $$PWD/src/qsshpush.h \
        $$PWD/src/qsshtask.h

SOURCES += \
        $$PWD/src/qsftpitem.cpp \
        $$PWD/src/qsshcommand.cpp \
        $$PWD/src/qsshconnection.cpp \
        $$PWD/src/qsshpull.cpp \
        $$PWD/src/qsshpush.cpp \
        $$PWD/src/qsshtask.cpp

LIBS += -L$$PWD/libssh/bin/lib -lssh


include($$PWD/QOption/qoption.pri)
