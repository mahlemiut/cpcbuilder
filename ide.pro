TEMPLATE	= app
QT		+= uitools
QT              += widgets
CONFIG		+= debug_and_release
QMAKE_CXXFLAGS += -Wno-implicit-fallthrough

CONFIG(debug, debug|release) {
	TARGET = cpcbuilder_debug
} else {
	TARGET = cpcbuilder
}

FORMS		= res/ide_main.ui
SOURCES		= src/main.cpp \
			  src/ui.cpp \
			  src/project.cpp \
			  src/dskbuild.cpp \
			  src/bineditor.cpp \
			  src/gfxeditor.cpp \
			  src/imgconvert.cpp \
                          src/appsettings.cpp
HEADERS		= src/main.h \
			  src/ui.h \
			  src/project.h \
			  src/dskbuild.h \
			  src/bineditor.h \
			  src/gfxeditor.h \
                          src/imgconvert.h \
                          src/appsettings.h
RESOURCES	= res/iderc.qrc
