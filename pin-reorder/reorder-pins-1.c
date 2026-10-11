#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct position {
   char x_mm[6];
   char y_mm[6];
   char orientation[3]; //180 or 0
} position;

typedef struct pin{
   position pos;
   char name[50];
   char number[50];
} pin;

int get_pos(pin pins[], FILE *fp){
      char ppch = '0';
   char pch = '0';
   char ch = '0';
   int pinid = 0;
   int i, j = 0;
   int pin_amount = 0;

   while ((ch = fgetc(fp)) != EOF){
      // ======== detect "pin" =============
      if (ppch=='p' && pch =='i' && ch=='n') {
         pin_amount ++;
         while ((ch = fgetc(fp)) != ')') {

            // ======= detect "at " ==========
            if (ppch=='a' && pch =='t' && ch==' ') {

               //copy x position number
               while ((ch = fgetc(fp)) !=' ' && ch!='\0'){
                  if (isdigit(ch) || ch =='.' || ch=='-'){
                     pins[i].pos.x_mm[j] = ch;
                     j++;
                  }
               }
               j = 0;
               //copy y position number
               while ((ch = fgetc(fp)) !=' ' && ch!='\0'){
                     if (isdigit(ch) || ch =='.' || ch=='-'){
                     pins[i].pos.y_mm[j] = ch;
                     j++;
                  }
               }
               j = 0;
               //copy orientation number
               while ((ch = fgetc(fp)) !=')'){
                     if (isdigit(ch) || ch =='.' || ch=='-'){
                     pins[i].pos.orientation[j] = ch;
                     j++;
                  }
               }
               j = 0;
               i++;
               putchar(ch);
            }
            ppch = pch;
            pch = ch;
         }
      }
      ppch = pch;
      pch = ch;
   }

   return 0;
}

int main(int argc, char *argv[])
{
   if (argc<2) {
      printf("Usage: %s <symbol_file.kicad_sym>\n", argv[0]);
   }

   FILE *fp = fopen(argv[1], "r");
   if (fp == NULL) {
      printf("\033[31m Error opening %s \n\r", argv[1]);
      return 1;
   }

   pin pins[5000] = {0};
   get_pos(pins, fp);

   // printf("\npin_amount = %d\n", pin_amount);
   // printf("pins[0].pos.x_mm = %s\n", pins[0].pos.x_mm);
   // printf("pins[0].pos.y_mm = %s\n", pins[0].pos.y_mm);

   fclose(fp);
   return EXIT_SUCCESS;
}
