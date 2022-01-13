#include "conversion.h"

char *itoa(int digit) {
    int i, remainder = 0;      // to store the last digit
    char *numbers;       // converted integer to chars stoarge
    int count_d = i = count_digit(digit);

    if(count_d > 0 && count_d < 2) {        // check if the digit is one, then convert.
        *numbers = digit + '0';
        *(numbers+1) = '\0';
        return numbers;
    } // otherwise the digit is more than one

    // the first thing you must do is reverse the number.
    // by invoking the reverse_Number func
    // digit = reverse_number(digit);
    while(digit > 0) {
        // Get the last digit
        remainder = digit % 10;
        numbers[i-1] = remainder + '0';       // add the ascii value of '0' character to get its character equlivalent.
        digit /= 10;                        // divide the digit by 10 to proceed with the number digit
        if(digit == 0) {
            numbers[count_d] = '\0';
        }
        i--;                                // incrementing i
    }
    return numbers;
}

int reverse_number(int digit) { 
    int remainder = 0, reverse = 0;
    while(digit != 0) {
        // Get the last digit
        remainder = digit % 10;
        // reverse the number
        reverse = reverse * 10 + remainder;
        // divide the digit by 10 to proceed with the other digits.
        digit = digit / 10;
    }

    // after all return the reversed number.
    return reverse;
}

int count_digit(int digit) {
    int counter = 0;
    if(digit == 0) {
        return 1;
    }

    while(digit > 0) {
        digit = digit / 10;
        counter++;
    }
    return counter;
}