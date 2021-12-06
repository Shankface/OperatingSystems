
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h> 

char buf[4096];

// prints what type of error occured for a given file or standard I/O
void printError(char *fileName, char *issue){
    fprintf(stderr, "Error occured %s file [%s] \n", issue, strcmp(fileName,"-") == 0 ? "standard input/output" : fileName);
    exit(-1);
}

// checks if given data is binary 
bool isBinary(char *buf, int length, char *inpFile){
    int i;
    for (i = 0; i < length; i++){
        if(!(isprint(buf[i]) || isspace(buf[i]))){
            fprintf(stderr, "WARNING: [%s] is a binary file.\n", inpFile);
            return true;
        }
    }
    return false;
}

// writes remaining data in partial write case
bool partialWrite(int r, int w, int *writeCount, char *buf, int *outFile){
    int i;
    char newBuf[r - w];
    for(i = w; i < r; i++){ // copies remaining data from buf that wasn't read to new buf
        newBuf[i - w] = buf[i];
    }
    int wr = write(*outFile, newBuf, r-w); // writes the new buf into the output
    if(wr < 0) printError("Output File", "writing to");
    *writeCount++;
}

// reads from a given file and writes it to end of given output file
void concat(char *inpFile, int *outFile){
    int readCount = 1;
    int writeCount = 0, byteCount = 0;
    int fdInp = strcmp(inpFile, "-") == 0 ? STDIN_FILENO : open(inpFile, O_RDONLY, 0666); // attempts to open file if a file  name is given or opens standard input if "-" is given 
    if(fdInp < 0) printError(inpFile, "opening"); // returns opening error if opening fails
    int r = read(fdInp, buf, 4096);
    if(r < 0) printError(inpFile, "reading"); // returns reading error if reading fails
    bool binary = false;

    while(r > 0){ // while data remains in the input file to read
        if(!binary) binary = isBinary(buf, r, inpFile);

        int w = write(*outFile, buf, r);
        if(w < 0) printError("Output File", "writing to"); // returns writing error if writing fails

        writeCount++;

        if(w != r) partialWrite(r, w, &writeCount, buf, outFile); // if less bytes were written than should be, try to write the missing data
        byteCount += r;

        if(r < 4096) break; // if less than 4096 bytes of data were read from the input then break out of loop because it means EOF was reached
        r = read(fdInp, buf, 4096);
        if(r < 0) printError(inpFile, "reading");
        readCount++;
    }

    if(strcmp(inpFile, "-") != 0) // closes file if input is not standard input
        if(close(fdInp) < 0) printError(inpFile, "closing"); //returns closing error if closing fails
    fprintf(stderr, "\nFile [%s] transferred %d bytes, read %d time(s), and wrote %d time(s).\n", strcmp(inpFile,"-") == 0 ? "standard input" : inpFile, byteCount, readCount, writeCount); // prints details about the concatenation
}

int main(int argc, char *argv[]){
    int c, i; 
    int outFile = STDOUT_FILENO; // output is standart out by default 

    while ((c = getopt(argc, argv, "o:")) != -1){ // if 'o' is included in the command arguments then set output file to what's written after it
        if(c == 'o'){
            outFile = open(optarg,O_WRONLY|O_CREAT|O_TRUNC,0666);
            if(outFile < 0) printError(optarg, "open");
        } else abort();
    }
    
    if(optind != argc){ // if any input files or hyphens are given, go through them one by one and concat them with output file
        for (i = optind; i < argc; i++){
            concat(argv[i], &outFile);
        }
    }

    else concat("-", &outFile); // if no input files or hyphens are given for inputs, then make standard input the input
    return 0;
}