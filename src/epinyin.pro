TEMPLATE = app
# TEMPLATE = lib


QT       += core
TARGET   = epinyin
DESTDIR = $$PWD/../dist

SOURCES += \
    ime/spellingtrie.cpp \
    ime/dicttrie.cpp \
    ime/ngram.cpp \
    ime/dictlist.cpp \
    ime/candidates.cpp \
    ime/epinyin.cpp

HEADERS  += \
    ime/spellingtrie.h \
    ime/dictdef.h \
    ime/dicttrie.h \
    ime/ngram.h \
    ime/dictlist.h \
    ime/candidates.h \
    ime/epinyin.h

RESOURCES += \
    epinyin.qrc


equals(TEMPLATE, app) {

QT       += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui

}

