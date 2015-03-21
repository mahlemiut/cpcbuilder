/*
 * main.cpp
 *
 *  Created on: 10/08/2011
 *      Author: bsr
 */

#include <stdio.h>
#include <stdlib.h>
#include "ui.h"

int main(int argc, char* argv[])
{
	QApplication app(argc,argv);
	QMainWindow* main_win = new ui_main;

	main_win->show();
	return app.exec();
}

