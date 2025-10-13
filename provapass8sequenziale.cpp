//
// Created by lapemaya on 10/13/25.
//

#include <crypt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <omp.h>

int main() {
    std::string password = "27122000";
    std::string salt = "AB";  // DES salt: primi 2 char

    char* hash = crypt(password.c_str(), salt.c_str());
    std::string passHash = hash;
    if (!hash) {
        std::cerr << "crypt() failed\n";
        return 1;
    }
    printf(hash);
    printf("\n");
    std::string found = "";

    std::string chars = "0123456789";
    int C = (int)chars.size();
    printf("C = %d\n", C);


    for (int a = 0; a < C; ++a) {
        for (int b = 0; b < C; ++b) {
            for (int c = 0; c < C; ++c) {
                for (int d = 0; d < C; ++d) {
                    for (int e = 0; e < C; ++e) {
                        for (int f = 0; f < C; ++f) {
                            for (int g = 0; g< C; ++g) {
                                for (int i = 0; i < C; ++i) {
                                    char s[9] = { chars[a], chars[b], chars[c], chars[d],chars[e], chars[f], chars[g], chars[i], '\0' };
                                    char* h = crypt(s, salt.c_str());
                                   // printf("Candidate: %s -> Hash: %s\n", s, h);
                                    //printf("%s\n", hash);
                                    if (strcmp(h, passHash.c_str()) == 0) {
                                        {
                                            if (found.empty()) {
                                                found = s;
                                                printf("Password trovata yuppi: %s\n", found.c_str());
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            printf("b= %d\n", b);
        }
        printf("a= %d\n", a);

    }

    if (!found.empty()) {
        printf("Pass finale: %s\n", found.c_str());
    } else {
        printf("Password non trovata\n");
    }

    return 0;
}