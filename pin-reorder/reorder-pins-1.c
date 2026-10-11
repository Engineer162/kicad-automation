/*KNOWN BUGS:
*
* 1)
* search the file for 'cursed' and take a gander at how shit the
* rounding situation in C is. Or maybe I'm not educated but like
* wtf is this? I'd expect a wrapper or something that solves this.
* it's almost 2027 and we still can't accurately divide shit by 100.
*
* 2)
* not very robust code. Positions are not very accurate.
*
* 3)
* I know I violate DRY a lot, I don't give a shit, the compiler is
* here to optimize. I don't have time cheers nor energy too think
* about being concise and elegant all the time. It's not a bug its
* efficiency.
*/

#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct position {
   char x_mm[6];
   char y_mm[6];
   char orientation[4]; //180 or 0
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
   return fabs(strtof(pins[1].pos.y_mm, NULL) - strtof(pins[0].pos.y_mm, NULL)); // strtof instead of atof because its more precise for floating-point conversion
}

float x_interval_between_pins(const pin pins[], size_t pin_amount){ // this function does not modify the pins array, so we declare it as const
   //can be calculated after sort
   float zero_orientation_x_pos_mm = 0.0f, inv_orientation_x_pos_mm = 0.0f;
   bool found_zero = false, found_inv = false;
   
   // find 2 pins, with 0 and 180 deg orientation
   // 0 orientation pins are on the left, 180 orientation pins are on
   // the right

   for (size_t i = 0; i < pin_amount; i++) {
      //find 0
      if (pins[i].pos.orientation[0]=='0' && !found_zero) {
         zero_orientation_x_pos_mm = strtof(pins[i].pos.x_mm, NULL);
         found_zero = true;
      }
      //find 1 (180)
      if (pins[i].pos.orientation[0]=='1' && !found_inv) {
         inv_orientation_x_pos_mm = strtof(pins[i].pos.x_mm, NULL);
         found_inv = true;
      }
      // if both pins are found, we can exit the loop early
      if (found_zero && found_inv) {
         break;
      }
   }

   //this is cursed (Is it cursed because you have to round to resolve floating-point inaccuracies?)
   return fabsf(zero_orientation_x_pos_mm - inv_orientation_x_pos_mm);
}

void print_pin(pin pins[], int i){
   printf("=================\n");
   printf("i = %d\n", i);
   printf("pos.x_mm = %s\n", pins[i].pos.x_mm);
   printf("pos.y_mm = %s\n", pins[i].pos.y_mm);
   printf("pos.orientation = %s\n", pins[i].pos.orientation);
}

int update_positions(pin pins[], size_t pin_amount, float y_interval, float x_interval){
   //this will be the new position data
   char new_x_pos_mm[6]={'0'}, new_y_pos_mm[6]={'0'}, new_orientation[4]={'0'};
   int half_pin_amount = 0;
   if (pin_amount > 1) { // only calculate half_pin_amount if there are more than 1 pin
      half_pin_amount = pin_amount/2; // it's late give me a break
   } else {
      half_pin_amount = pin_amount;
   }


   // Use strncpy instead of strcpy to avoid buffer overflows
   for (int i=0; i<pin_amount; i++){
      if (i<half_pin_amount) {
         strncpy(new_x_pos_mm, "0", sizeof(new_x_pos_mm));
         snprintf(new_y_pos_mm, sizeof(pins[i].pos.y_mm), "%.2f", ((-1)*i*y_interval));
         strncpy(new_orientation, "0", sizeof(new_orientation));

         strncpy(pins[i].pos.x_mm, new_x_pos_mm, sizeof(pins[i].pos.x_mm));
         strncpy(pins[i].pos.y_mm, new_y_pos_mm, sizeof(pins[i].pos.y_mm));
         strncpy(pins[i].pos.orientation, new_orientation, sizeof(pins[i].pos.orientation));

      } else {
         snprintf(new_x_pos_mm, sizeof(pins[i].pos.x_mm), "%.2f", (x_interval));
         snprintf(new_y_pos_mm, sizeof(pins[i].pos.y_mm), "%.2f", ((-1)*(i-half_pin_amount)*y_interval));
         strncpy(new_orientation, "180", sizeof(new_orientation));

         strncpy(pins[i].pos.x_mm, new_x_pos_mm, sizeof(pins[i].pos.x_mm));
         strncpy(pins[i].pos.y_mm, new_y_pos_mm, sizeof(pins[i].pos.y_mm));
         strncpy(pins[i].pos.orientation, new_orientation, sizeof(pins[i].pos.orientation));
      }
      print_pin(pins, i);
   }
   return 0;
}

int main(int argc, char *argv[])
{
   if (argc<2) {
      printf("Usage: %s <symbol_file.kicad_sym>\n", argv[0]);
      return 1; // if we dont return here, the program will continue and crash.
   }

   FILE *fp = fopen(argv[1], "r");
   if (fp == NULL) {
      fprintf(stderr, "\033[31m Error opening %s \n\r \033[0m", argv[1]); // add "\033[0m" to reset the terminal color back to default, else the terminal will be red for the remaining session. Also use fprintf with stderr for error messages.
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

   update_positions(pins, pin_amount, y_interval, x_interval);

   // printf("\npin_amount = %d\n", pin_amount);
   // printf("pins[0].pos.x_mm = %s\n", pins[0].pos.x_mm);
   // printf("pins[0].pos.y_mm = %s\n", pins[0].pos.y_mm);

   fclose(fp);
   return EXIT_SUCCESS;
}
