// Wilmer Henao

#include "png_debug.h"
#include <stdint.h>
#include <stdio.h>
//#include <sys\errno.h> //DID NOT INCLUDE THESE LIBRARIES FOR PORTABILITY ONE OF THE COMPUTERS I TESTED AT THE
//#include <sys\stat.h>  //BEGINNING WAS WINDOWS AND LATER SOME LINUX SYSTEMS ALSO FAILED BEC. LIB NOT AVAILBLE

void process(char* name);

int verbose = 0;

int main(int argc, char** argv) {
  for (int i=1; i<argc; i++) {
    if(!strEq (argv[i], "--verbose") )  {
      process(argv[i]);
    } else {
      verbose = 1;
    }
  }
  return 0;
}

void process(char* name) {
  FILE *pFile;
  size_t fsize;

  pFile = fopen(name, "rb");
	
  if(pFile == NULL) {
    fprintf (stderr, "There was an error opening the file: %s", name);
    return;
  } else {
		//Calculation the size of the file in bytes (I should probably optimize this)
    fseek(pFile, 0, SEEK_END);
    fsize = ftell(pFile);
    PNGDbg debugger;
		// copies the whole file to internal buffer in debugger
    initPNGDbg(&debugger, name, pFile, fsize, verbose);
    fclose(pFile);
    runPNGDbg(&debugger);
		// deallocs memory allocated in init 
    clearPNGDbg(&debugger); 
	}
}
