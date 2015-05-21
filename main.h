
#include <stdio.h>
#include <Windows.h>
#include "traf.h"



 


HANDLE hPipe;

 



// lamps
// 0 - red (min count 2), 1 - yellow 1 (min count 2), 2 - green (min count 8), 3 - yellow 2 (min count 2)
int lamps[7];
// indicates the timer for the specified lamp (between 1-6)
int lampsCount[7];
// indicate number of vehicles at lights
int vehicles[7];
// indicate number of times at lights
int times[7];

// store state of sensors
int sensors[7];

// indicates traffic phase
// 1 - north/south, 2 - east/west, 3 - nturn, 4 - sturn 
int phase = 0;

// try to set the specified lamp
int tryset(int l, int c) {
  int change = 1;

  if (lampsCount[l] == 0) {
    switch (lamps[l]) {
    case 0 : {
      if (c != 0) {
        lamps[l] = 1;
        lampsCount[l] = 2;
      }
      break;
    }
    case 1 : {
      lamps[l] = 2;
      lampsCount[l] = 8;
      break;
    }
    case 2 : {
      if (c != 2) {
        lamps[l] = 3;
        lampsCount[l] = 2;
      }
      break;
    }
    case 3 : {
      lamps[l] = 0;
      lampsCount[l] = 2;
      break;
    }
    }
  }

  if (lamps[l] != c) {
    change = 0;
  } else if (lamps[l] == 0) {
    if (lampsCount[l] != 0) {
      change = 0;
    }
  }


  return (change);
}

// state driven control
void statemachine() {
  int change = 1;

  switch (phase) {
  case 0 : {
    tryset(NORTH, RED);
    tryset(NTURN, RED);
    tryset(WEST, RED);
    tryset(SOUTH, RED);
    tryset(STURN, RED);
    tryset(EAST, RED);
    break;
  }
  case 1 : {
    change = tryset(NTURN, RED) && change;
    change = tryset(WEST, RED) && change;
    change = tryset(STURN, RED) && change;
    change = tryset(EAST, RED) && change;

    if (change == 1) {
      change = tryset(NORTH, GREEN) && change;
      change = tryset(SOUTH, GREEN) && change;

      if (change == 1) {
        vehicles[NORTH] = 0;
        times[NORTH] = 0;
        vehicles[SOUTH] = 0;
        times[SOUTH] = 0;
      }
    }
    break;
  }
  case 2 : {
    change = tryset(NTURN, RED) && change;
    change = tryset(NORTH, RED) && change;
    change = tryset(STURN, RED) && change;
    change = tryset(SOUTH, RED) && change;

    if (change == 1) {
      change = tryset(EAST, GREEN) && change;
      change = tryset(WEST, GREEN) && change;

      if (change == 1) {
        vehicles[EAST] = 0;
        times[EAST] = 0;
        vehicles[WEST] = 0;
        times[WEST] = 0;
      }
    }
    break;
  }
  case 3 : {
    change = tryset(EAST, RED) && change;
    change = tryset(WEST, RED) && change;
    change = tryset(STURN, RED) && change;
    change = tryset(SOUTH, RED) && change;

    if (change == 1) {
      change = tryset(NORTH, GREEN) && change;
      change = tryset(NTURN, GREEN) && change;

      if (change == 1) {
        vehicles[NORTH] = 0;
        times[NORTH] = 0;
        vehicles[NTURN] = 0;
        times[NTURN] = 0;
      }
    }
    break;
  }
  case 4 : {
    change = tryset(EAST, RED) && change;
    change = tryset(WEST, RED) && change;
    change = tryset(NORTH, RED) && change;
    change = tryset(NTURN, RED) && change;

    if (change == 1) {
      change = tryset(SOUTH, GREEN) && change;
      change = tryset(STURN, GREEN) && change;

      if (change == 1) {
        vehicles[SOUTH] = 0;
        times[SOUTH] = 0;
        vehicles[STURN] = 0;
        times[STURN] = 0;
      }
    }
    break;
  }
  }
}
 
int main() {
  int i;
  int vehicles_sum;
  int times_sum;
  int max, max1, max2, max3, max4;
  char	DataBuffer[BUFFER_SIZE];
  DWORD cbBytes;
  char s1[50],s2[50],s3[50],s4[50],s5[50];
  BOOL DisplayProcConnect ;
  int pro_run=0;
  BOOL Out_val;
  
  STARTUPINFO si,ti;
  PROCESS_INFORMATION pi,di;

	// Setting for CreateProcess
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	// Setting for CreateProcess
	ZeroMemory( &ti, sizeof(ti) );
    ti.cb = sizeof(ti);
    ZeroMemory( &di, sizeof(di) );



	


   
    
	//Simulator Memory Open
 if(trafopen() == -1) {
    printf("shared memory error\n");
    return -1;
  }



			 // Start the child processes & named pipe 

	  if( !CreateProcessA( ".\\traffsim.exe",   // No module name (use command line)
        NULL,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    		) 
   		 {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return;
    	} 
		else
			{
			printf("Starting The Traffic Simulation Process\n");
		}

		
		hPipe = CreateNamedPipeA( 
				 g_szPipeName,			   // pipe name 
				 PIPE_ACCESS_DUPLEX,	   // read/write access 
				 PIPE_TYPE_MESSAGE |	   // message type pipe 
				 PIPE_READMODE_MESSAGE |   // message-read mode 
				 PIPE_WAIT, 			   // blocking mode 
				 PIPE_UNLIMITED_INSTANCES, // max. instances  
				 BUFFER_SIZE,			   // output buffer size 
				 BUFFER_SIZE,			   // input buffer size 
				 NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
				 NULL); 				   // default security attribute 
			
			if (INVALID_HANDLE_VALUE == hPipe) 
			{
				 printf("\nError occurred while " 
						"creating the pipe: %d", GetLastError()); 
				 return 1;	//Error
			}
			else
			{
				 printf("\nCreateNamedPipe()For communication with Display Process");
			}
			
			printf("\nWaiting for Display Process To Start");



			
		if( !CreateProcessA( ".\\Display.exe",  // No module name (use command line)
		        NULL,        // Command line
		        NULL,           // Process handle not inheritable
		        NULL,           // Thread handle not inheritable
		        FALSE,          // Set handle inheritance to FALSE
		        CREATE_NEW_CONSOLE,              // No creation flags
		        NULL,           // Use parent's environment block
		        NULL,           // Use parent's starting directory 
		        &ti,            // Pointer to STARTUPINFO structure
        		&di )           // Pointer to PROCESS_INFORMATION structure
    	) 
	  {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return;
    } 
	
		//Wait for the Display process
		DisplayProcConnect = ConnectNamedPipe(hPipe, NULL);
		  
		  if (FALSE == DisplayProcConnect)
		  {
			   printf("\nError occurred while connecting" 
					  " to the client: %d", GetLastError()); 
			   CloseHandle(hPipe);
			   return 1;  //Error
		  }
		  else
		  {
			   printf("\nCommunication Channel Setup Succesful with Display Process\n");
		  }
		
   				
	   
    			// End Of Invoking of Child processes 


		

 
		
  // initialize lamp arrays
  for (i = 1; i <= 6; i++) {
    lamps[i] = 0; // the lamp initial light is red
    lampsCount[i] = 2; // for two seconds
  }

  // initialize vehicles and times arrays
  for (i = 1; i <= 6; i++) {
    vehicles[i] = 0;
    times[i] = 0;
  }
  

  // loop until get quit message
  while (quit(0) == 0) {
 Sleep(1000);
	s1[0]='\0';
	s2[0]='\0';
	s3[0]='\0';
	s4[0]='\0';
	s5[0]='\0';
	DataBuffer[0]='\0';
    // read sensor states and calculate waiting vehicles
    vehicles_sum = 0;
    for (i = 1; i <= 6; i++) {
      sensors[i] = sensor(i);

      // increment number of vehicles
      if (sensors[i] == 1) {
        vehicles[i]++;
      }
 
      vehicles_sum = vehicles_sum + vehicles[i];
    }

    // calculate witing times
    times_sum = 0;
    for (i = 1; i <= 6; i++) {
      times[i] = times[i] + vehicles[i];
      times_sum = times_sum + times[i];
    }

    // decide what is the next phase
    phase = 0;
    max = 0;

    max1 = times[NORTH] + times[SOUTH];
    max2 = times[EAST] + times[WEST];
    max3 = times[NORTH] + times[NTURN];
    max4 = times[SOUTH] + times[STURN];

    if (max1 > max) {
      max = max1;
      phase = 1;
    }
    if (max2 > max) {
      max = max2;
      phase = 2;
    }
    if (max3 > max) {
      max = max3;
      phase = 3;
    }
    if (max4 > max) {
      max = max4;
      phase = 4;
    }


	sprintf(s5,"%d ", phase);
	
	
	strcat(DataBuffer,s5);
	
	sprintf(s1,"total vehicles: %d / ", vehicles_sum);
	
	strcat(DataBuffer,s1);

	
    for (i = 1; i <= 6; i++) {
		sprintf(s3,"%d ",vehicles[i]);
      strcat(s2,s3);
    }

	strcat(DataBuffer,s2);

	
    sprintf(s4,"times: %d - %d %d %d %d\n", times_sum, max1, max2, max3, max4);
    strcat(DataBuffer,s4);
 
		
		
	
				  	  Out_val = WriteFile( 
			          hPipe,                // handle to pipe 
						  DataBuffer  ,//buffer to write from 
			        strlen(DataBuffer)+1,  // number of bytes to write, include the NULL 
			          &cbBytes,             // number of bytes written 
			          NULL);                // not overlapped I/O 
			    
			     if ( (!Out_val) || (strlen(DataBuffer)+1!= cbBytes))
			     {
			          printf("\nError occurred while writing" 
			                 " to the client: %d", GetLastError()); 
			          CloseHandle(hPipe);
			          return 1;  //Error
			     }
				


				
    // steps lamps
    for (i = 1; i <= 6; i++) {
      if (lampsCount[i] > 0) {
        lampsCount[i]--;
      }

      switch (lamps[i]) {
      case 0 : {
       setlight(i, RED);
       break;
      }
      case 1 : {
       setlight(i, AMBER);
       break;
      }
      case 2 : {
       setlight(i, GREEN);
       break;
      }
      case 3 : {
       setlight(i, AMBER);
       break;
      }
      }
    }
	// waiting for child process Display to end
  
 statemachine();
  }
  	//waiting for Child process to end
  	  CloseHandle(hPipe);
   CloseHandle( di.hProcess );
   CloseHandle( di.hThread );
  	CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
  return (0);
}
