#include <chrono>
#include <crypt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <omp.h>

int main() {
    ////////////////////////////////////////////////////
    ///                 CREAZIONE HASH PASSWORD      ///
    ////////////////////////////////////////////////////
    std::string password = "01031998";
    const std::string salt = "AB";  // DES salt: primi 2 char

    char* hash = crypt(password.c_str(), salt.c_str());
    if (!hash) {
        std::cerr << "crypt() failed\n";
        return 1;
    }
    std::string found;
    const char* target = hash; /////////////////////IMPORTANT: PUNTATORE COSTANTE
    auto start = std::chrono::high_resolution_clock::now();


    ////////////////////////////////////////////////////
    ///                 INIZIO ITERAZIONI            ///
    ////////////////////////////////////////////////////
    int NUM_ITER=100;
    for (int i=0;i<NUM_ITER;i++) {
#pragma omp parallel default(none) shared(target,salt,found)//private(found, salt) ////////////IMPORTANT: PRIVATE FOUND rende più veloce 0.02 sec di media su 60 tentativi non è vero
    {
        //printf("Thread %d in esecuzione\n", omp_get_thread_num());
        struct crypt_data data{};
        data.initialized = 0;


#pragma omp for collapse(3) schedule(dynamic) ////////////IMPORTANT: DYNAMIC VELOCIZZA
        for (int a = 0; a <= 31; ++a) {
            for (int b = 0; b <= 12; ++b) {
                for (int c = 0; c <= 2025; ++c) {
                    /////////////////////////////////IMPORTANT:COSTRUZIONE MANUALE STRINGA

                    ////////////////////////////////////////////////////
                    ///         GENERAZIONE PASSWORD ITERATIVA       ///
                    ////////////////////////////////////////////////////
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

                    ////////////////////////////////////////////////////
                    ///                 CONTROLLO PASSWORD           ///
                    ////////////////////////////////////////////////////

                    char* h = crypt_r(date, salt.c_str(),&data);
                    if (strcmp(h, target) == 0) {
#pragma omp critical
                        {
                            // ===============================================
                            // ==                SEZIONE CRITICA            ==
                            // ===============================================
                            if (found.empty()) {
                                found = date;
                                //printf("Password trovata yuppi: %s\n", found.c_str()); //TODO: scommentare

                            }

                            // ===============================================
                            // ==                FINE SEZIONE CRITICA       ==
                            // ===============================================
                        }

                    }
                } // fine loop anno
            } // fine loop mese
        } // fine loop giorno
    } // fine parallel
    found=""; // IMPORTANT reset found per loop successivo
}

    ////////////////////////////////////////////////////
    ///                 PRINT DI CONTROLLO           ///
    ////////////////////////////////////////////////////

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Tempo impiegato: " << elapsed.count() << " secondi\n";
    std::cout<<"Tempo medio:"<<(elapsed.count()/NUM_ITER)<<" secondi\n";
    if (!found.empty()) {
        printf("Pass finale: %s\n", found.c_str());
    } else {
        printf("Password non trovata\n");
    }

    return 0;
}

//media=0.7679
//media senza puntatore const è uguale
//media senza private è minore 0.75...
//fallimento totale
//dynamic velocizza  0.74

