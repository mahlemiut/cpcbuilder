/*
 * bineditor.cpp
 *
 *  Created on: 1/09/2011
 *      Author: bsr
 */

#include <QtGui>
#include "bineditor.h"

bineditor::bineditor(QWidget* parent) :
	QFrame(parent)
{
	m_data = nullptr;
	m_parent = parent;
	setFrameShape(QFrame::StyledPanel);
	setMinimumHeight(300);
	setMinimumWidth(500);
}

void bineditor::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QFont fnt("Courier New",14);
	QBrush bg(QColor(255,255,255));
	QFontMetrics metrics(fnt);
	unsigned char* ptr;
	int x;
	int locy=0;
	unsigned int counter=0;
	QString text;

	fnt.setBold(true);
	painter.setClipRect(event->rect());
	painter.setPen(QColor(0,0,0));
	painter.setBackground(bg);
	painter.setFont(fnt);
	painter.fillRect(event->rect(),bg);
	painter.setBackgroundMode(Qt::OpaqueMode);

	ptr = m_data;  // reset pointer to start of data

	while(counter < m_datasize)
	{
		text.clear();
		text += QString::number(counter,16).toUpper().rightJustified(4,'0');
		text += " : ";
		for(x=0;x<16;x++)
		{
			if(counter < m_datasize)
			{
				text += QString::number(*ptr,16).toUpper().rightJustified(2,'0');
				text += " ";
				ptr++;
			}
			counter++;
		}
		if((locy >= event->rect().top() && locy <= event->rect().bottom()) ||
			(locy+metrics.height() >= event->rect().top() && locy+metrics.height() <= event->rect().bottom()))
				painter.drawText(0,locy,metrics.width(text),locy+metrics.height(),Qt::AlignLeft,text);
		locy += metrics.height();
		setMinimumWidth(metrics.width(text));
	}
	setMinimumHeight(locy);

	QFrame::paintEvent(event);
}

void bineditor::set_data(unsigned char* data, unsigned long long size)
{
	if(m_data != nullptr)  // free data if it has already been set
		delete m_data;
	m_data = data;
	m_datasize = size;
}
