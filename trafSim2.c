/* TrafSim2.c
 *
 * gcc -mwindows -e_mainCRTStartup trafsim2.c -o trafsim2.exe
 * -m emulation
 * -e start address
 * -lgdi32 may be reqd.
 * -mno-cygwin for stand-alone (uses MS libs, saves about 220K)
 *
 * Hong Zhou (adapted from trafsim1.c by Mark Phythian)
 */


#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <time.h>
#include <tchar.h>
#include "Traf.h"

#define MAP_FILENAME    "traffile"
#define MAP_MAPNAME     "mapmemory"

// ID of the timer for animated windows updates
#define ID_TIMER        1

// timeout for updates in milliseconds
#define UPDATE_TIME     50

#define PERSEC 1000/UPDATE_TIME

void traffic_tick(void);
double rnd(double m);
void reset(void);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

char szAppName[] = "TrafSim2a";

traffic *pMapMem;
HANDLE  hFile, hMap;

short sen[7] = {0,0,0,0,0,0,0}; 
short waiting[7] = {0,0,0,0,0,0,0};
short beyond[7] = {0,0,0,0,0,0,0};
short first = 0;
short persec;
short mode = 1;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int nWinMode)
{
    HWND        hwnd;
    MSG         msg;
    WNDCLASSEX  wndclass;

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR *psd;

    wndclass.cbSize        = sizeof(WNDCLASSEX);
    wndclass.style         = CS_HREDRAW | CS_VREDRAW;   // 0 = default
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInstance;
    wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);   // desktop icon 32x32
    wndclass.hIconSm       = LoadIcon(NULL, IDI_WINLOGO);       // minimize/menu icon 16x16
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);      // WHITE_BRUSH
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = TEXT("TrafSim2a");


    // null acl - unlimited access
    psd = (SECURITY_DESCRIPTOR *)LocalAlloc( LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(psd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = psd;
    sa.bInheritHandle = TRUE;

	
	

    hFile = CreateFileA(MAP_FILENAME, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

    if( hFile == NULL )
    {
        printf("CreateFile() failed\n");
        exit(1) ;
    }

    hMap = CreateFileMappingA( hFile, NULL, PAGE_READWRITE, 0, BYTES_TO_MAP, MAP_MAPNAME);
    if( hMap == NULL )
    {
        printf("CreateFileMapping() failed\n");
        exit(1);
    }

    pMapMem = (traffic *)MapViewOfFile( hMap, FILE_MAP_ALL_ACCESS, 0, 0, BYTES_TO_MAP);
    if( pMapMem == NULL )
    {
        printf("MapViewOfFile() failed\n");
        exit(1);

    }

    if( !RegisterClassEx(&wndclass) )
        return 0;

    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                          TEXT("TrafSim2a"), TEXT("Traffic Simulator -- Version 2a"),
                          WS_OVERLAPPEDWINDOW,    // normal
                          0, 0,                   // x, y may be CW_USEDEFAULT
                          600, 600,               // width, height may be CW_USEDEFAULT
                          HWND_DESKTOP,           // no parent window
                          NULL,
                          hInstance, NULL);

    ShowWindow(hwnd, nWinMode);
    UpdateWindow(hwnd);

    // set the window update time
    SetTimer(hwnd, ID_TIMER, UPDATE_TIME, NULL);

    // main message loop
    while( GetMessage(&msg, NULL, 0, 0) )
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
	
    return msg.wParam;
}

double rnd(double m)
{
	double r;

	r = m * (double)rand() / RAND_MAX;	
	return r;
}

void traffic_tick()
{
short i;

	if (first == 0)                      	
	{              					
		first = 1;
		persec = PERSEC;
        srand((unsigned)time(NULL));       // randomise random number generator
	}
	else
    {
	  persec--;
      if (persec == 0)
      {
        persec = PERSEC;

		if (mode == 0)
        {
        sen[1] = (rnd(1) < ((double)NFLOW*0.75/3600.0));  // car appears randomly at FLOW rate
		sen[2] = (rnd(1) < ((double)NFLOW*0.25/3600.0));  // car appears randomly at FLOW rate
		sen[3] = (rnd(1) < ((double)WFLOW/3600.0));  // car appears randomly at FLOW rate
		sen[4] = (rnd(1) < ((double)SFLOW*0.75/3600.0));  // car appears randomly at FLOW rate
		sen[5] = (rnd(1) < ((double)SFLOW*0.25/3600.0));  // car appears randomly at FLOW rate
		sen[6] = (rnd(1) < ((double)EFLOW/3600.0));  // car appears randomly at FLOW rate
        }
        else
        {
        for (i = 1;i<7;i++)
           sen[i] = pMapMem->sensor[i];
        }
        
        for (i=1;i<7;i++)
        {
          if ((pMapMem->light[i] == RED) || (pMapMem->light[i] == AMBER))
          {
            if (sen[i] == 1)
            {
              waiting[i]++;
              if (waiting[i] > 5)
              {
                waiting[i] = 5;
                beyond[i]++;
              }
              else 
              {
                pMapMem->sensor[i] = 1;
              }   
            }    
          }
          else //if (pMapMem->light[i] == GREEN)
          {
            waiting[i] = 0;
            if (beyond[i] > 0)
            {
              if (sen[i] == 1)
              {
                beyond[i]++;
              }
              beyond[i]--;
              if (beyond[i] == 0)
                pMapMem->sensor[i] = 0;
              else
                pMapMem->sensor[i] = 1;
            }
            else if (sen[i] == 1)
              {
                pMapMem->sensor[i] = 1;
              }   
            
          }
        }
      }
  }
			
		 
        

 					
}
LRESULT CALLBACK WndProc(HWND hwnd,
                         UINT message,
                         WPARAM wParam, LPARAM lParam)
{
    HDC             hdc;
    PAINTSTRUCT     ps;
    RECT            ClientRect;
    HBRUSH          hbrush;
    HBRUSH          hbrushw;
    HBRUSH          hbrushr;
    HBRUSH          hbrushy;
    HBRUSH          hbrushg;
    HBRUSH          hbrushp;
    HBRUSH          hbrushm;
    HPEN            hpen;
    POINT           oldPoint;
    HDC             hdcMem;
    static HBITMAP  hBitMap = (HBITMAP)0;   // must be static
    short           cxClient, cyClient;
    static int      xPosRect = 0;           // must be static
    static int      xPosBall = 50;          // must be static
    static int      xStepRect = 2;          // must be static
    static int      xStepBall = 1;          // must be static
    static int      BallWidth = 20;         // must be static
    static int      RectWidth = 50;         // must be static

    char c, cc, str[8] = {"Key = "};
    char cval;
    short lval, i, j;

    switch( message )
    {
        case WM_TIMER:
            if( ! hBitMap )
                break;

            traffic_tick();
            hdc = GetDC(hwnd);
            SetMapMode(hdc, MM_TEXT);           // pixel mode
            GetClientRect(hwnd, &ClientRect);
            hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem, hBitMap);

            // filled rectangle
            hbrushw = CreateSolidBrush(RGB(255, 255, 255));
            hbrushr = CreateSolidBrush(RGB(255, 0, 0));
            hbrushg = CreateSolidBrush(RGB(0, 255, 0));
            hbrushy = CreateSolidBrush(RGB(255, 255, 0));
            SelectObject(hdcMem, hbrushw);
            Rectangle(hdcMem, ClientRect.left, ClientRect.top,
                              ClientRect.right, ClientRect.bottom);

            //SelectObject(hdcMem, hbrush);

            // line drawing
            hpen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
            SelectObject(hdcMem, hpen);

            MoveToEx(hdcMem, 200, 50, &oldPoint);
            LineTo(hdcMem, 200, 200);
            LineTo(hdcMem, 50, 200);
            
            MoveToEx(hdcMem, 400, 50, &oldPoint);
            LineTo(hdcMem, 400, 200);
            LineTo(hdcMem, 550, 200);
            
            MoveToEx(hdcMem, 200, 500, &oldPoint);
            LineTo(hdcMem, 200, 350);
            LineTo(hdcMem, 50, 350);
            
            MoveToEx(hdcMem, 400, 500, &oldPoint);
            LineTo(hdcMem, 400, 350);
            LineTo(hdcMem, 550, 350);
            
            DeleteObject(hpen);

            hpen = CreatePen(PS_DASH, 1, RGB(0, 0, 0));
            SelectObject(hdcMem, hpen);
            
            MoveToEx(hdcMem, 266, 50, &oldPoint);
            LineTo(hdcMem, 266, 200);
            
            MoveToEx(hdcMem, 333, 50, &oldPoint);
            LineTo(hdcMem, 333, 200);
            
             MoveToEx(hdcMem, 266, 350, &oldPoint);
            LineTo(hdcMem, 266, 500);
            
            MoveToEx(hdcMem, 333, 350, &oldPoint);
            LineTo(hdcMem, 333, 500);
            
            MoveToEx(hdcMem, 50, 275, &oldPoint);
            LineTo(hdcMem, 200, 275);
            
            MoveToEx(hdcMem, 550, 275, &oldPoint);
            LineTo(hdcMem, 400, 275);

            DeleteObject(hpen);

            hpen = CreatePen(PS_DASH, 5, RGB(0, 0, 0));
            SelectObject(hdcMem, hpen);
            
            MoveToEx(hdcMem, 276, 195, &oldPoint);
            LineTo(hdcMem, 323, 195);

            MoveToEx(hdcMem, 343, 195, &oldPoint);
            LineTo(hdcMem, 390, 195);
            
            MoveToEx(hdcMem, 276, 355, &oldPoint);
            LineTo(hdcMem, 323, 355);

            MoveToEx(hdcMem, 210, 355, &oldPoint);
            LineTo(hdcMem, 256, 355);

            MoveToEx(hdcMem, 195, 210, &oldPoint);
            LineTo(hdcMem, 195, 265);

            MoveToEx(hdcMem, 405, 285, &oldPoint);
            LineTo(hdcMem, 405, 340);
            
            DeleteObject(hpen);

            //hbrushb = CreateSolidBrush(RGB(255, 0, 255));
            //SelectObject(hdcMem, hbrushb);
            hpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(hdcMem, hpen);

            lval = pMapMem->light[1];
            if (lval == GREEN)
            {
              SelectObject(hdcMem, hbrushg);
              Ellipse(hdcMem, 340, 210, 350, 220);    //G L1 
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 340, 220, 350, 230);    //Y   
              Ellipse(hdcMem, 340, 230, 350, 240);    //R     
            }
            else if (lval == AMBER)
            {
              SelectObject(hdcMem, hbrushy);
              Ellipse(hdcMem, 340, 220, 350, 230);    //Y   
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 340, 210, 350, 220);    //G L1 
              Ellipse(hdcMem, 340, 230, 350, 240);    //R     
            }
            else if (lval == RED)
            {
              SelectObject(hdcMem, hbrushr);
              Ellipse(hdcMem, 340, 230, 350, 240);    //R     
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 340, 210, 350, 220);    //G L1 
              Ellipse(hdcMem, 340, 220, 350, 230);    //Y   
            }
            
            if (pMapMem->light[2] == GREEN)
            {
              SelectObject(hdcMem, hbrushg);
              Ellipse(hdcMem, 320, 210, 330, 220);    //G L2
            }
            else
            {
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 320, 210, 330, 220);    //G L2
            }
            
            lval = pMapMem->light[3];
            if (lval == GREEN)
            {
              SelectObject(hdcMem, hbrushg);
              Ellipse(hdcMem, 150, 180, 160, 190);    //G L3
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 160, 180, 170, 190);    //Y
              Ellipse(hdcMem, 170, 180, 180, 190);    //R
            }
            else if (lval == AMBER)
            {
              SelectObject(hdcMem, hbrushy);
              Ellipse(hdcMem, 160, 180, 170, 190);    //Y
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 150, 180, 160, 190);    //G L3
              Ellipse(hdcMem, 170, 180, 180, 190);    //R
            }
            else if (lval == RED)
            {
              SelectObject(hdcMem, hbrushr);
              Ellipse(hdcMem, 170, 180, 180, 190);    //R
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 150, 180, 160, 190);    //G L3
              Ellipse(hdcMem, 160, 180, 170, 190);    //Y
              }
            
            lval = pMapMem->light[4];
            if (lval == GREEN)
            {
              SelectObject(hdcMem, hbrushg);
              Ellipse(hdcMem, 250, 330, 260, 340);    //G L4
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 250, 320, 260, 330);    //Y 
              Ellipse(hdcMem, 250, 310, 260, 320);    //R 
            }
            else if (lval == AMBER)
            {
              SelectObject(hdcMem, hbrushy);
              Ellipse(hdcMem, 250, 320, 260, 330);    //Y 
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 250, 330, 260, 340);    //G L4
              Ellipse(hdcMem, 250, 310, 260, 320);    //R 
            }
            else if (lval == RED)
            {
              SelectObject(hdcMem, hbrushr);
              Ellipse(hdcMem, 250, 310, 260, 320);    //R 
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 250, 330, 260, 340);    //G L4
              Ellipse(hdcMem, 250, 320, 260, 330);    //Y 
            }
             
            if (pMapMem->light[5] == GREEN)
            {
              SelectObject(hdcMem, hbrushg);
              Ellipse(hdcMem, 270, 330, 280, 340);    //G L5
            }
            else
            {
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 270, 330, 280, 340);    //G L5
            }
            
            lval = pMapMem->light[6];
            if (lval == GREEN)
            {
              SelectObject(hdcMem, hbrushg);
              Ellipse(hdcMem, 430, 360, 440, 370);    //G L6
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 420, 360, 430, 370);    //Y
              Ellipse(hdcMem, 410, 360, 420, 370);    //R
            }
            else if (lval == AMBER)
            {
              SelectObject(hdcMem, hbrushy);
              Ellipse(hdcMem, 420, 360, 430, 370);    //Y
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 430, 360, 440, 370);    //G L6
              Ellipse(hdcMem, 410, 360, 420, 370);    //R
            }
            else if (lval == RED)
            {
              SelectObject(hdcMem, hbrushr);
              Ellipse(hdcMem, 410, 360, 420, 370);    //R
              SelectObject(hdcMem, hbrushw);
              Ellipse(hdcMem, 430, 360, 440, 370);    //G L6
              Ellipse(hdcMem, 420, 360, 430, 370);    //Y
            }
            
            DeleteObject(hpen);
            
            hbrushp = CreateSolidBrush(RGB(255, 0, 255));
            SelectObject(hdcMem, hbrushp);
            hpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(hdcMem, hpen);
            if (pMapMem->sensor[1] == 1)
              SelectObject(hdcMem, hbrushp);
            else  
              SelectObject(hdcMem, hbrushw);
            Rectangle(hdcMem, 350, 30, 390, 40);     //S1
            if (pMapMem->sensor[2] == 1)
              SelectObject(hdcMem, hbrushp);
            else  
              SelectObject(hdcMem, hbrushw);
            Rectangle(hdcMem, 280, 30, 320, 40);     //S2
            if (pMapMem->sensor[3] == 1)
              SelectObject(hdcMem, hbrushp);
            else  
              SelectObject(hdcMem, hbrushw);
            Rectangle(hdcMem, 30, 215, 40, 260);     //S3
            if (pMapMem->sensor[4] == 1)
              SelectObject(hdcMem, hbrushp);
            else  
              SelectObject(hdcMem, hbrushw);
            Rectangle(hdcMem, 210,510, 250,520);     //S4
            if (pMapMem->sensor[5] == 1)
              SelectObject(hdcMem, hbrushp);
            else  
              SelectObject(hdcMem, hbrushw);
            Rectangle(hdcMem, 280,510, 320,520);     //S5
            if (pMapMem->sensor[6] == 1)
              SelectObject(hdcMem, hbrushp);
            else  
              SelectObject(hdcMem, hbrushw);
            Rectangle(hdcMem, 560,290, 570,330);     //S6
            
            TextOutA(hdcMem, 160, 5,"North   2    1", ARRAYSIZE("North   2    1")-1); 
            TextOutA(hdcMem, 10, 170,"West  3", ARRAYSIZE("West  3")-1); 
            TextOutA(hdcMem, 225, 525,"4    5   South", ARRAYSIZE("4    5   South")-1); 
            TextOutA(hdcMem, 460, 360,"6  East",  ARRAYSIZE("6  East")-1); 

            
					str[6] =c;
            TextOutA(hdcMem, 10, 465, str, 7); 

            TextOutA(hdcMem, 10, 365,"(M) Mode =",ARRAYSIZE("(M) Mode =")-1); 
            if (mode == 1)
              TextOutA(hdcMem, 10, 385, "MANUAL", ARRAYSIZE("MANUAL")-1); 
            else
              TextOutA(hdcMem, 10, 385, "RANDOM", ARRAYSIZE("RANDOM")-1); 
            
            //TextOutA(hdcMem, 10, 485, (LPCTSTR)"(M) Mode", 8); 
            TextOutA(hdcMem, 10, 505, "(R) to Reset", ARRAYSIZE("(R) to Reset")-1); 
            TextOutA(hdcMem, 10, 525, "(Q) to Quit", ARRAYSIZE("(Q) to Quit")-1); 
            
            DeleteObject(hpen);
            
            hpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(hdcMem, hpen);
            SelectObject(hdcMem, hbrush);
            if (((pMapMem->light[1] == GREEN) || (pMapMem->light[2] == GREEN) || (pMapMem->light[4] == GREEN)
             || (pMapMem->light[5] == GREEN)) && ((pMapMem->light[3] == GREEN) || (pMapMem->light[6] == GREEN)))
            {
              TextOutA(hdcMem, 250, 265, "CRASH", 10);   // when N or S 'and' E or W are GO a crash will occur
	        }
                     
            DeleteObject(hpen);
            
            hpen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
            SelectObject(hdcMem, hpen);
            hbrushm = CreateSolidBrush(RGB(128, 128, 128));
            SelectObject(hdcMem, hbrushp);
              for (j = 1; j <= waiting[1]; j++)
              {
                Rectangle(hdcMem, 355,218-30*j, 380,193-30*j);     //S1
              }
              for (j = 1; j <= waiting[2]; j++)
              {
                Rectangle(hdcMem, 285,218-30*j, 310,193-30*j);     //S2
              }
              for (j = 1; j <= waiting[3]; j++)
              {
                Rectangle(hdcMem, 195-30*j,220,220-30*j, 245);     //S3
              }
              for (j = 1; j <= waiting[4]; j++)
              {
                Rectangle(hdcMem, 220,330+30*j, 245,355+30*j);     //S4
              }
              for (j = 1; j <= waiting[5]; j++)
              {
                Rectangle(hdcMem, 285,330+30*j, 310,355+30*j);     //S5
              }
              for (j = 1; j <= waiting[6]; j++)
              {
                Rectangle(hdcMem,380+30*j, 300,405+30*j, 325);     //S4
              }

            // copy from memory bitmap to window bitmap
            BitBlt(hdc, ClientRect.left, ClientRect.top,
                    ClientRect.right - ClientRect.left,
                    ClientRect.bottom - ClientRect.top,
                    hdcMem,
                    ClientRect.left, ClientRect.top,
                    SRCCOPY);

            DeleteObject(hbrush);
            DeleteObject(hbrushr);
            DeleteObject(hbrushy);
            DeleteObject(hbrushg);
            DeleteObject(hbrushw);
            DeleteObject(hbrushp);

            ReleaseDC(hwnd, hdc);
            DeleteDC(hdcMem);
            return 0;

        case WM_CREATE:
            hdc = GetDC(hwnd);
            GetClientRect(hwnd, &ClientRect);
            ReleaseDC(hwnd, hdc);
            reset();
            return 0;

        case WM_SIZE:
            hdc = GetDC(hwnd);
            cxClient = LOWORD(lParam);
            cyClient = HIWORD(lParam);
            GetClientRect(hwnd, &ClientRect);

            hdcMem = CreateCompatibleDC(hdc);

            // if bitmap exists (old size), delete it
            if( hBitMap )
                DeleteObject(hBitMap);
            hBitMap = CreateCompatibleBitmap(hdc,
                            ClientRect.right, ClientRect.bottom);
            ReleaseDC(hwnd, hdc);
            return 0;      

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;

        case WM_DESTROY:
            // if bitmap exists, delete it
            CloseHandle(hMap);
            CloseHandle(hFile);
            if( hBitMap )
                DeleteObject(hBitMap);

            KillTimer(hwnd, ID_TIMER);
            PostQuitMessage(0);
            return 0;

        case WM_CHAR:
		
            c = (char)LOWORD(wParam);
            cc = toupper(c);
			
            cval = cc - '0';
            if ((cval > 0) && (cval < 7))
            {
              pMapMem->sensor[cval] ^= 1;
            }
            else if ((cc > '@') && (cc < 'G'))
            {
              pMapMem->light[cc - '@'] = pMapMem->light[cc - '@'] ^ GREEN;
            }
            else if (cc == 'M')
            {
              mode ^= 1;        //toggle mode
              for (i=1;i<7;i++)
                pMapMem->sensor[i] = 0;
            }
            else if (cc == 'Q')
            {
              pMapMem->key = c;
              pMapMem->quit = 1;
              SendMessage(hwnd,WM_DESTROY,0,0);
            }
            else if (cc == 'R')
            {
              reset();
			  
			  
            }
            else
            {
              pMapMem->key = c;
            }
            return 0;
            
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
		
    return 0;
}
#if 1
void reset(void)
{
      short i;
      for (i=1;i<7;i++)
     {
     pMapMem->sensor[i] = 0;
     pMapMem->light[i] = RED;
     waiting[i] = 0;
     beyond[i] = 0;
     sen[i] = 0;
     }
     pMapMem->key = 0;
     pMapMem->quit = 0;          
     pMapMem->crash = 0;
     first = 0;          
}
#endif
