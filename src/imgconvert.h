/*
 * imgconvert.h
 *
 *  Created on: 15/01/2014
 */

#ifndef IMGCONVERT_H_
#define IMGCONVERT_H_

#include <stdio.h>
#include <QtGui>
#include <QString>
#include <QImage>

class imageconvert
{
public:
	imageconvert(QString fname);
	~imageconvert() {}
	bool convert(unsigned char* buffer, int buffer_size, int mode);
	//bool convert_scr(unsigned char* buffer, int buffer_size) {}  // TODO
	int get_width() { return m_width; }
	int get_height() { return m_height; }
	QColor get_colour(int col);
	int calculate_size(int mode)
	{
		if(!m_loaded)
			return 0;

		// For now, we'll stick with the default screen size
		return 16384;

		switch(mode)
		{
		default:
		case 0:
			return (m_image.width()/2) * m_image.height();
		case 1:
			return (m_image.width()/4) * m_image.height();
		case 2:
			return (m_image.width()/8) * m_image.height();
		case 3:
			return (m_image.width()/2) * m_image.height();
		}
	}
private:
	bool convert_mode0(unsigned char* buffer, int buffer_size);
	bool convert_mode1(unsigned char* buffer, int buffer_size);
	bool convert_mode2(unsigned char* buffer, int buffer_size);
	int m_width;  // set after conversion
	int m_height;  // set after conversion
	QImage m_image;
	bool m_loaded;  // true if image is loaded and ready, false if loading failed
};

#endif /* IMGCONVERT_H_ */
