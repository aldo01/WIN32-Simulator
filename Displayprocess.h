


#include <stdio.h>
#include <Windows.h>

//Name given to the pipe
#define g_szPipeName "\\\\.\\Pipe\\MyNamedPipe"
//Pipe name format - \\servername\pipe\pipename
//This pipe is for server on the same computer, 
//however, pipes can be used to
//connect to a remote server

#define BUFFER_SIZE 4096 //1k


HANDLE hPipe;







int main()
{


char  DataBuffer[BUFFER_SIZE];
DWORD cbBytes;
	
	//Send the message to server
	BOOL Out_val;


int i;

 
   
   //Connect to the server pipe using CreateFile()
   hPipe = CreateFileA( 
		g_szPipeName,	// pipe name 
		GENERIC_READ |	// read and write access 
		GENERIC_WRITE, 
		0,				// no sharing 
		NULL,			// default security attributes
		OPEN_EXISTING,	// opens existing pipe 
		0,				// default attributes 
		NULL);			// no template file 
   
   if (INVALID_HANDLE_VALUE == hPipe) 
   {
		printf("\nError occurred while connecting" 
			   " to the server: %d", GetLastError()); 
		//One might want to check whether the server pipe is busy
		//This sample will error out if the server pipe is busy
		//Read on ERROR_PIPE_BUSY and WaitNamedPipe() for that
		return 1;  //Error
   }
   else
   {
		printf("\nCommunication Channel setup with MAIN Executable Successfull");
   }

   printf("\nStart Reading\n");

   while(1){
   	
	

						  Out_val = ReadFile( 
							   hPipe,				 // handle to pipe 
							   DataBuffer,			 // buffer to receive data 
							   sizeof(DataBuffer),	 // size of buffer 
							   &cbBytes,			 // number of bytes read 
							   NULL);				 // not overlapped I/O 
						  
						  if ( (!Out_val) || (0 == cbBytes)) 
						  {
							   printf("\n%d Error occurred while reading" 
									  " from the  %d   server: %d", GetLastError(),Out_val,cbBytes); 

							   CloseHandle(hPipe);
							   
							   return 1;  //Error
						  }
						  
						

	
 switch (DataBuffer[0]){
   case '0':{
	 printf("phase: %s", "stop ");
	 break;
   }	   
   case '1':{
	 printf("phase: %s", "north/south ");
	 break;
   }	   
   case '2':{
	 printf("phase: %s", "east/west ");
	 break;
   }	   
   case '3':{
	 printf("phase: %s", "north/nturn ");
	 break;
   }	   
   case '4':{
	 printf("phase: %s", "south/sturn ");
	 break;
   }	   
   }
 
    printf("%2s",&DataBuffer[1]);
   	}
CloseHandle(hPipe);


return 1;

}
