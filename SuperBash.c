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
#include <string.h>

typedef enum {false=0, true=1} bool;

char fgetcurrc(FILE* file);
void fskipc(int n, FILE* file);
bool fmatchs(char* str, FILE* file);

int main(int argc, char* argv[]) {
 char c = fgetc(stdin);

 while (c != EOF) {
  //trims spaces before and after '='
  if (c == ' ' || c == '=') {
   int spaces = 0;
   for (; c == ' '; c = fgetc(stdin)) spaces++;
   if (c == '=') {
    fputc('=', stdout);
    for (c = fgetc(stdin); c == ' '; c = fgetc(stdin));
   }
   else for (; spaces > 0; spaces--) fputc(' ', stdout);
  }

  //puts newline after closing bracket if there isn't one there already
  else if (c == ']') {
   fputc(']', stdout);
   c = fgetc(stdin);
   if (c != '\n') fputc('\n', stdout);
  }

  //replaces repeat with first part of while loop
  else if (c == 'r') {
   char s[] = "repeat ";
   char b[8] = "r";
   int i;
   for (i = 0; b[i] == s[i]; i++) b[i+1] = fgetc(stdin);
   c = b[i];
   b[i] = '\0';
   if (i == strlen(s)) fputs("repeatIndex1=0\n\nwhile [ $repeatIndex1 -lt ", stdout);
   else fputs(b, stdout);
  }

  //replaces times with second part of while loop
  else if (c == 't') {
   char s[] = "times\n";
   char b[9] = "t";
   int i;
   for (i = 0; b[i] == s[i]; i++) b[i+1] = fgetc(stdin);
   c = b[i];
   b[i] = '\0';
   if (i == strlen(s)) fputs("]\n", stdout);
   else fputs(b, stdout);
  }

  // replace opening brack with third part of while loop
  else if (c == '{') {
   fputs("do", stdout);
   c = fgetc(stdin);
  }

  //replaces closing bracket with fourth part of while loop
  else if (c == '}') {
   fputs("repeatIndex1=$[$repeatIndex1+1]\n\ndone", stdout);
   c = fgetc(stdin);
  }

  //else just output the character
  else {
   fputc(c, stdout);
   c = fgetc(stdin);
  }
 }
}
