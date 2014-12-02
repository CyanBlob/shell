/*
 Group:   16
 Members: George Davis
          Shashi Dongur
          Gary Johnson
          Andrew Thomas
 Class:   CSCE3600
 Session: Fall 2014
 Section: T/Th 10:00am
 Project: Minor Assignment - SuperBash
 Compile: gcc main.c -o SuperBash
 Usage:   SuperBash [infile] [outfile]
*/

#include <stdio.h>
typedef enum {false=0, true=1} bool;

char fgetcurrc(FILE* file);
void fskipc(int n, FILE* file);
bool fmatchs(char* str, FILE* file);

int main(int argc, char* argv[]) {
 FILE* inFile = fopen(argv[1], "r");
 FILE* outFile = fopen(argv[2], "w");
 //FILE* outFile = stdout; //debug

 char c;

 for (c = fgetc(inFile); c != EOF; c = fgetc(inFile)) {
  //trims spaces before and after '='
  if (c == ' ' || c == '=') {
   int spaces = 0;
   for (c = fgetcurrc(inFile); c == ' '; c = fgetc(inFile)) spaces++;
   if (c == '=') {
    fputc('=', outFile);
    for (c = fgetc(inFile); c == ' '; c = fgetc(inFile));
   }
   else {
    int i;
    for (i = 0; i < spaces; i++) fputc(' ', outFile);
   }
   fskipc(-1, inFile);
  }

  //puts newline after closing bracket
  else if (c == ']') {
   fputs("]\n", outFile);
  }

  //replaces repeat with while loop
  else if (fmatchs("repeat", inFile)) {
   fputs("repeatIndex1=0\n\n", outFile);
   fputs("while [ $repeatIndex1 -lt ", outFile);
   for (c = fgetc(inFile); !fmatchs("times\n\n{", inFile); c = fgetc(inFile)) fputc(c, outFile);
   fputs("]\n\ndo\n", outFile);
   for (c = fgetc(inFile); !fmatchs("}", inFile); c = fgetc(inFile)) fputc(c, outFile);
   fputs("repeatIndex1=$[$repeatIndex1+1]\n\ndone\n", outFile);
  }

  else fputc(c, outFile);
 }

}

//gets current character
char fgetcurrc (FILE* file) {
 fseek(file, -1, SEEK_CUR);
 return fgetc(file);
}

//skips n characters
void fskipc (int n, FILE* file) {
 fseek(file, n, SEEK_CUR);
}

//matches and consumes string in file, returns true if successful
bool fmatchs (char* str, FILE* file) {
 char c;
 int skip = 0;
 for (c = fgetcurrc(file); *str != '\0'; c = fgetc(file)) {
  if (c == *str) {
   str++;
   skip--;
  }
  else {
   fskipc(skip, file);
   return false;
  }
 }
 return true;
}
