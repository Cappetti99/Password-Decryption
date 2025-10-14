#include <crypt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <chrono>

int main() {
    std::string password = "27122000";
    std::string salt = "AB";  // DES salt: primi 2 char

    char* hash = crypt(password.c_str(), salt.c_str());
    std::string passHash = hash;
    if (!hash) {
        std::cerr << "crypt() failed\n";
        return 1;
    }
    std::string found;
    auto start = std::chrono::high_resolution_clock::now();
        for (int a = 0; a <= 31; ++a) {
            for (int b = 0; b <= 12; ++b) {
                for (int c = 0; c <= 2025; ++c) {
                    char date[9];
                    std::snprintf(date, 9, "%02d%02d%04d", a, b, c);
                    std::string s(date);
                    char* h = crypt(s.c_str(), salt.c_str());
                    if (strcmp(h, passHash.c_str()) == 0) {
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