//Wilmer Henao

#include "png_debug.h"
#include "CRCChecks.h"

int are_equal(const char* a, const char* b, int max_chars) {
  return (strncmp(a,b, max_chars)==0);
}

int strEq(const char* a, const char* b) {
  return (strncmp(a,b , strlen(b) +1) == 0);
}

void dbgError(PNGDbg* d, Error e) {
/*Print an error according to its type*/
  const char* msg = "";
  switch(e) {
    #define ERRORCASE(x)   case x:  msg= #x;  break;
	ERRORCASE(PNG_LESS_THAN_8_BYTES)
	ERRORCASE(PNG_INVALID_SIGNATURE)
	ERRORCASE(CHUNK_LESS_THAN_12_BYTES)
	ERRORCASE(END_OF_BUFFER_BEFORE_CRC)
	ERRORCASE(IHDR_DECLARED_LENGTH_LESS_THAN_13)
    #undef ERRORCASE
		default :
    msg = "DEFAULT MSG";
  }

  fprintf(stderr, "%08x :  %s : Error(%s)",
	  (unsigned int)d->_cur, d->_fname, msg);
}

// Constructors
void initChunk(Chunk* ch) {
  memset(ch, 0, sizeof(*ch));	
};

void initIHDR_Info(IHDR_Info* i) {
  memset(i, 0, sizeof(*i));
}

void allocatePNGDbg(PNGDbg* d){
  memset(d, 0, sizeof(*d));
}

void initPNGDbg(PNGDbg* d, const char* fname, 
		FILE* ifile, size_t fsize, int verbose){
/* This function initializes the data into the PNGDbg object which contains most important info in the PNG
file
INPUT:  - d:An empty PNGDbg structure
	- fname: name of the PNG file
	- iFile: object of type FILE containing the png in bytes
	- fsize: Size in bytes of the PNG file
	- verbose: Checking "--verbose"
OUTPUT: - d: Filled out with the propper information
*/
  allocatePNGDbg(d);
  d->_verbose = verbose;
  d->_fname = fname;
  d->_bufSize = fsize;
 	
  fseek ( ifile , 0L , SEEK_SET ); // Position @ beginning
	
	// This taken from http://www.cplusplus.com/reference/clibrary/cstdio/fread/
  d->_buffer = (uchar*)malloc(fsize); // Allocate memory in the heap to contain the whole file
  if (d->_buffer == NULL) {
    fprintf (stderr, "Memory error when trying to allocate %d bytes to file: %s",
	     (int)fsize, d->_fname);
    exit (2);
  }
	
  size_t result = fread ((char*)d->_buffer,1,fsize, ifile); // Copy the file into the buffer (CHECK CAST!)
  if (result != fsize) {
    fputs ("Reading error",stderr); exit (3);
  } else {
#if DBGL >0 
    fprintf(stdout, "Successfully read %d bytes from file %s\n", (int)result, d->_fname);
#endif 	
  }
};

void clearPNGDbg(PNGDbg* d){
  if(d->_buffer) {
    free(d->_buffer);
    d->_buffer = NULL;
  }
}

void clearChunk(Chunk* ch){
  free(ch);
}

int check_PNG_signature(PNGDbg* d){
/*This is the function that checks the signature of the PNG file to make sure that it is INDEED*/
/*The first eight bytes of a PNG file always contain the following (decimal) values: 137 80 78 71 13 10 26 10
Check that this is the case and if not then throw error.  This is 8 bytes*/
/*Hexadecimal values are 89 50 4E 47 0D 0A 1A 0A.  This will be printed*/
  d->_cur = 0; 
  if (d->_bufSize < 8) {
    dbgError(d, PNG_LESS_THAN_8_BYTES);
    return 0;
  }
	
  fprintf(stdout, "File %s has signature:  %02x %02x %02x %02x %02x %02x %02x %02x\n", 
	  d->_fname,
	  d->_buffer[0], d->_buffer[1], d->_buffer[2], d->_buffer[3] ,
	  d->_buffer[4], d->_buffer[5], d->_buffer[6], d->_buffer[7] ); // print signtr

  const uchar PNGsng[8] = { 137, 80, 78, 71, 13, 10, 26, 10};
  for(size_t i=0; i< 8; i++){
    if( (uchar) d->_buffer[i] != PNGsng[i]){
      dbgError(d, PNG_INVALID_SIGNATURE); // If not PNG Throw error
      return 0;
    }
  }

  d->_cur = 8; // Move the pointer 8 bytes (right after signature)
  return 1;
}


uint32_t readShowU32(PNGDbg* d, const char* label ) {
  size_t curBefore = d->_cur;
  uint32_t read= read_uint32(d);

  if(d->_verbose> 0){
    fprintf(stdout, "%08x : %s :\t\t%s= %d\n", 
	    (unsigned int)curBefore, d->_fname, label, read );
  }
  return read;
}

uint16_t readShowU16(PNGDbg* d, const char* label ) {
  size_t curBefore = d->_cur;
  uint16_t read= read_uint16(d);
	
  if(d->_verbose> 0) {
    fprintf(stdout, "%08x : %s :\t\t%s= %d\n",
	    (unsigned int)curBefore, d->_fname,  label, read ); 
  }
  return read;
}

uint8_t readShowU8(PNGDbg* d, const char* label ) {
  size_t curBefore = d->_cur;
  uint8_t read= read_uint8(d);

  if(d->_verbose> 0) {
    fprintf(stdout, "%08x : %s :\t\t%s= %d\n",
	    (unsigned int)curBefore, d->_fname,  label, read );
  }
  return read;
}

int readChunk(PNGDbg* d, Chunk* ch) {
/*This function does most of the calculations.
INPUT: d: PNGDbg with all the png data, buffer, specifications, etc.
       ch: a Chunk structure (empty)
OUTPUT:  Prints to screen most of the debugger's information
*/

  char* supChunks[19] = {"IHDR","cHRM","gAMA","iCCP","sBIT","sRGB","PLTE","bKGD","hIST",
			 "tRNS","pHYs","sPLT","IDAT","tIME","iTXt","tEXt","zTXt","IEND"," __UNRECOGNIZED"};
  size_t stbegin = 0;
  if (d->_bufSize - d->_cur  < 12){ //Length, Type and CRC all have 4 bytes => 4*3 = 12 bytes minimum  
    dbgError(d, CHUNK_LESS_THAN_12_BYTES);
    return 0;
  }

  ch->_length = read_uint32(d);
  stbegin = d->_cur;

  if(d->_verbose) { 
    fprintf(stdout, "%08x : %s :  Chunk starts. length: %d\n" ,
	    (unsigned int)(d->_cur-4), 	d->_fname, ch->_length);
  }

  read_char(d, ch->_type, 4); //Chunk type has 4 types according to specs
	
  ch->_type[4] ='\0'; // just in case I want to use it as text
	
  fprintf(stdout, "%08x : %s :\tChunk type: '%s'\n" , (unsigned int)(d->_cur-4),
	  d->_fname, ch->_type);
	// Throw warning if chunk does not belong
  int match = 0;
  for(size_t i =0; i<19; i++){
    if(strEq(ch->_type, supChunks[i])){
      match=1;
    }
  }
  if(!match){
    fprintf(stderr, "WARNING: '%s' is not necessarily a supported chunk type\n", ch->_type);
  }
	// Continue with data member
  if (d->_cur + ch->_length  > d->_bufSize) {
    fprintf(stderr, "%08x : %s :\tERROR: insufficient data for chunk of declared data length %d\n",
	    (unsigned int)d->_cur, d->_fname, ch->_length);
    return 0;
  }
	
  ch->_data_offset = d->_cur; //Where the data starts.  Length is variable.  Could be zero.

  if(strEq(ch->_type, "IHDR") ){  //CHECK IF I SHOULD CHECK FOR DECIMALS 73 72 68 82 instead
    read_IHDR_data(d, ch);
  } else {
  	if(d->_verbose) { //Enter here if it's not IHDR chunck
    		fprintf(stdout, "%08x : %s :\tdata: %d bytes\n" ,
		    (unsigned int)(d->_cur), d->_fname, ch->_length);
  	}
  d->_cur += ch->_length; //Position pointer after data
  }
	
  if (d->_cur + 4 > d->_bufSize) {
    dbgError(d, END_OF_BUFFER_BEFORE_CRC);
    return 0;
  }

  // CRC TEST RESULTS
  unsigned long crctest = 0;	
  crctest = crc(&(d->_buffer[stbegin]), (int)(d->_cur-stbegin) ); //Do the test of the CRC hash
  fprintf(stdout, "%08x : %s :\tResult of the CRC test is %08lx\n", (unsigned int)(d->_cur-4), d->_fname, (long unsigned int)ntohl(crctest));

  // CRC from the buffer
  ch->_crc = read_uint32(d);
  if(d->_verbose) {
    fprintf(stdout, "%08x : %s :\tCRC signature=%08x\n" ,
	    (unsigned int)(d->_cur-4), d->_fname, ntohl(ch->_crc));
  }
  if(ntohl(ch->_crc) != ntohl(crctest)){
    fprintf(stderr, "Warning: CRC signatures don't match\n");
  }

  return 1;	//Return one if success (to multiply with allGood variable in caller)
}

int read_IHDR_data(PNGDbg* d, Chunk* ch) {
  if(ch->_length < 13) {
    dbgError(d, IHDR_DECLARED_LENGTH_LESS_THAN_13); //IHDR needs 13 bytes of data...
    return 0;
  }
	
  if(ch->_length > 13) {
    fprintf(stderr, "WARN: IHDR declared length is more than 13. will ignore bytes after 13\n");
  }
	
  d->_IHDR_info.width   = readShowU32(d, "width");
  d->_IHDR_info.height = readShowU32(d, "height");
	
  d->_IHDR_info.bit_depth  = readShowU8(d, "bit_depth");
  d->_IHDR_info.color_type = readShowU8(d, "color_type");
  d->_IHDR_info.compression_method = readShowU8(d, "compression_methode");
  d->_IHDR_info.filter_method 		= readShowU8(d, "filter_method");
  d->_IHDR_info.interlace_method 	= readShowU8(d, "interlace_method");
		
  d->_cur += (ch->_length - 13);  // should be 0 for a good png. 

  return 1;
}
/////////////////////////////////////////////////////////////////////////////////////
// SEE IF I CAN PUT THEM TOGETHER IN ONE STRUCTURE
uint32_t read_uint32(PNGDbg* d) {
// Reads endian safe data
  assert(d->_cur + 4 <= d->_bufSize);
  uint32_t result = 0u; //0u is the Minimum value for this type
	
  for (size_t i = 0; i < 4 ; ++i) {
    result <<= 8; // Do 8 bits = 1 byte @ a time
    result |= d->_buffer[d->_cur]; //basically copies ones to result
    d->_cur++; //Move cur one byte up
  }
  return result;
}

uint16_t read_uint16(PNGDbg* d){
// Reads two bytes endian safe
  assert(d->_cur + 2<= d->_bufSize); 
  uint16_t result = 0u;
	
  for (size_t i = 0; i < 2 ; ++i) {
    result <<= 8;
    result |= d->_buffer[d->_cur];
		  
    d->_cur++;
  }
  return result;
}

uint8_t read_uint8(PNGDbg* d){
// Reads one byte endian safe no for cycle needed
  uint8_t result = 0;
  assert(d->_cur + 1<= d->_bufSize);
  result |= d->_buffer[d->_cur];
  d->_cur++;
  return result;
}
//////////////////////////////////////////////////////////////////////////////////////

void runPNGDbg(PNGDbg* d) {
/*This function wraps up a few other functions that make all the heavy lifting*/
  if(!check_PNG_signature(d) )  { return; } ;
	
  Chunk ch; //Chunk has 4 members from documentation
  int allGood =1;
  while(d->_cur < d->_bufSize && allGood) {
    initChunk(&ch);
    allGood &= readChunk(d, &ch);
  }
	//clearChunk(&ch); // Not in the heap => Unnecessary
}

int read_char(PNGDbg* d, char* dest  , size_t num) {
// copies n := min(num,  d->_bufSize - d->_cur)  
// bytes from d->_buffer starting at d->_cur and copies them onto dest
// returns n;
  int available = d->_bufSize - d->_cur;
  assert(available > 0 ); //probably redundant since I already checked for 12 bytes.
  size_t n = ( (int) num < available ? num : (size_t) available);
	
  memcpy( dest, (char*) &(d->_buffer[d->_cur] ) ,  n ); // Cpy frm Addrss strtng @ crrnt pstn
  d->_cur += n; // Update adress position
  return n; //Return anything.  Doesn't matter.
}
