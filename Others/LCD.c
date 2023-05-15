/**************************************************************
The initial and control for 320×240 16Bpp TFT LCD--LTS350Q1_PE1 & LQ035Q7DB02
**************************************************************/

#include <string.h>
#include "2410addr.h"
#include "2410lib.h"
#include "def.h"
#include "2410slib.h"

#include "LCD.h"
//extern unsigned char xyx_240_320[];	//宽240，高320

volatile unsigned short LCD_BUFER[SCR_YSIZE_TFT_240320][SCR_XSIZE_TFT_240320];

/**************************************************************
320×240 16Bpp TFT LCD数据和控制端口初始化
**************************************************************/
void Lcd_Port_Init(void)
{
    rGPCUP=0xffffffff; // Disable Pull-up register
    rGPCCON=0xaaaa56a9; //Initialize VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 

    rGPDUP=0xffffffff; // Disable Pull-up register
    rGPDCON=0xaaaaaaaa; //Initialize VD[15:8]
}

/**************************************************************
320×240 16Bpp TFT LCD功能模块初始化
**************************************************************/
void Lcd_Init(void)
{
	rLCDCON1=(CLKVAL_TFT_240320<<8)|(MVAL_USED<<7)|(3<<5)|(12<<1)|0;
    	// TFT LCD panel,12bpp TFT,ENVID=off
	rLCDCON2=(VBPD_240320<<24)|(LINEVAL_TFT_240320<<14)|(VFPD_240320<<6)|(VSPW_240320);
	rLCDCON3=(HBPD_240320<<19)|(HOZVAL_TFT_240320<<8)|(HFPD_240320);
	rLCDCON4=(MVAL<<8)|(HSPW_240320);
	rLCDCON5=(1<<11)|(0<<9)|(0<<8)|(0<<6)|(BSWP<<1)|(HWSWP);	//FRM5:6:5,HSYNC and VSYNC are inverted
	rLCDSADDR1=(((U32)LCD_BUFER>>22)<<21)|M5D((U32)LCD_BUFER>>1);
	rLCDSADDR2=M5D( ((U32)LCD_BUFER+(SCR_XSIZE_TFT_240320*LCD_YSIZE_TFT_240320*2))>>1 );
	rLCDSADDR3=(((SCR_XSIZE_TFT_240320-LCD_XSIZE_TFT_240320)/1)<<11)|(LCD_XSIZE_TFT_240320/1);
	rLCDINTMSK|=(3); // MASK LCD Sub Interrupt
	rLPCSEL&=(~7); // Disable LPC3600
	rTPAL=0; // Disable Temp Palette
}

/**************************************************************
LCD视频和控制信号输出或者停止，1开启视频输出
**************************************************************/
void Lcd_EnvidOnOff(int onoff)
{
    if(onoff==1)
	rLCDCON1|=1; // ENVID=ON
    else
	rLCDCON1 =rLCDCON1 & 0x3fffe; // ENVID Off
}

/**************************************************************
LPC3600 is a timing control logic unit for LTS350Q1-PD1 or LTS350Q1-PD2
**************************************************************/
void Lcd_Lpc3600Enable(void)
{
    rLPCSEL&=~(7);
    rLPCSEL|=(7); // 240320,Enable LPC3600
}    

/**************************************************************
320×240 8Bpp TFT LCD 电源控制引脚使能
**************************************************************/
void Lcd_PowerEnable(int invpwren,int pwren)
{
    //GPG4 is setted as LCD_PWREN
    rGPGUP = rGPGUP|(1<<4); // Pull-up disable
    rGPGCON = rGPGCON|(3<<8); //GPG4=LCD_PWREN
    //Enable LCD POWER ENABLE Function
    rLCDCON5 = rLCDCON5&(~(1<<3))|(pwren<<3);   // PWREN
    rLCDCON5=rLCDCON5&(~(1<<5))|(invpwren<<5);   // INVPWREN
}

/**************************************************************
320×240 8Bpp TFT LCD 颜色初始化
**************************************************************/
void Lcd_Palette_Init(void)
{
    unsigned char cdata, p_red, p_green, p_blue;
    U32 *palette;
    
	//#define PALETTE     0x4d000400    //Palette start address
    palette=(U32 *)PALETTE;
    *palette++=0; //black
    for(cdata=1;cdata<255;cdata++)
    {
		p_red=(cdata & 0xe0);
		p_green=(cdata & 0x1c);
		p_blue=(cdata & 0x03);
    	*palette++=((U32)((p_red<<8)|(p_green<<6)|(p_blue<<3)));
    }
    *palette=0xffff; //white
}

/**************************************************************
320×240 16Bpp TFT LCD移动观察窗口
**************************************************************/
void Lcd_MoveViewPort(int vx,int vy)
{
    U32 addr;

    SET_IF(); 
	#if (LCD_XSIZE_TFT_240320<32)
    	    while((rLCDCON1>>18)<=1); // if x<32
	#else	
    	    while((rLCDCON1>>18)==0); // if x>32
	#endif
    
    addr=(U32)LCD_BUFER+(vx*2)+vy*(SCR_XSIZE_TFT_240320*2);
	rLCDSADDR1= ( (addr>>22)<<21 ) | M5D(addr>>1);
	rLCDSADDR2= M5D(((addr+(SCR_XSIZE_TFT_240320*LCD_YSIZE_TFT_240320*2))>>1));
	CLR_IF();
}    

/**************************************************************
320×240 16Bpp TFT LCD移动观察窗口
**************************************************************/
void MoveViewPort(void)
{
    int vx=0,vy=0,vd=1;

    Uart_Printf("\n*Move the LCD view windos:\n");
    Uart_Printf(" press 8 is up\n");
    Uart_Printf(" press 2 is down\n");
    Uart_Printf(" press 4 is left\n");
    Uart_Printf(" press 6 is right\n");
    Uart_Printf(" press Enter to exit!\n");

    while(1)
    {
    	switch(Uart_Getch())
    	{
    	case '8':
	    if(vy>=vd)vy-=vd;    	   	
        break;

    	case '4':
    	    if(vx>=vd)vx-=vd;
    	break;

    	case '6':
                if(vx<=(SCR_XSIZE_TFT_240320-LCD_XSIZE_TFT_240320-vd))vx+=vd;   	    
   	    break;

    	case '2':
                if(vy<=(SCR_YSIZE_TFT_240320-LCD_YSIZE_TFT_240320-vd))vy+=vd;   	    
   	    break;

    	case '\r':
   	    return;

    	default:
	    break;
		}
	Uart_Printf("vx=%3d,vy=%3d\n",vx,vy);
	Lcd_MoveViewPort(vx,vy);
    }
}

/**************************************************************
320×240 16Bpp TFT LCD单个象素的显示数据输出
**************************************************************/
void PutPixel(U32 x,U32 y,U32 c)
{
	if ( (x < SCR_XSIZE_TFT_240320) && (y < SCR_YSIZE_TFT_240320) )
	LCD_BUFER[(y)][(x)] = c;
}

/**************************************************************
320×240 16Bpp TFT LCD全屏填充特定颜色单元或清屏
**************************************************************/
void Lcd_ClearScr(U16 c)
{
	unsigned int x,y ;
		
    for( y = 0 ; y < SCR_YSIZE_TFT_240320 ; y++ )
    {
    	for( x = 0 ; x < SCR_XSIZE_TFT_240320 ; x++ )
    	{
			LCD_BUFER[y][x] = c;
    	}
    }
}

/**************************************************************
LCD屏幕显示垂直翻转
// LCD display is flipped vertically
// But, think the algorithm by mathematics point.
//   3I2
//   4 I 1
//  --+--   <-8 octants  mathematical cordinate
//   5 I 8
//   6I7
**************************************************************/
void Glib_Line(int x1,int y1,int x2,int y2,int color)
{
	int dx,dy,e;
	dx=x2-x1; 
	dy=y2-y1;
    
	if(dx>=0)
	{
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 1/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 2/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 8/8 octant
			{
				e=dy-dx/2;
				while(x1<=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1+=1;
					e+=dy;
				}
			}
			else		// 7/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1+=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
	else //dx<0
	{
		dx=-dx;		//dx=abs(dx)
		if(dy >= 0) // dy>=0
		{
			if(dx>=dy) // 4/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1+=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 3/8 octant
			{
				e=dx-dy/2;
				while(y1<=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1+=1;
					e+=dx;
				}
			}
		}
		else		   // dy<0
		{
			dy=-dy;   // dy=abs(dy)

			if(dx>=dy) // 5/8 octant
			{
				e=dy-dx/2;
				while(x1>=x2)
				{
					PutPixel(x1,y1,color);
					if(e>0){y1-=1;e-=dx;}	
					x1-=1;
					e+=dy;
				}
			}
			else		// 6/8 octant
			{
				e=dx-dy/2;
				while(y1>=y2)
				{
					PutPixel(x1,y1,color);
					if(e>0){x1-=1;e-=dy;}	
					y1-=1;
					e+=dx;
				}
			}
		}	
	}
}

/**************************************************************
在LCD屏幕上画一个矩形
**************************************************************/
void Glib_Rectangle(int x1,int y1,int x2,int y2,int color)
{
    Glib_Line(x1,y1,x2,y1,color);
    Glib_Line(x2,y1,x2,y2,color);
    Glib_Line(x1,y2,x2,y2,color);
    Glib_Line(x1,y1,x1,y2,color);
}

/**************************************************************
在LCD屏幕上用颜色填充一个矩形
**************************************************************/
void Glib_FilledRectangle(int x1,int y1,int x2,int y2,int color)
{
    int i;

    for(i=y1;i<=y2;i++)
	Glib_Line(x1,i,x2,i,color);
}

/**************************************************************
在LCD屏幕上指定坐标点画一个指定大小的图片
**************************************************************/
void Paint_Bmp(int x0,int y0,int h,int l,unsigned char bmp[])
{
	int x,y;
	U32 c;
	int p = 0;
	
    for( y = 0 ; y < l ; y++ )
    {
    	for( x = 0 ; x < h ; x++ )
    	{
    		c = bmp[p+1] | (bmp[p]<<8) ;

			if ( ( (x0+x) < SCR_XSIZE_TFT_240320) && ( (y0+y) < SCR_YSIZE_TFT_240320) )
			LCD_BUFER[y0+y][x0+x] = c ;

    		p = p + 2 ;
    	}
    }
}

/**************************************************************
**************************************************************/
void Test_Lcd_Tft_16Bpp_240_320(void)
{
	//unsigned int x,y ;
	//U32 c;
	
	#define DELAY_TIME		36000

	#ifdef DEBUG
    	Uart_Printf("\nTest LTS350Q1_PE1 & LQ035Q7DB02 (TFT LCD)!\n");
	#endif	

    Lcd_Port_Init();
    Lcd_Init();
    Lcd_EnvidOnOff(1);		//turn on vedio

	Lcd_ClearScr(0xffff);		//fill all screen with some color
	
	Glib_FilledRectangle( 12, 12, 228, 308,0x0000);		//fill a Rectangle with some color

	Glib_FilledRectangle( 24, 24, 88,296,0x001f);		//fill a Rectangle with some color
	Glib_FilledRectangle( 89, 24,152,296,0x07e0);		//fill a Rectangle with some color
	Glib_FilledRectangle(153, 24,216,296,0xf800);		//fill a Rectangle with some color

	Glib_FilledRectangle( 70, 110, 170, 210,0xf7e0);		//fill a Rectangle with some color
	Glib_FilledRectangle( 90, 130, 150, 190,0x07ff);		//fill a Rectangle with some color
	Glib_FilledRectangle(110, 150, 130, 170,0xf81f);		//fill a Rectangle with some color

	#ifdef DEBUG
	#define Uart_Printf printf
    	Uart_Printf( "\nrGPBCON=0x%x\n", rGPBCON );
    	Uart_Printf( "\trGPBUP=0x%x\n", rGPBUP );
    	Uart_Printf( "rGPCCON=0x%x\n", rGPCCON );
    	Uart_Printf( "\trGPCUP=0x%x\n", rGPCUP );
    	Uart_Printf( "rGPDCON=0x%x\n", rGPDCON );
    	Uart_Printf( "\trGPDUP=0x%x\n", rGPDUP );
    	Uart_Printf( "rGPGCON=0x%x\n", rGPGCON );
    	Uart_Printf( "\trGPGUP=0x%x\n\n", rGPGUP );

    	Uart_Printf( "rLCDCON1=0x%x\n", rLCDCON1 );
    	Uart_Printf( "rLCDCON2=0x%x\n", rLCDCON2 );
    	Uart_Printf( "rLCDCON3=0x%x\n", rLCDCON3 );
    	Uart_Printf( "rLCDCON4=0x%x\n", rLCDCON4 );
    	Uart_Printf( "rLCDCON5=0x%x\n\n", rLCDCON5 );

    	Uart_Printf( "rLCDSADDR1=0x%x\n", rLCDSADDR1 );
    	Uart_Printf( "rLCDSADDR2=0x%x\n", rLCDSADDR2 );
    	Uart_Printf( "rLCDSADDR3=0x%x\n\n", rLCDSADDR3 );
    	
    	Uart_Printf( "rLCDINTMSK=0x%x\n", rLCDINTMSK );
    	Uart_Printf( "rLPCSEL=0x%x\n", rLPCSEL );
    	Uart_Printf( "rTPAL=0x%x\n\n", rTPAL );
	#endif	

    //Paint_Bmp( 0, 0, 240, 320, xyx_240_320 );		//paint a bmp
}
//*************************************************************
//Upadated 220515
