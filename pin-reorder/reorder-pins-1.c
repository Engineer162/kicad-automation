#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

size_t get_pos(pin pins[], FILE *fp){
   char ppch = '0';
   char pch = '0';
   char ch = '0';
   int pinid = 0;
   int i = 0, j = 0;
   size_t pin_amount = 0;

   while ((ch = fgetc(fp)) != EOF){
      // ======== detect "pin" =============
      if (ppch=='p' && pch =='i' && ch=='n') {
         pin_amount ++;
         while ((ch = fgetc(fp)) != ')') {

            // ======= detect "at " ==========
            if (ppch=='a' && pch =='t' && ch==' ') {

               //copy x position number
               while ((ch = fgetc(fp)) !=' '){
                  if (isdigit(ch) || ch =='.' || ch=='-'){
                     pins[i].pos.x_mm[j] = ch;
                     j++;
                  }
               }
               j = 0;
               //copy y position number
               while ((ch = fgetc(fp)) !=' '){
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
            }
            ppch = pch;
            pch = ch;
         }
      }
      ppch = pch;
      pch = ch;
   }
   rewind(fp);
   return pin_amount;
}

size_t get_name_and_number(pin pins[], FILE *fp){
   char ppch = '0';
   char pch = '0';
   char ch = '0';
   int pinid = 0;
   int i = 0, j = 0;
   size_t pin_amount = 0;

   while ((ch = fgetc(fp)) != EOF){
      // ======== detect "e "" (for "name ") =============
      if (ppch=='e' && pch ==' ' && ch=='"') {
         pin_amount++;
         while ((ch = fgetc(fp))!='"') {
            //copy name
            pins[i].name[j] = ch;
            j++;
         }
         j = 0;

         //jump to next "
         while ((ch = fgetc(fp)) !='"'){}

         //copy number (can include letters)
         while ((ch = fgetc(fp)) !='"'){
            pins[i].number[j] = ch;
            j++;
         }
         j = 0;
         i++;
      }
      ppch = pch;
      pch = ch;
   }

   //exit loop at "effect"
   // if (ppch=='e' && pch =='f' && ch=='f') break;
   ppch = pch;
   pch = ch;

   rewind(fp);
   return pin_amount;
}

//this function signature is enforced by qsort, any other won't work
int compare(const void* a, const void* b){
   const pin* pina = (const pin*)a;
   const pin* pinb = (const pin*)b;
   return strcmp(pina->name, pinb->name);
}

int reorder(pin pins[], FILE *fp, size_t pin_amount){
   qsort(pins, pin_amount, sizeof(pin), compare);
   return 0;
}

float y_interval_between_pins(pin pins[]){
   //needs to be calculated before sort!
   float y_interval = fabs(atof(pins[1].pos.y_mm) - atof(pins[0].pos.y_mm));
   return round(y_interval*100.0)/100.0;
}

float x_interval_between_pins(pin pins[], size_t pin_amount){
   //can be calculated after sort
   float x_interval=0;
   int i=0;
   // find 2 pins, with 0 and 180 deg orientation
   // 0 orientation pins are on the left, 180 orientation pins are on
   // the right
   float zero_orientation_x_pos_mm=0, inv_orientation_x_pos_mm=0;
   for (int i=0; i<pin_amount; i++) {
      //find 0
      if (pins[i].pos.orientation[0]=='0') {
         zero_orientation_x_pos_mm = atof(pins[i].pos.x_mm);
      }
      //find 1 (180)
      if (pins[i].pos.orientation[0]=='1') {
         inv_orientation_x_pos_mm = atof(pins[i].pos.x_mm);
         break;
      }
   }

   //this is cursed
   x_interval = fabsf(zero_orientation_x_pos_mm - inv_orientation_x_pos_mm);
   return round(x_interval*100.0)/100.0;
}

int update_positions(pin pins[], size_t pin_amount, size_t y_interval, size_t x_interval){

   for (int i=0; i<pin_amount; i++){
       // ftoa(i*x_interval, pins[i].pos.x_mm, 2);
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
   //pin amount can be used to check if both functions parsed the same
   //amount of pins but idc for now
   size_t pin_amount = get_pos(pins, fp);
   get_name_and_number(pins, fp);
   float x_interval = x_interval_between_pins(pins, pin_amount);
   float y_interval = y_interval_between_pins(pins);
   reorder(pins, fp, pin_amount);

   printf("detected x interval = %f\n", x_interval);
   printf("detected y interval = %f\n", y_interval);

   // printf("\npin_amount = %d\n", pin_amount);
   // printf("pins[0].pos.x_mm = %s\n", pins[0].pos.x_mm);
   // printf("pins[0].pos.y_mm = %s\n", pins[0].pos.y_mm);

   fclose(fp);
   return EXIT_SUCCESS;
}
