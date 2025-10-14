#include <chrono>
#include <crypt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <omp.h>

int main() {
    std::string password = "27122000";
    std::string salt = "AB";  // DES salt: primi 2 char

    char* hash = crypt(password.c_str(), salt.c_str());
    if (!hash) {
        std::cerr << "crypt() failed\n";
        return 1;
    }
    std::string found = "";

    std::string chars = "0123456789";
    int C = (int)chars.size();
    printf("C = %d\n", C);
    auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel
    {
        //printf("Thread %d in esecuzione\n", omp_get_thread_num());
        struct crypt_data data;
        data.initialized = 0;

#pragma omp for collapse(8) schedule(static)
        for (int a = 0; a < C; ++a) {
            for (int b = 0; b < C; ++b) {
                for (int c = 0; c < C; ++c) {
                    for (int d = 0; d < C; ++d) {
                        for (int e = 0; e < C; ++e) {
                            for (int f = 0; f < C; ++f) {
                                for (int g = 0; g< C; ++g) {
                                    for (int i = 0; i < C; ++i) {

                                        char s[9] = { chars[a], chars[b], chars[c], chars[d],chars[e], chars[f], chars[g], chars[i], '\0' };

                                        char* h = crypt_r(s, salt.c_str(),&data);
                                        //printf("Candidate: %s -> Hash: %s\n", s, h);
                                        if (strcmp(h, hash) == 0) {
#pragma omp critical
                                            {
                                                if (found.empty()) {
                                                    found = s;
                                                    printf("Password trovata yuppi: %s\n", found.c_str());
                                                }
                                            }
                                        }
                                    }
                                }
                            }
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