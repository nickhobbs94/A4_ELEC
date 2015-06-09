#include "alt_types.h"
#include "conversions.h"
#include "altstring.h"
#include <stdio.h>

int main(void){
	alt_u8 arry[3] = {1,2,3};
	alt_32 ans = extract_little(arry, 3);
	printf("%x\n", ans);

	alt_8 string1[] = "Why, hello there!";
	alt_8 string2[20];
	altstrcpy(string2, string1);
	uppercasestring(string2);
	printf("%s\n", string2);
	return 0;
}