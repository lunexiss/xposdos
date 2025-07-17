#include "encrypt.h"

void encrypt(char *input, char *key) {
    int i = 0;
    int j = 0;

    while (input[i] != '\0') {
        if (key[j] == '\0') {
            j = 0; // loop the key
        }

        input[i] = input[i] ^ key[j];
        i++;
        j++;
    }
}
