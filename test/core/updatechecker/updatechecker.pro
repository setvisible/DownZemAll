#isEmpty(TEMPLATE):TEMPLATE=app
TARGET       = tst_updatechecker
CONFIG      += testcase
QT           = core testlib
SOURCES     += tst_updatechecker.cpp

CONFIG      += c++11

# Include:
#INCLUDEPATH += ../../../include

# Resources:
HEADERS     += $$PWD/../../../src/core/updatechecker_p.h
