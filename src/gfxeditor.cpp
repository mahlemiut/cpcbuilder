/*
 * gfxeditor.cpp
 *
 *  Created on: 5/09/2011
 *      Author: bsr
 */

#include <QtGui>
#include "gfxeditor.h"

static const unsigned char default_palette[16] =
{
	16, 10, 6, 28, 11, 20, 21, 13, 7, 25, 23, 14, 18, 25, 0, 0
};

static const unsigned short default_12bit_palette[16] =
{
	0x0006, 0x0ff0, 0x0606, 0x0060, 0x0fff, 0x000f, 0x0600, 0x00ff,
	0x06f6, 0x0f66, 0x060f, 0x06f0, 0x0f00, 0x0f66, 0x0000, 0x0000
};

static const int cpc_colours[32][3] =
{
	{ 0x60, 0x60, 0x60 },		   /* white */
	{ 0x60, 0x60, 0x60 },		   /* white */
	{ 0x00, 0xff, 0x60 },		   /* sea green */
	{ 0xff, 0xff, 0x60 },		   /* pastel yellow */
	{ 0x00, 0x00, 0x60 },		   /* blue */
	{ 0xff, 0x00, 0x60 },		   /* purple */
	{ 0x00, 0x60, 0x60 },		   /* cyan */
	{ 0xff, 0x60, 0x60 },		   /* pink */
	{ 0xff, 0x00, 0x60 },		   /* purple */
	{ 0xff, 0xff, 0x60 },		   /* pastel yellow */
	{ 0xff, 0xff, 0x00 },		   /* bright yellow */
	{ 0xff, 0xff, 0xff },		   /* bright white */
	{ 0xff, 0x00, 0x00 },		   /* bright red */
	{ 0xff, 0x00, 0xff },		   /* bright magenta */
	{ 0xff, 0x60, 0x00 },		   /* orange */
	{ 0xff, 0x60, 0xff },		   /* pastel magenta */
	{ 0x00, 0x00, 0x60 },		   /* blue */
	{ 0x00, 0xff, 0x60 },		   /* sea green */
	{ 0x00, 0xff, 0x00 },		   /* bright green */
	{ 0x00, 0xff, 0xff },		   /* bright cyan */
	{ 0x00, 0x00, 0x00 },		   /* black */
	{ 0x00, 0x00, 0xff },		   /* bright blue */
	{ 0x00, 0x60, 0x00 },		   /* green */
	{ 0x00, 0x60, 0xff },		   /* sky blue */
	{ 0x60, 0x00, 0x60 },		   /* magenta */
	{ 0x60, 0xff, 0x60 },		   /* pastel green */
	{ 0x60, 0xff, 0x60 },		   /* lime */
	{ 0x60, 0xff, 0xff },		   /* pastel cyan */
	{ 0x60, 0x00, 0x00 },		   /* Red */
	{ 0x60, 0x00, 0xff },		   /* mauve */
	{ 0x60, 0x60, 0x00 },		   /* yellow */
	{ 0x60, 0x60, 0xff }		   /* pastel blue */
};


gfxeditor::gfxeditor(QWidget* parent) :
	QWidget(parent),
    m_data(nullptr),
	m_width(0),
	m_height(0),
    m_mode(1)
{
	unsigned char* paldata;
	int x;

	m_data = nullptr;
	m_parent = parent;

	// setup the child widget into a layout
	m_layout = new QGridLayout(dynamic_cast<QWidget*>(this));
	m_scrollarea = new QScrollArea(dynamic_cast<QWidget*>(m_layout));
	m_toolbar = new QToolBar(dynamic_cast<QWidget*>(m_layout));
	m_frame_palette = new paletteeditor(dynamic_cast<QWidget*>(m_layout));
	m_frame_gfx = new gfxdisplay(dynamic_cast<QWidget*>(this),m_frame_palette);
	m_combo_mode = new QComboBox(dynamic_cast<QWidget*>(m_toolbar));
	m_combo_zoom = new QComboBox(dynamic_cast<QWidget*>(m_toolbar));
	setLayout(m_layout);

	m_toolbar->addSeparator();

	// Add screen mode combo box to toolbar
	m_combo_mode->addItem("Mode 0");
	m_combo_mode->addItem("Mode 1");
	m_combo_mode->addItem("Mode 2");
	m_combo_mode->setCurrentIndex(1);
	m_toolbar->addWidget(m_combo_mode);
	connect(m_combo_mode,SIGNAL(currentIndexChanged(int)),m_frame_gfx,SLOT(set_mode(int)));
	connect(m_combo_mode,SIGNAL(currentIndexChanged(int)),this,SLOT(draw_scene()));

	m_toolbar->addSeparator();

	// add basic drawing function buttons to toolbar
	m_actgroup_drawing = new QActionGroup(this);
	QAction* act1 = new QAction("Plot",this);
	act1->setIcon(QIcon(":/images/plot.ico"));
	act1->setCheckable(true);
	act1->setChecked(true);
	connect(act1,SIGNAL(triggered()),m_frame_gfx,SLOT(draw_mode_point()));
	m_toolbar->addAction(act1);
	QAction* act2 = new QAction("Line",this);
	act2->setIcon(QIcon(":/images/line.ico"));
	connect(act2,SIGNAL(triggered()),m_frame_gfx,SLOT(draw_mode_line()));
	m_toolbar->addAction(act2);
	act2->setCheckable(true);
	QAction* act3 = new QAction("Box",this);
	act3->setIcon(QIcon(":/images/box.ico"));
	connect(act3,SIGNAL(triggered()),m_frame_gfx,SLOT(draw_mode_box()));
	m_toolbar->addAction(act3);
	act3->setCheckable(true);
	m_actgroup_drawing->addAction(act1);
	m_actgroup_drawing->addAction(act2);
	m_actgroup_drawing->addAction(act3);
	m_actgroup_drawing->setExclusive(true);

	m_toolbar->addSeparator();

	// add zoom level combo box to toolbar
	for(x=1;x<11;x++)
	{
		char str[6];
		sprintf(str,"%ix",x);
		m_combo_zoom->addItem(str);
	}
	m_combo_zoom->setCurrentIndex(0);
	m_toolbar->addWidget(m_combo_zoom);
	connect(m_combo_zoom,SIGNAL(currentIndexChanged(int)),m_frame_gfx,SLOT(set_zoom(int)));
//	connect(m_combo_zoom,SIGNAL(currentIndexChanged(int)),this,SLOT(draw_scene()));

	m_toolbar->addSeparator();

	// set default palette
	paldata = reinterpret_cast<unsigned char*>(malloc(PAL_SIZE_CPC));
	memcpy(paldata,&default_palette,PAL_SIZE_CPC);
	m_frame_palette->set_data(paldata,PAL_SIZE_CPC);
	delete(paldata);  // palette data is copied to the palette editor, so this isn't needed anymore

	paldata = reinterpret_cast<unsigned char*>(malloc(PAL_SIZE_PLUS));
	memcpy(paldata,&default_12bit_palette,PAL_SIZE_PLUS);
	m_frame_palette->set_data(paldata,PAL_SIZE_PLUS);
	delete(paldata);  // palette data is copied to the palette editor, so this isn't needed anymore

	m_scrollarea->setWidget(dynamic_cast<QWidget*>(m_frame_gfx));

	m_layout->addWidget(m_toolbar,0,0);
	m_layout->addWidget(m_scrollarea,1,0);
	m_layout->addWidget(m_frame_palette,1,1,Qt::AlignRight);
	m_layout->setColumnMinimumWidth(1,30);
	m_layout->setColumnStretch(0,100);
	m_layout->setColumnStretch(1,1);

	// setup gfxdisplay
	m_frame_gfx->setCursor(Qt::CrossCursor);
//	m_frame_gfx->setAlignment(Qt::AlignTop | Qt::AlignLeft);
//	m_frame_gfx->setScene(&m_scene);
	m_frame_gfx->set_mode(1);
	m_frame_gfx->set_format_screen();
}

gfxeditor::~gfxeditor()
{
//	if(m_data != NULL)
//		delete m_data;
}

QColor gfxeditor::get_colour(unsigned int pen)
{
	return QColor(cpc_colours[pen][0],cpc_colours[pen][1],cpc_colours[pen][2]);
}

void gfxeditor::set_data(unsigned char* data, unsigned long long size)
{
	if(m_data != nullptr)
		free(m_data);
	m_data = data;
	m_datasize = size;
	m_frame_gfx->set_data(data,m_width,m_height);
}

bool gfxeditor::load_pal_normal(unsigned char* data)
{
	unsigned int x;

	if(data == nullptr)
		return false;

	for(x=0;x<16;x++)
		m_frame_palette->set_colour(data[x] % 32,x);

	return true;
}

bool gfxeditor::load_pal_12bit(unsigned short* data)
{
	unsigned int x;

	if(data == nullptr)
		return false;

	for(x=0;x<16;x++)
		m_frame_palette->set_colour_12(data[x] & 0x0fff,x);

	return true;
}

bool gfxeditor::toggle_pal()
{
	return m_frame_palette->toggle_pal();
}

void gfxeditor::export_palette_to_clipboard()
{
	m_frame_palette->export_palette_to_clipboard();
}

// plot a pixel at the location given, in the selected pen
void gfxeditor::plot(int x, int y)
{
	unsigned int loc = 0;
	unsigned char mask = 0;  // bit mask for the pixel written
	int pen = m_frame_palette->get_selected();
	unsigned char col = 0;

	// the gfx editor uses the CPC screen layout
	if(m_frame_gfx->get_format() == SCR_FORMAT_SCREEN)
	{
		loc = m_width * (y / 8);
		loc += 0x800 * (y % 8);
	}
	else if(m_frame_gfx->get_format() == SCR_FORMAT_LINEAR)
		loc = m_width * y;

	switch(m_frame_gfx->get_mode())
	{
	case 2:
		loc += (x / 8);
		mask = 0x01 << (7 - (x % 8));
		col = (pen & 0x01) << (7 - (x % 8));
		break;
	case 1:
		loc += (x / 4);
		mask = 0x11 << (3 - (x % 4));
		col = ((pen & 0x02) << 2) >> (x % 4);
		col |= ((pen & 0x01) << 7) >> (x % 4);
		break;
	case 3:
		pen &= 0x03;
	[[fallthrough]];
	case 0:
		loc += (x / 2);
		mask = 0x55 << (1 - (x % 2));
		col = ((pen & 0x08) >> 2) >> (x % 2);
		col |= ((pen & 0x04) << 3) >> (x % 2);
		col |= ((pen & 0x02) << 2) >> (x % 2);
		col |= ((pen & 0x01) << 7) >> (x % 2);
		break;
	}
	if(loc < m_datasize)
    {
        unsigned char data = m_data[loc];
        m_data[loc] = (data &= ~mask) | col;
    }
}

// draw a line
void gfxeditor::line(int x0, int y0, int x1, int y1)
{
	bool done = false;
	int sx,sy,error,err2;
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	if(x0 < x1)
		sx = 1;
	else
		sx = -1;
	if(y0 < y1)
		sy = 1;
	else
		sy = -1;
	error = dx - dy;

	while(!done)
	{
		plot(x0,y0);
		if((x0 == x1) && (y0 == y1))
			done = true;
		else
		{
			err2 = error * 2;
			if(err2 > -dy)
			{
				error -= dy;
				x0 += sx;
			}
			if(err2 < dx)
			{
				error += dx;
				y0 += sy;
			}
		}
	}
}

// draw a box
void gfxeditor::box(int x0, int y0, int x1, int y1)
{
	int xdir = 1;
	int ydir = 1;
	int x,y;

	// check if end co-ords are above and/or left of first click
	if(x1 < x0)
		xdir = -xdir;
	if(y1 < y0)
		ydir = -ydir;

	for(x=x0;x!=x1;x+=xdir)
	{
		plot(x,y0);
		plot(x,y1);
	}
	for(y=y0;y!=y1;y+=ydir)
	{
		plot(x0,y);
		plot(x1,y);
	}
	plot(x1,y1);
}

void gfxeditor::mousePressEvent(QMouseEvent* event)
{
	QWidget::mousePressEvent(event);
}

//-------------------------------
// Tile editor widget code

tileeditor::tileeditor(QWidget* parent) :
		gfxeditor(parent),
		m_tile_current(1),
		m_tile_total(1)
{
	m_label_tile_left = new QLabel(dynamic_cast<QWidget*>(m_toolbar));
	m_label_tile_right = new QLabel(dynamic_cast<QWidget*>(m_toolbar));
	m_spin_tile = new QSpinBox(dynamic_cast<QWidget*>(m_toolbar));

	// add tile selection to toolbar
	m_label_tile_left->setText("Tile");
	m_spin_tile->setValue(1);
	m_toolbar->addWidget(m_label_tile_left);
	m_toolbar->addWidget(m_spin_tile);
	m_toolbar->addWidget(m_label_tile_right);

	connect(m_spin_tile,SIGNAL(valueChanged(int)),this,SLOT(current_tile_changed(int)));

	// add tile add/remove buttons to toolbar
	m_act_addtile = new QAction("Add tile",this);
	m_act_addtile->setIcon(QIcon(":/images/add.ico"));
	connect(m_act_addtile,SIGNAL(triggered()),this,SLOT(add_gfx_tile()));
	m_toolbar->addAction(m_act_addtile);

	m_act_deltile = new QAction("Remove tile",this);
	m_act_deltile->setIcon(QIcon(":/images/remove.ico"));
	connect(m_act_deltile,SIGNAL(triggered()),this,SLOT(remove_gfx_tile()));
	m_toolbar->addAction(m_act_deltile);

	m_label_tile_right->setText(QString("of %1").arg(get_tile_num()));

	m_toolbar->addSeparator();
	m_frame_gfx->set_format_linear();
}

tileeditor::~tileeditor()
{
}

void tileeditor::calculate_tiles()
{
	// calculate total gfx tiles in data
	if(m_width != 0 && m_height != 0)
	{
		m_tile_total = m_datasize / (m_width * m_height);
		m_label_tile_right->setText(QString("of %1").arg(get_tile_num()));
		m_spin_tile->setRange(1,m_tile_total);
	}
	else
	{
		m_label_tile_right->setText("of ??");
		m_spin_tile->setRange(1,1);
	}
}

void tileeditor::add_gfx_tile()
{
	unsigned char* new_data;
	m_datasize += (m_width * m_height);
	new_data = reinterpret_cast<unsigned char*>(realloc(m_data,m_datasize));
	if(new_data != nullptr)
	{
		m_data = new_data;
		memset(new_data+(m_tile_total*(m_width*m_height)),0,(m_width*m_height));  // clear data in new tile
		m_frame_gfx->realloc_data(new_data);
	}
	else
		m_datasize -= (m_width * m_height);  // revert size change
	calculate_tiles();
}

void tileeditor::remove_gfx_tile()
{
	unsigned char* new_data;
	if(m_datasize <= static_cast<unsigned long long>(m_width * m_height))
		return;  // Only one tile left, no point deleting that.
	m_datasize -= (m_width * m_height);
	new_data = reinterpret_cast<unsigned char*>(realloc(m_data,m_datasize));
	// TODO: perhaps move data after the current tile back so that it's the current tile that is deleted
	if(new_data != nullptr)
	{
		m_data = new_data;
		m_frame_gfx->realloc_data(new_data);
	}
	else
		m_datasize += (m_width * m_height);  // revert size change
	calculate_tiles();
}

void tileeditor::set_data(unsigned char* data, unsigned long long size)
{
	gfxeditor::set_data(data,size);
	calculate_tiles();
}

// plot a pixel at the location given, in the selected pen
void tileeditor::plot(unsigned int x, unsigned int y)
{
	unsigned int loc;
	unsigned char mask = 0;  // bit mask for the pixel written
	int pen = m_frame_palette->get_selected();
	unsigned char col = 0;

	// the tile editor does not use the CPC screen layout, it is stored linearly
	loc = ((m_tile_current-1) * (m_width*m_height));  // go to start of current tile data
	loc += m_width * y;

	switch(m_frame_gfx->get_mode())
	{
	case 2:
		loc += (x / 8);
		mask = 0x01 << (7 - (x % 8));
		col = (pen & 0x01) << (7 - (x % 8));
		break;
	case 1:
		loc += (x / 4);
		mask = 0x11 << (3 - (x % 4));
		col = ((pen & 0x02) << 2) >> (x % 4);
		col |= ((pen & 0x01) << 7) >> (x % 4);
		break;
	case 3:
		pen &= 0x03;
	[[fallthrough]];
	case 0:
		loc += (x / 2);
		mask = 0x55 << (1 - (x % 2));
		col = ((pen & 0x08) >> 2) >> (x % 2);
		col |= ((pen & 0x04) << 3) >> (x % 2);
		col |= ((pen & 0x02) << 2) >> (x % 2);
		col |= ((pen & 0x01) << 7) >> (x % 2);
		break;
	}
	if(loc < m_datasize)
    {
        unsigned char data = m_data[loc];
        m_data[loc] = (data &= ~mask) | col;
    }
}


//-------------------------------
// Palette Editor widget code

paletteeditor::paletteeditor(QWidget* parent) :
		QFrame(parent),
		m_mode(0),
		m_selected(0),
		m_12bit_palette(false)
{
	m_parent = parent;
	setFrameShape(QFrame::StyledPanel);
	setMinimumWidth(30);
}

void paletteeditor::set_data(unsigned char* data, unsigned int size)
{
	if(size != PAL_SIZE_CPC && size != PAL_SIZE_PLUS)
		return;
	if(size == PAL_SIZE_CPC)
		memcpy(m_data,data,size);
	if(size == PAL_SIZE_PLUS)
		memcpy(reinterpret_cast<unsigned char*>(m_12bit_data),data,size);
	m_palette_size = size;
}

void paletteeditor::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect rect(5,5,20,20);
	int x,count=16;

	painter.setPen(QColor(0,0,0));

	switch(m_mode)
	{
	case 0:
		count = 16;
		break;
	case 1:
	case 3:
		count = 4;
		break;
	case 2:
		count = 2;
		break;
	}
	if(m_12bit_palette == false)
	{
		for(x=0;x<count;x++)
		{
			painter.setBrush(get_colour(*(m_data+x)));
			painter.drawRect(rect);
			rect.adjust(0,20,0,20);
		}
	}
	else
	{
		for(x=0;x<count;x++)
		{
			int r = (*(m_12bit_data+x) & 0x00f0);
			int g = (*(m_12bit_data+x) & 0x0f00) >> 4;
			int b = (*(m_12bit_data+x) & 0x000f) << 4;
			painter.setBrush(QColor(r,g,b));
			painter.drawRect(rect);
			rect.adjust(0,20,0,20);
		}
	}

	rect.setCoords(5,5+(m_selected*20),24,24+(m_selected*20));
	painter.setPen(QColor(255,0,0));
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(rect);

	QFrame::paintEvent(event);
}

QColor paletteeditor::get_colour(unsigned int pen)
{
	return QColor(cpc_colours[pen][0],cpc_colours[pen][1],cpc_colours[pen][2]);
}

void paletteeditor::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		int selected;

		// determine which box has been clicked
		if(event->pos().x() < 5 || event->pos().x() > 25)
			return;
		if(event->pos().y() < 5 || event->pos().y() > 20*16)
			return;
		selected = (event->pos().y()-5) / 20;

		m_selected = selected;

		repaint();
	}
}

void paletteeditor::mouseDoubleClickEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		unsigned int selected;

		// determine which box has been clicked
		if(event->pos().x() < 5 || event->pos().x() > 25)
			return;
		if(event->pos().y() < 5 || event->pos().y() > 20*16)
			return;
		selected = (static_cast<unsigned int>(event->pos().y())-5) / 20;

		// selected new colour
		QColor colour = QColorDialog::getColor();
		if(!colour.isValid())
			return;

		// convert to nearest CPC colour
		if(colour.blue() < 30)
			colour.setBlue(0);
		else if(colour.blue() > 196)
			colour.setBlue(255);
		else colour.setBlue(96);
		if(colour.red() < 30)
			colour.setRed(0);
		else if(colour.red() > 196)
			colour.setRed(255);
		else colour.setRed(96);
		if(colour.green() < 30)
			colour.setGreen(0);
		else if(colour.green() > 196)
			colour.setGreen(255);
		else colour.setGreen(96);

		// for the standard CPC, find the matching hardware colour
		set_pen(selected,colour);

		// and repaint the widget
		repaint();
	}
}

bool paletteeditor::set_pen(unsigned int index, QColor colour)
{
	unsigned char x;

	for(x=0;x<32;x++)
	{
		if(colour.blue() == cpc_colours[x][2] && colour.red() == cpc_colours[x][0] && colour.green() == cpc_colours[x][1])
		{
			set_colour(x,index);
			return true;
		}
	}

	QMessageBox::warning(this,tr("Palette error"),tr("Cannot set graphic pen, unable to find pen in lookup table.  This shouldn't happen. :/"));

	return false;
}

QColor paletteeditor::get_pen(unsigned int index)
{
	QColor colour;

	if(index < m_palette_size)
	{
		if(m_12bit_palette == false)
			colour = get_colour(m_data[index]);
		else
		{
			colour.setRed((m_12bit_data[index] & 0x00f0));
			colour.setGreen((m_12bit_data[index] & 0x0f00) >> 4);
			colour.setBlue((m_12bit_data[index] & 0x000f) << 4);
		}
	}

	return colour;
}

bool paletteeditor::toggle_pal()
{
	if(m_12bit_palette == true)
		m_12bit_palette = false;
	else
		m_12bit_palette = true;

	repaint();
	return m_12bit_palette;
}

void paletteeditor::export_palette_to_clipboard()
{
	QClipboard* clip = QApplication::clipboard();
	QString txt;

	if(!m_12bit_palette)
	{
		int x;
		txt = "defb ";
		txt += QString("%1").arg(m_data[0]);
		for(x=1;x<16;x++)
		{
			txt += ",";
			txt += QString("%1").arg(m_data[x]);
		}
	}
	else
	{
		int x;
		txt = "defw ";
		txt += QString("%1").arg(m_12bit_data[0]);
		for(x=1;x<16;x++)
		{
			txt += ",";
			txt += QString("%1").arg(m_12bit_data[x]);
		}
	}
	clip->setText(txt);
}

// gfxdisplay functions
gfxdisplay::gfxdisplay(QWidget* parent, paletteeditor* pal) :
		QFrame(parent),
		m_data(nullptr),
		m_palette(pal),
		m_x(0),
		m_y(0),
		m_zoom(1),
		m_mode(1),
		m_draw_mode(GFX_DRAW_POINT),
		m_line_click(false),
		m_box_click(false),
		m_tile_current(1),
		m_format(SCR_FORMAT_LINEAR)
{
	setMouseTracking(true);
}

void gfxdisplay::set_data(unsigned char* data, unsigned int x, unsigned int y)
{
	if(m_data != nullptr)
	{
		delete m_data;
		m_data = nullptr;
	}

	if(data == nullptr)
	{
		m_width = 0;
		m_height = 0;
		resize(0,0);
		return;
	}

	m_data = data;
	m_width = x;
	m_height = y;
	resize((m_width * 8) * m_zoom,(m_height * m_zoom) * 2);
}

unsigned int gfxdisplay::get_pen(unsigned int x, unsigned char byte)
{
	int loc = x % 8;
	unsigned int pen = 0;

	switch(m_mode)
	{
	case 0:
		if(loc < 4)
			pen = ((byte & 0x80) >> 7) | ((byte & 0x20) >> 3) | ((byte & 0x08) >> 2) | ((byte & 0x02) << 2);
		else
			pen = ((byte & 0x40) >> 6) | ((byte & 0x10) >> 2) | ((byte & 0x04) >> 1) | ((byte & 0x01) << 3);
		break;
	case 1:
		if(loc == 0 || loc == 1)
			pen = ((byte & 0x80) >> 7) | ((byte & 0x08) >> 2);
		if(loc == 2 || loc == 3)
			pen = ((byte & 0x40) >> 6) | ((byte & 0x04) >> 1);
		if(loc == 4 || loc == 5)
			pen = ((byte & 0x20) >> 5) | ((byte & 0x02));
		if(loc == 6 || loc == 7)
			pen = ((byte & 0x10) >> 4) | ((byte & 0x01) << 1);
		break;
	case 2:
		switch(loc)
		{
		case 0:
			pen = ((byte & 0x80) >> 7);
			break;
		case 1:
			pen = ((byte & 0x40) >> 6);
			break;
		case 2:
			pen = ((byte & 0x20) >> 5);
			break;
		case 3:
			pen = ((byte & 0x10) >> 4);
			break;
		case 4:
			pen = ((byte & 0x08) >> 3);
			break;
		case 5:
			pen = ((byte & 0x04) >> 2);
			break;
		case 6:
			pen = ((byte & 0x02) >> 1);
			break;
		case 7:
			pen = (byte & 0x01);
			break;
		}
		break;
	case 3:  // not an official mode
		if(loc < 4)
			pen = ((byte & 0x08) >> 2) | ((byte & 0x02) << 2);
		else
			pen = ((byte & 0x04) >> 1) | ((byte & 0x01) << 3);
		break;
	}
	return pen;
}

void gfxdisplay::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect display = event->rect();
	int x,y;

	if (m_zoom <= 0)
		return;

	if (m_data == nullptr)
		return;

	if(m_format == SCR_FORMAT_LINEAR)
	{
		for(y=display.top();y<=display.bottom();y++)  // scanline
		{
			for(x=display.left();x<=display.right();x++)  // pixel
			{
				unsigned int px = static_cast<unsigned int>(x), py = static_cast<unsigned int>(y);
				unsigned int ptr,pen;

				convert_pixel(&px,&py);
				// determine which part of the gfx data to draw
				if(px < m_width && py < m_height)
				{
					ptr = (m_width * m_height) * (m_tile_current - 1);
					ptr += (m_width * py) + px;
					pen = get_pen((x / m_zoom),m_data[ptr]);
					painter.setPen(QPen(m_palette->get_pen(pen)));
					painter.drawPoint(x,y);
				}
			}
		}
	}
	else // SCR_FORMAT_SCREEN
	{
		for(y=display.top();y<=display.bottom();y++)  // scanline
		{
			for(x=display.left();x<=display.right();x++)  // pixel
			{
				unsigned int px = x, py = y;
				unsigned int ptr,pen;

				convert_pixel(&px,&py);
				// determine which part of the gfx data to draw
				if(px < m_width && py < m_height)
				{
					ptr = (py & 0x0007) * 0x800;  // row address
					ptr += (py >> 3) * 80; // column
					ptr += px; // byte
					pen = get_pen((x / m_zoom),m_data[ptr]);
					painter.setPen(QPen(m_palette->get_pen(pen)));
					painter.drawPoint(x,y);
				}
			}
		}
	}

	// draw an effect line or box if we are in the middle of drawing a line or box
	if(m_draw_mode == GFX_DRAW_LINE && m_line_click == true)
	{
		painter.setPen(QPen(Qt::white,1,Qt::DashLine));
		painter.drawLine(m_guide_x0,m_guide_y0,m_guide_x1,m_guide_y1);
	}
	if(m_draw_mode == GFX_DRAW_BOX && m_box_click == true)
	{
		painter.setPen(QPen(Qt::white,1,Qt::DashLine));
		painter.drawRect(m_guide_x0,m_guide_y0,m_guide_x1-m_guide_x0,m_guide_y1-m_guide_y0);
	}
}

void gfxdisplay::mousePressEvent(QMouseEvent* event)
{
	int x,y;
	int lx,ly;
	int draw_mode = get_draw_mode();
	QWidget* viewport = dynamic_cast<QWidget*>(parent());
	QScrollArea* area = dynamic_cast<QScrollArea*>(viewport->parent());
	gfxeditor* editor = dynamic_cast<gfxeditor*>(area->parent());

	QWidget::mousePressEvent(event);

	if(event->button() != Qt::LeftButton)
		return;

	// Adjust to the gfxview object coordinates
	x = event->x();
	y = event->y();
	x += area->horizontalScrollBar()->value();
	y += area->verticalScrollBar()->value();

	lx = x;
	ly = y;

	convert_coords(reinterpret_cast<unsigned int*>(&x),reinterpret_cast<unsigned int*>(&y));

	if(x > static_cast<int>(get_width_pixel())-1)
		return;

	if(y > static_cast<int>(get_height_pixel())-1)
		return;


	if(draw_mode == GFX_DRAW_LINE && line_guide() == false)
	{
		m_hold_x = x;
		m_hold_y = y;
		set_guide_begin(lx,ly);
		line_guide_active(true);
		return;
	}

	if(draw_mode == GFX_DRAW_BOX && box_guide() == false)
	{
		m_hold_x = x;
		m_hold_y = y;
		set_guide_begin(lx,ly);
		box_guide_active(true);
		return;
	}

	if(draw_mode == GFX_DRAW_LINE && line_guide() == true)
	{
		line_guide_active(false);
		editor->line(m_hold_x,m_hold_y,x,y);
		editor->draw_scene();
		return;
	}
	if(draw_mode == GFX_DRAW_BOX && box_guide() == true)
	{
		box_guide_active(false);
		editor->box(m_hold_x,m_hold_y,x,y);
		editor->draw_scene();
		return;
	}

	if(draw_mode == GFX_DRAW_POINT)
	{
		editor->plot(x,y);
		editor->draw_scene();
	}
}

void gfxdisplay::mouseMoveEvent(QMouseEvent* event)
{
	QWidget* viewport = dynamic_cast<QWidget*>(parent());
	QScrollArea* area = dynamic_cast<QScrollArea*>(viewport->parent());
	gfxeditor* widget = dynamic_cast<gfxeditor*>(area->parent());
	m_guide_x1 = event->x()	+ area->horizontalScrollBar()->value();
	m_guide_y1 = event->y() + area->verticalScrollBar()->value();
	if(widget)
		widget->draw_scene();
	QWidget::mouseMoveEvent(event);
}

// converts widget coordinates to scanline/byte coordinates
void gfxdisplay::convert_coords(unsigned int *x, unsigned int *y)
{
	unsigned int mode_width[4] = { 4, 2, 1, 4 };
	unsigned int pixel_width = mode_width[get_mode()];
	unsigned int xp,yp;

	xp = *x / (pixel_width * m_zoom);
//	if(m_format == SCR_FORMAT_LINEAR)
		yp = *y / (m_zoom * 2);
//	else // SCR_FORMAT_SCREEN
//	{
//		yp = *y / (m_zoom * 2);
//		yp = ((yp % 8) * (m_height / 8)) + (yp >> 3);
//	}

	*x = xp;
	*y = yp;
}

// converts widget coordinates to byte-based co-ordinates
void gfxdisplay::convert_pixel(unsigned int *x, unsigned int *y)
{
	unsigned int xp,yp;

	xp = *x / (8 * m_zoom);
//	if(m_format == SCR_FORMAT_LINEAR)
		yp = *y / (m_zoom * 2);  // mode 2 pixels are higher than they are wide, mode 1 pixels are more or less square
//	else  // SCR_FORMAT_SCREEN
//	{
//		yp = *y / (m_zoom * 2);
//		yp = ((yp % 8) * (m_height / 8)) + (yp >> 3);
//	}

	*x = xp;
	*y = yp;
}

