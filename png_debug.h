// Wilmer Henao
#ifndef PNG_DEBUG_H
#define PNG_DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  // provides uint32_t: portable 4 byte ints as required by the spec
#include <assert.h>
#include <arpa/inet.h> //ntohl
#define DBGL 10

// spec: refers to this 
// http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

//compares strings a and b for equality up to max_chars
int are_equal(const char* , const char* , int );

// compares strings a and b  for equality, up to the (length of b+1)  
// (i.e. assuming b is \0 terminated)  good for cases in which b is a string literal 
int strEq(const char* , const char* ) ; 


typedef unsigned char uchar; 

enum ChunkT  {
  IHDR   ,  
  cHRM ,
  gAMA ,
  iCCP   ,
  sBIT   ,
  sRGB  ,
  PLTE  ,
  bKGD ,
  hIST   ,
  tRNS  ,
  pHYs  ,
  sPLT   ,
  IDAT   ,
  tIME  ,
  iTXt   ,
  tEXt   ,
  zTXt   ,
  IEND    ,
  __UNRECOGNIZED, 
  CHUNK_TOT
};


typedef struct  {
	// Pieces of data as shown in the documentation
  uint32_t _length;
  char _type[5]; // last byte for null terminating character
	// so that we can use it as a string 

	// position of data in the file 
  uint32_t  _data_offset; 
  uint32_t  _crc;	 
} Chunk;

void initChunk(Chunk*);

typedef struct  { // see spec sec 4.1.1 or http://www.w3.org/TR/PNG/#11IHDR
  uint32_t  width;
  uint32_t height;
  uint8_t   bit_depth;
  uint8_t   color_type;
  uint8_t   compression_method;
  uint8_t   filter_method;
  uint8_t   interlace_method; 
} IHDR_Info;

void initIHDR_Info(IHDR_Info* );

typedef struct {
/////////////allocated by initPNGDbg/////////////////////
  const char* _fname; // file being debugged
  int 	_verbose;
	// a whole png file will be copied here 
	// maybe not optimal so CORRECT IF I HAVE TIME!!!
  uchar* 	_buffer;
  size_t  _bufSize;
////////////////////////////////////////////////////////////

  size_t  _cur; // current position in buffer when we are reading 
  IHDR_Info _IHDR_info; 
  size_t 	_chunk_cnt[CHUNK_TOT]; // hold counts of different chunk types
	
} PNGDbg;

// High Level debugger functions
void initPNGDbg(PNGDbg* , const char* , 
		FILE* ,size_t , int );
void runPNGDbg(PNGDbg* ); 
void clearPNGDbg(PNGDbg* ); 
void clearChunk(Chunk *);
int readChunk(PNGDbg* , Chunk* ); 
void showChunkInfo(PNGDbg* );

// parsing data in specific chunk types, 
// precondition for all is that d->_cur is set at start of data
int read_IHDR_data(PNGDbg* , Chunk* );

// reads a field from a chunk ad advances cur
// if (d->_verbose)  field is displayed, along with offset , file name and label 
uint32_t readShowU32(PNGDbg* , const char* );
uint16_t readShowU16(PNGDbg* , const char* );
uint8_t   readShowU8  (PNGDbg* , const char* );

// CHECKS:

// PRE we are at the begining of the file 
// returns true if  file is at least 8 bytes long and the first 8 bytes are PNG signature (see spec:3.1 )
// 137 80 78 71 13 10 26 10. If this is not the case debugger should abort right away 
int check_PNG_signature(PNGDbg* );

// check to be carried out after reaching end of file, checks 
int checkEND(PNGDbg* ); 

// ERROR reporting 
typedef enum {
	PNG_LESS_THAN_8_BYTES, 
	PNG_INVALID_SIGNATURE,
	CHUNK_LESS_THAN_12_BYTES,
	END_OF_BUFFER_BEFORE_CRC,
	IHDR_DECLARED_LENGTH_LESS_THAN_13
} Error;

void dbgError(PNGDbg* , Error ); 

// Low level functions to get data from the file 
// reads 4 bytes and advances d->_cur by 4, assumes cur +4 < _bufSize 
uint32_t read_uint32(PNGDbg* );
uint16_t read_uint16(PNGDbg* );
uint8_t read_uint8(PNGDbg* );

//copies n := min(num,  d->_bufSize - d->_cur)  
// bytes from d->_buffer starting at d->_cur and copies them onto dest
// advance d->_cur by n
// returns n;   
int read_char(PNGDbg* , char* , size_t);

void allocatePNGDbg(PNGDbg* );

#endif
