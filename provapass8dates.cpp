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
    double start = omp_get_wtime();

#pragma omp parallel
    {
        printf("Thread %d in esecuzione\n", omp_get_thread_num());
        struct crypt_data data;
        data.initialized = 0;


#pragma omp for collapse(3) schedule(static)
        for (int a = 0; a < 31; ++a) {
            for (int b = 0; b < 12; ++b) {
                for (int c = 0; c < 2025; ++c) {
                    char date[9];
                    //std::snprintf(date, 8, "%02d%02d%04d", a, b, c);
                    date[8] = '\0';
                    printf("data=%s\n", date);


                    //char s[9] = { chars[a], chars[b], chars[c], chars[d],chars[e], chars[f], chars[g], chars[i], '\0' };

                    char* h = crypt_r(date, salt.c_str(),&data);
                    //printf("Candidate: %s -> Hash: %s\n", s, h);
                    if (strcmp(h, hash) == 0) {
#pragma omp critical
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
    }
    double end= omp_get_wtime();
    printf("Tempo finale=%f\n", end-start);
    if (!found.empty()) {
        printf("Pass finale: %s\n", found.c_str());
    } else {
        printf("Password non trovata\n");
    }

    return 0;
}