/*********************************************************************************************************
*
* File                : epd.c
* Hardware Environment:
* Build Environment   : RealView MDK-ARM  Version: 4.74
* Version             : V1.0
* By                  : V
*
*                                  (c) Copyright 2005-2014, WaveShare
*                                       http://www.waveshare.net
*                                          All Rights Reserved
*
*********************************************************************************************************/
#include "K32_epd.h"
#include "K32.h"
#include <Arduino.h>



const int wake_up = 12;
const int reset = 14;


/*
command define
*/
static const unsigned char _cmd_handshake[8] = {0xA5, 0x00, 0x09, CMD_HANDSHAKE, 0xCC, 0x33, 0xC3, 0x3C};				//CMD_HANDSHAKE
static const unsigned char _cmd_read_baud[8] = {0xA5, 0x00, 0x09, CMD_READ_BAUD, 0xCC, 0x33, 0xC3, 0x3C};				//CMD_READ_BAUD
static const unsigned char _cmd_stopmode[8] = {0xA5, 0x00, 0x09, CMD_STOPMODE, 0xCC, 0x33, 0xC3, 0x3C}; 				//CMD_STOPMODE
static const unsigned char _cmd_update[8] = {0xA5, 0x00, 0x09, CMD_UPDATE, 0xCC, 0x33, 0xC3, 0x3C}; 					//CMD_UPDATE
static const unsigned char _cmd_load_font[8] = {0xA5, 0x00, 0x09, CMD_LOAD_FONT, 0xCC, 0x33, 0xC3, 0x3C};				//CMD_LOAD_FONT
static const unsigned char _cmd_load_pic[8] = {0xA5, 0x00, 0x09, CMD_LOAD_PIC, 0xCC, 0x33, 0xC3, 0x3C}; 				//CMD_LOAD_PIC




static unsigned char _cmd_buff[CMD_SIZE];



/*
private function
*/
static void _putchars(const unsigned char * ptr, int n);
static unsigned char _verify(const void * ptr, int n);



/*******************************************************************************
* Function Name  : static void _putchars(const unsigned char * ptr, int n)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
static void _putchars(const unsigned char * ptr, int n)
{
	int i, x;

	for(i = 0; i < n; i++)
	{
	    x = ptr[i];
		Serial2.write(x);
	}
}
/*******************************************************************************
* Function Name  : static unsigned char _verify(const void * ptr, int n)
* Description    :
* Input          : ptr
                   n
* Output         : None
* Return         :
*******************************************************************************/
static unsigned char _verify(const void * ptr, int n)
{
	int i;
	unsigned char * p = (unsigned char *)ptr;
	unsigned char result;

	for(i = 0, result = 0; i < n; i++)
	{
		result ^= p[i];
	}

	return result;
}




/*******************************************************************************
* Function Name  : void epd_init(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_init(void)
{
	//pinMode(epd_on, HIGH);
	Serial2.begin(115200, SERIAL_8N1, 16, 17);
	pinMode(wake_up, HIGH);
	pinMode(reset, HIGH);
}
/*******************************************************************************
* Function Name  : void epd_reset(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_reset(void)
{
	digitalWrite(reset, LOW);
	delayMicroseconds(10);
	digitalWrite(reset, HIGH);
	delayMicroseconds(500);
	digitalWrite(reset, LOW);
	delay(3000);
}
/*******************************************************************************
* Function Name  : void epd_init(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_wakeup(void)
{
	digitalWrite(wake_up, LOW);
	delayMicroseconds(10);
	digitalWrite(wake_up, HIGH);
	delayMicroseconds(500);
	digitalWrite(wake_up, LOW);
	delay(10);
}

/*******************************************************************************
* Function Name  : void epd_handshake(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_handshake(void)
{
	memcpy(_cmd_buff, _cmd_handshake, 8);
	_cmd_buff[8] = _verify(_cmd_buff, 8);

	_putchars(_cmd_buff, 9);
}
/*******************************************************************************
* Function Name  : void epd_set_baud(long baud)
* Description    : set uart baud
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_set_baud(long baud)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0D;

	_cmd_buff[3] = CMD_SET_BAUD;

	_cmd_buff[4] = (baud >> 24) & 0xFF;
	_cmd_buff[5] = (baud >> 16) & 0xFF;
	_cmd_buff[6] = (baud >> 8) & 0xFF;
	_cmd_buff[7] = baud & 0xFF;

	_cmd_buff[8] = FRAME_E0;
	_cmd_buff[9] = FRAME_E1;
	_cmd_buff[10] = FRAME_E2;
	_cmd_buff[11] = FRAME_E3;
	_cmd_buff[12] = _verify(_cmd_buff, 12);

	_putchars(_cmd_buff, 13);

	delay(10);
	Serial2.begin(baud);
}
/*******************************************************************************
* Function Name  : void epd_read_baud(void)
* Description    : read uart baud
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_read_baud(void)
{
	memcpy(_cmd_buff, _cmd_read_baud, 8);
	_cmd_buff[8] = _verify(_cmd_buff, 8);

	_putchars(_cmd_buff, 9);
}
/*******************************************************************************
* Function Name  : void epd_set_memory(unsigned char mode)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_set_memory(unsigned char mode)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;

	_cmd_buff[3] = CMD_MEMORYMODE;

	_cmd_buff[4] = mode;

	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;
	_cmd_buff[9] = _verify(_cmd_buff, 9);

	_putchars(_cmd_buff, 10);
}

/*******************************************************************************
* Function Name  : void epd_enter_stopmode(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_enter_stopmode(void)
{
	memcpy(_cmd_buff, _cmd_stopmode, 8);
	_cmd_buff[8] = _verify(_cmd_buff, 8);

	_putchars(_cmd_buff, 9);}
/*******************************************************************************
* Function Name  : void epd_udpate(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_udpate(void)
{
	memcpy(_cmd_buff, _cmd_update, 8);
	_cmd_buff[8] = _verify(_cmd_buff, 8);

	_putchars(_cmd_buff, 9);
}
/*******************************************************************************
* Function Name  : void epd_screen_rotation(unsigned char mode)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_screen_rotation(unsigned char mode)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;

	_cmd_buff[3] = CMD_SCREEN_ROTATION;

	_cmd_buff[4] = mode;

	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;
	_cmd_buff[9] = _verify(_cmd_buff, 9);

	_putchars(_cmd_buff, 10);
}
/*******************************************************************************
* Function Name  : void epd_load_font(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_load_font(void)
{
	memcpy(_cmd_buff, _cmd_load_font, 8);
	_cmd_buff[8] = _verify(_cmd_buff, 8);

	_putchars(_cmd_buff, 9);
}
/*******************************************************************************
* Function Name  : void epd_load_pic(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_load_pic(void)
{
	memcpy(_cmd_buff, _cmd_load_pic, 8);
	_cmd_buff[8] = _verify(_cmd_buff, 8);

	_putchars(_cmd_buff, 9);
}
/*******************************************************************************
* Function Name  : void epd_set_color(unsigned char color, unsigned char bkcolor)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_set_color(unsigned char color, unsigned char bkcolor)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0B;

	_cmd_buff[3] = CMD_SET_COLOR;

	_cmd_buff[4] = color;
	_cmd_buff[5] = bkcolor;

	_cmd_buff[6] = FRAME_E0;
	_cmd_buff[7] = FRAME_E1;
	_cmd_buff[8] = FRAME_E2;
	_cmd_buff[9] = FRAME_E3;
	_cmd_buff[10] = _verify(_cmd_buff, 10);

	_putchars(_cmd_buff, 11);
}
/*******************************************************************************
* Function Name  : void epd_set_en_font(unsigned char font)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_set_en_font(unsigned char font)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;

	_cmd_buff[3] = CMD_SET_EN_FONT;

	_cmd_buff[4] = font;

	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;
	_cmd_buff[9] = _verify(_cmd_buff, 9);

	_putchars(_cmd_buff, 10);
}
/*******************************************************************************
* Function Name  : void epd_set_ch_font(unsigned char font)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_set_ch_font(unsigned char font)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0A;

	_cmd_buff[3] = CMD_SET_CH_FONT;

	_cmd_buff[4] = font;

	_cmd_buff[5] = FRAME_E0;
	_cmd_buff[6] = FRAME_E1;
	_cmd_buff[7] = FRAME_E2;
	_cmd_buff[8] = FRAME_E3;
	_cmd_buff[9] = _verify(_cmd_buff, 9);

	_putchars(_cmd_buff, 10);
}
/*******************************************************************************
* Function Name  : void epd_draw_pixel(int x0, int y0)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_draw_pixel(int x0, int y0)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0D;

	_cmd_buff[3] = CMD_DRAW_PIXEL;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	_cmd_buff[8] = FRAME_E0;
	_cmd_buff[9] = FRAME_E1;
	_cmd_buff[10] = FRAME_E2;
	_cmd_buff[11] = FRAME_E3;
	_cmd_buff[12] = _verify(_cmd_buff, 12);

	_putchars(_cmd_buff, 13);
}
/*******************************************************************************
* Function Name  : void epd_draw_line(int x0, int y0, int x1, int y1)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_draw_line(int x0, int y0, int x1, int y1)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x11;

	_cmd_buff[3] = CMD_DRAW_LINE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;

	_cmd_buff[12] = FRAME_E0;
	_cmd_buff[13] = FRAME_E1;
	_cmd_buff[14] = FRAME_E2;
	_cmd_buff[15] = FRAME_E3;
	_cmd_buff[16] = _verify(_cmd_buff, 16);

	_putchars(_cmd_buff, 17);
}
/*******************************************************************************
* Function Name  : void epd_fill_rect(int x0, int y0, int x1, int y1)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_fill_rect(int x0, int y0, int x1, int y1)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x11;

	_cmd_buff[3] = CMD_FILL_RECT;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;

	_cmd_buff[12] = FRAME_E0;
	_cmd_buff[13] = FRAME_E1;
	_cmd_buff[14] = FRAME_E2;
	_cmd_buff[15] = FRAME_E3;
	_cmd_buff[16] = _verify(_cmd_buff, 16);

	_putchars(_cmd_buff, 17);
}
/*******************************************************************************
* Function Name  : void epd_draw_circle(int x0, int y0, int r)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_draw_circle(int x0, int y0, int r)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0F;

	_cmd_buff[3] = CMD_DRAW_CIRCLE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (r >> 8) & 0xFF;
	_cmd_buff[9] = r & 0xFF;


	_cmd_buff[10] = FRAME_E0;
	_cmd_buff[11] = FRAME_E1;
	_cmd_buff[12] = FRAME_E2;
	_cmd_buff[13] = FRAME_E3;
	_cmd_buff[14] = _verify(_cmd_buff, 14);

	_putchars(_cmd_buff, 15);
}
/*******************************************************************************
* Function Name  : void epd_fill_circle(int x0, int y0, int r)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_fill_circle(int x0, int y0, int r)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x0F;

	_cmd_buff[3] = CMD_FILL_CIRCLE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (r >> 8) & 0xFF;
	_cmd_buff[9] = r & 0xFF;


	_cmd_buff[10] = FRAME_E0;
	_cmd_buff[11] = FRAME_E1;
	_cmd_buff[12] = FRAME_E2;
	_cmd_buff[13] = FRAME_E3;
	_cmd_buff[14] = _verify(_cmd_buff, 14);

	_putchars(_cmd_buff, 15);
}
/*******************************************************************************
* Function Name  : void epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x15;

	_cmd_buff[3] = CMD_DRAW_TRIANGLE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;
	_cmd_buff[12] = (x2 >> 8) & 0xFF;
	_cmd_buff[13] = x2 & 0xFF;
	_cmd_buff[14] = (y2 >> 8) & 0xFF;
	_cmd_buff[15] = y2 & 0xFF;

	_cmd_buff[16] = FRAME_E0;
	_cmd_buff[17] = FRAME_E1;
	_cmd_buff[18] = FRAME_E2;
	_cmd_buff[19] = FRAME_E3;
	_cmd_buff[20] = _verify(_cmd_buff, 20);

	_putchars(_cmd_buff, 21);
}
/*******************************************************************************
* Function Name  : void epd_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x15;

	_cmd_buff[3] = CMD_FILL_TRIANGLE;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;
	_cmd_buff[8] = (x1 >> 8) & 0xFF;
	_cmd_buff[9] = x1 & 0xFF;
	_cmd_buff[10] = (y1 >> 8) & 0xFF;
	_cmd_buff[11] = y1 & 0xFF;
	_cmd_buff[12] = (x2 >> 8) & 0xFF;
	_cmd_buff[13] = x2 & 0xFF;
	_cmd_buff[14] = (y2 >> 8) & 0xFF;
	_cmd_buff[15] = y2 & 0xFF;

	_cmd_buff[16] = FRAME_E0;
	_cmd_buff[17] = FRAME_E1;
	_cmd_buff[18] = FRAME_E2;
	_cmd_buff[19] = FRAME_E3;
	_cmd_buff[20] = _verify(_cmd_buff, 20);

	_putchars(_cmd_buff, 21);
}
/*******************************************************************************
* Function Name  : void epd_clear(void)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_clear(void)
{
	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = 0x00;
	_cmd_buff[2] = 0x09;

	_cmd_buff[3] = CMD_CLEAR;

	_cmd_buff[4] = FRAME_E0;
	_cmd_buff[5] = FRAME_E1;
	_cmd_buff[6] = FRAME_E2;
	_cmd_buff[7] = FRAME_E3;
	_cmd_buff[8] = _verify(_cmd_buff, 8);

	_putchars(_cmd_buff, 9);
}

/*******************************************************************************
* Function Name  : void epd_disp_char(unsigned char ch, int x0, int y0);
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_disp_char(unsigned char ch, int x0, int y0)
{
	unsigned char buff[2];

	buff[0] = ch;
	buff[1] = 0;

	epd_disp_string(buff, x0, y0);
}
/*******************************************************************************
* Function Name  : void epd_disp_string(const void * p, int x0, int y0)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_disp_string(const void * p, int x0, int y0)
{
	int string_size;
	unsigned char * ptr = (unsigned char *)p;


	string_size = strlen((const char *)ptr);
	string_size += 14;

	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = (string_size >> 8) & 0xFF;
	_cmd_buff[2] = string_size & 0xFF;

	_cmd_buff[3] = CMD_DRAW_STRING;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	strcpy((char *)(&_cmd_buff[8]), (const char *)ptr);

	string_size -= 5;

	_cmd_buff[string_size] = FRAME_E0;
	_cmd_buff[string_size + 1] = FRAME_E1;
	_cmd_buff[string_size + 2] = FRAME_E2;
	_cmd_buff[string_size + 3] = FRAME_E3;
	_cmd_buff[string_size + 4] = _verify(_cmd_buff, string_size + 4);

	_putchars(_cmd_buff, string_size + 5);
}
/*******************************************************************************
* Function Name  : void epd_disp_bitmap(const void * p, int x0, int y0)
* Description    :
* Input          :
* Output         : None
* Return         :
* Attention		   : None
*******************************************************************************/
void epd_disp_bitmap(const void * p, int x0, int y0)
{
	int string_size;
	unsigned char * ptr = (unsigned char *)p;

	string_size = strlen((const char *)ptr);
	string_size += 14;

	_cmd_buff[0] = FRAME_B;

	_cmd_buff[1] = (string_size >> 8) & 0xFF;
	_cmd_buff[2] = string_size & 0xFF;

	_cmd_buff[3] = CMD_DRAW_BITMAP;

	_cmd_buff[4] = (x0 >> 8) & 0xFF;
	_cmd_buff[5] = x0 & 0xFF;
	_cmd_buff[6] = (y0 >> 8) & 0xFF;
	_cmd_buff[7] = y0 & 0xFF;

	strcpy((char *)(&_cmd_buff[8]), (const char *)ptr);

	string_size -= 5;

	_cmd_buff[string_size] = FRAME_E0;
	_cmd_buff[string_size + 1] = FRAME_E1;
	_cmd_buff[string_size + 2] = FRAME_E2;
	_cmd_buff[string_size + 3] = FRAME_E3;
	_cmd_buff[string_size + 4] = _verify(_cmd_buff, string_size + 4);

	_putchars(_cmd_buff, string_size + 5);
}

void FillScreen(int nbIcon)
{
	String indexStr;
	String str;
	int x0;
	int y0;

	epd_init();
	epd_wakeup();
	epd_clear();
	epd_set_memory(MEM_TF);

	for (int i=0;i<nbIcon;i++)
	{
		indexStr = String(random(1,7));
		str = 'i' + indexStr + ".BMP ";
		char character[str.length()] ;
		str.toCharArray(character, str.length());
		x0 = ((i%8)*100);
		y0 = (i-(i%8))*120/8;
		epd_disp_bitmap(character, x0 ,y0);
	}


	epd_udpate();
}

void DrawPic(int iPic, int nbIcon){

	bool picLoaded = false;
	bool errorFlag= false;
	bool picSend= false;
	int ByteReceived;
	int index = 0;
	String indexStr;
	String str;
	int x0;
	int y0;


  epd_init();
  epd_wakeup();
	epd_clear();
  epd_set_memory(MEM_TF);

  //epd_clear();

	// for(int i = 0; i < nbIcon; i++)
	// {
	// 	index ++;
	// 	if (index == 7)
	// 	{
	// 		index = 1;
	// 	}
	// 	indexStr = String(index);
	// 	str = 'i' + indexStr + ".BMP ";
	// 	char character[str.length()] ;
	// 	str.toCharArray(character, str.length());
	// 	int x0 = ((i%8)*100);
	// 	int y0 = (i-(i%8))*120/8;
	// 	epd_disp_bitmap(character, x0 ,y0);
	// 	//LOGF4("%d print image %d at (%d,%d)", i, index, x0, y0 );
	// }

	indexStr = String(iPic);
	str = 'i' + indexStr + ".BMP ";
	char character[str.length()] ;
	str.toCharArray(character, str.length());
	x0 = ((nbIcon%8)*100);
	y0 = (nbIcon-(nbIcon%8))*120/8;
	epd_disp_bitmap(character, x0 ,y0);

	epd_udpate();
  // while(1){
  //   ByteReceived = Serial2.read();
  //   if(ByteReceived == 13){
  //     picSend= true;
  //   }
  //   if(ByteReceived == 69){
  //     errorFlag= true;
  //     if(!picSend) noSDcard();
  //     if(picSend) noPic();
  //     //resetFunc();
  //     //counter = 0;
  //     //Serial.flush();
  //     break;
  //   }
  //   if(ByteReceived == 75 && picSend && picLoaded){
  //     break;
  //   }
	//
  //   if(ByteReceived == 75 && picSend){
  //     picLoaded = true;
  //   }
	//
  // }


}

void noSDcard(){
  epd_wakeup();
  //delay(5000);
  epd_clear();
  epd_set_ch_font(GBK32);
  epd_set_en_font(ASCII32);
  epd_disp_string("Can't find SD card", 0, 300);
  epd_udpate();
  delay(10000);
}

void noPic(){
  epd_wakeup();
  //delay(5000);
  epd_clear();
  epd_set_ch_font(GBK32);
  epd_set_en_font(ASCII32);
  epd_disp_string("Can't find this picture", 0, 300);
  epd_udpate();
  delay(10000);
}
