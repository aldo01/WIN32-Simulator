#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "traf.h"
#define BYTES_TO_MAP    50

HANDLE  hFile, hMap;        // a few windows handles for shared memory access

traffic    *pMapMem;        // a pointer to the shared memory
                            // required to communicate with simulator                               

short trafopen(void)        // establishes shared memory link to simulator
{
    SECURITY_ATTRIBUTES sa;       // also required for windows
    SECURITY_DESCRIPTOR *psd;
	

          
        
    // null acl - unlimited access
    psd = (SECURITY_DESCRIPTOR *)LocalAlloc( LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(psd, TRUE, NULL, FALSE);
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = psd;
    sa.bInheritHandle = TRUE;

    // create file whose buffer area becomes shared memory
    hFile = CreateFileA(MAP_FILENAME, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);

    if( hFile == NULL )
    {
        printf("CreateFile() failed\n");     // if fail probably file error
        return -1;
    }

    // this sets up a mapping of the buffer memory as a shared area
    hMap = CreateFileMappingA( hFile, NULL, PAGE_READWRITE, 0, BYTES_TO_MAP, MAP_MAPNAME);
    if( hMap == NULL )
    {
        printf("CreateFileMapping() failed\n");
        return -1;
    }

    // get a pointer to the shared area and cast it as pointer to traffic structure
    pMapMem = (traffic *)MapViewOfFile( hMap, FILE_MAP_ALL_ACCESS, 0, 0, BYTES_TO_MAP);
    if( pMapMem == NULL )
    {
        printf("MapViewOfFile() failed\n");
        return -1;
    }
    return 1;  //success 
                	}

void trafclose(void)     // closes shared memory link to simulator
{
    CloseHandle(hMap);
    CloseHandle(hFile);
}

// sensor(s) returns 0 or 1 showing the state of sensor number s

short sensor(short s)    // provide sensor number 1 - 6
{
      short val;
      if (s < 7)         // if valid number read its state into val
      {
         val = pMapMem->sensor[s];
         if (val != 0)
         {
            pMapMem->sensor[s] = 0;    // reset the sensor so only read once
         }
         return val;      // return val (state of sensor) 0 or 1
      }
      else
         return -1;       // return -1 if error 
}   

// light(number,condition) sets the light state of light number to condition

short setlight(short n, short colour)
{
     if ((colour < 3) && (n < 7))   // check valid values for light number and colour
     {
        pMapMem->light[n] = colour;  // set the light to that 'colour' as per traf.h
        return 1;
     }
     else
        return 0;                    // returns 1 is successful or 0 for error
}

// getkey() returns the ascii value of any key press, zero if no key
// once a key value is read, it is reset zero so a key is only read once 

char getkey(void)
{
     char c;
     c = pMapMem->key;          // read the key value detected by simulator
     if (c != 0)
     {
           pMapMem->key = 0;    // reset so only 1 key per press
     }
     return c;                 // return the key value, zero if no key
}

// quit(q) can be used to command the simulator to quit using quit(1)
// or it can be used to detect if the simulator is quiting
// in this case quit(0) will return the value of the quit variable from the simulator
// 0 is normal operation, 1 is quiting

short quit(short q)
{
        if (q == 1)
        {
           pMapMem->quit = 1;
           return 1;
        }
        else
            return pMapMem->quit;
}

// reset() is used to reset the simulator to its initial condition
// may be used for testing purposes

void reset(void)
{
     int i;
     for (i=1;i<7;i++)
     {
     pMapMem->sensor[i] = 0;
     pMapMem->light[i] = 0;
     }
     pMapMem->key = 0;
     pMapMem->quit = 0;          
     pMapMem->crash = 0;          
}

// crash() returns the value of 0 for normal operation and 1 for a crash
// a crash is detected when a light combination could cause an accident
// regardless of the presence or absence of cars

short crash(void)
{
     return pMapMem->crash;
}
