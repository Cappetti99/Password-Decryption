#include <crypt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <chrono>

int main() {
    std::string password = "27122000";
    const std::string salt = "AB";  // DES salt: primi 2 char

    char* hash = crypt(password.c_str(), salt.c_str());

    if (!hash) {
        std::cerr << "crypt() failed\n";
        return 1;
    }
    const std::string target = hash;
    std::string found;
    auto start = std::chrono::high_resolution_clock::now();
        for (int a = 0; a <= 31; ++a) {
            for (int b = 0; b <= 12; ++b) {
                for (int c = 0; c <= 2025; ++c) {

                    char date[9];
                    date[0] = char('0' + (a / 10));
                    date[1] = char('0' + (a % 10));
                    date[2] = char('0' + (b / 10));
                    date[3] = char('0' + (b % 10));
                    int y = c;
                    date[4] = char('0' + ((y / 1000) % 10));
                    date[5] = char('0' + ((y / 100) % 10));
                    date[6] = char('0' + ((y / 10) % 10));
                    date[7] = char('0' + (y % 10));
                    date[8] = '\0';
                    char* h = crypt(date, salt.c_str());
                    if (strcmp(h,target.c_str()) == 0) {
                        {
                            if (found.empty()) {
                                found = date;
                                printf("Password trovata yuppi: %s\n", found.c_str());
                            }
                        }
                    }
                }
            }
        }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi\n";
    if (!found.empty()) {
        printf("Pass finale: %s\n", found.c_str());
    } else {
        printf("Password non trovata\n");
    }

    return 0;
}