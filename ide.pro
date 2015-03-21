TEMPLATE	= app
CONFIG		+= uitools
CONFIG		+= debug_and_release

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
			  src/imgconvert.cpp
HEADERS		= src/main.h \
			  src/ui.h \
			  src/project.h \
			  src/dskbuild.h \
			  src/bineditor.h \
			  src/gfxeditor.h \
			  src/imgconvert.h \
			  ui_ide_main.h
RESOURCES	= res/iderc.qrc
