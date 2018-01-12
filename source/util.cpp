#include "util.h"
char *itoa(int number, char *str, int base){
    (void) base; //assume base 10
    sprintf(str, "%i", number);
    return str;
}
