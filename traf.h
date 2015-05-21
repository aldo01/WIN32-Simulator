/* traf.h - definitions/declarations for intersection subroutines */

#define MAP_FILENAME    "traffile"
#define MAP_MAPNAME     "mapmemory"

#define OFF 0
#define ON 1

#define NFLOW 1000		// North flow rate = 1000
#define SFLOW 900		// South flow rate = 900
#define EFLOW 700		// East flow rate = 700
#define WFLOW 600		// West flow rate = 600

#define NORTH 1			// North signal constant = 1 
#define NTURN 2			// North turn signal constant = 2 
#define WEST 3			// West signal constant = 3
#define SOUTH 4			// South signal constant = 4
#define STURN 5			// South turn signal constant = 5
#define EAST 6			// East signal constant = 6
	
#define  GREEN 2 		// green = 0
#define  AMBER 1        // amber = 1
#define  RED 0          // red = 2


//Name given to the pipe
#define g_szPipeName "\\\\.\\Pipe\\MyNamedPipe"
//Pipe name format - \\.\pipe\pipename

#define BUFFER_SIZE 4096 //1k



typedef struct traffic
{
short sensor[7];
short light[7];
char key;
short quit;
short crash;
} traffic;

short trafopen(void);
    
void trafclose(void);

char getkey(void);

short sensor(short s);

short setlight(short n, short colour);

short quit(short q);

void reset(void);

short crash(void);
