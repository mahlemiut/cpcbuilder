/*
 * bineditor.h
 *
 *  Created on: 1/09/2011
 *      Author: bsr
 */

#ifndef BINEDITOR_H_
#define BINEDITOR_H_

#include <QtWidgets>

class bineditor : public QFrame
{
	Q_OBJECT

public:
	bineditor(QWidget* parent = nullptr);
	~bineditor() { if(m_data != nullptr) delete m_data; }
	QWidget* parent() { return m_parent; }
	unsigned char* data() { return m_data; }
	void set_data(unsigned char* data, unsigned long long size);
protected:
	void paintEvent(QPaintEvent* event);
private:
	QWidget* m_parent;
	unsigned char* m_data;  // pointer to data to display
	unsigned long long m_datasize;  // size of data
};

#endif /* BINEDITOR_H_ */
