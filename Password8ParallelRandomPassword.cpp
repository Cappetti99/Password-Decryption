#include <atomic>
#include <chrono>
#include <crypt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <omp.h>
#include <random>
#include <sstream>


int main() {
    ////////////////////////////////////////////////////
    ///                 CREAZIONE HASH PASSWORD      ///
    ////////////////////////////////////////////////////

    const std::string salt = "AB";  // DES salt: primi 2 char
    std::string found;
    std::atomic<bool> found_flag(false);

    static std::random_device rd;
    static std::mt19937 gen(rd());
    const auto start = std::chrono::high_resolution_clock::now();
    ////////////////////////////////////////////////////
    ///                 INIZIO ITERAZIONI            ///
    ////////////////////////////////////////////////////
    constexpr int NUM_ITER=100;
    for (int i=0;i<NUM_ITER;i++) {

        //create a random date string in the format ddmmyyyy
        //with dd from 00 to 31, mm from 00 to 12, yyyy from 0000 to 2025
        std::uniform_int_distribution<int> giorno(1, 31);
        std::uniform_int_distribution<int> mese(1, 12);
        std::uniform_int_distribution<int> anno(0, 2025);

        const int g = giorno(gen);
        const int m = mese(gen);
        const int y = anno(gen);

        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << g
            << std::setw(2) << std::setfill('0') << m
            << std::setw(4) << std::setfill('0') << y;
        //printf("%s\n",oss.str().c_str());
        const char* target = crypt(oss.str().c_str(), salt.c_str());
#pragma omp parallel default(none) shared(salt,found,gen,target, found_flag)//private(found, salt) ////////////IMPORTANT: PRIVATE FOUND rende più veloce 0.02 sec di media su 60 tentativi non è vero
    {
        //printf("Thread %d in esecuzione\n", omp_get_thread_num());
        struct crypt_data data{};
        data.initialized = 0;


#pragma omp for collapse(3) schedule(dynamic) ////////////IMPORTANT: DYNAMIC VELOCIZZA
        for (int a = 0; a <= 31; ++a) {
            for (int b = 0; b <= 12; ++b) {
                for (int c = 0; c <= 2025; ++c) {
                    #pragma omp cancellation point for // IMPORTANT check flag
                    if (found_flag.load(std::memory_order_acquire)) {
                        #pragma omp cancel for
                    }



                    /////////////////////////////////IMPORTANT:COSTRUZIONE MANUALE STRINGA

                    ////////////////////////////////////////////////////
                    ///         GENERAZIONE PASSWORD ITERATIVA       ///
                    ////////////////////////////////////////////////////
                    char date[9];
                    date[0] = static_cast<char>('0' + (a / 10));
                    date[1] = static_cast<char>('0' + (a % 10));
                    date[2] = static_cast<char>('0' + (b / 10));
                    date[3] = static_cast<char>('0' + (b % 10));
                    const int y1 = c;
                    date[4] = static_cast<char>('0' + ((y1 / 1000) % 10));
                    date[5] = static_cast<char>('0' + ((y1 / 100) % 10));
                    date[6] = static_cast<char>('0' + ((y1 / 10) % 10));
                    date[7] = static_cast<char>('0' + (y1 % 10));
                    date[8] = '\0';

                    ////////////////////////////////////////////////////
                    ///                 CONTROLLO PASSWORD           ///
                    ////////////////////////////////////////////////////

                    const char* h = crypt_r(date, salt.c_str(),&data);
                    if (strcmp(h, target) == 0) {
#pragma omp critical
                        {
                            // ===============================================
                            // ==                SEZIONE CRITICA            ==
                            // ===============================================
                            if (found.empty()) {
                                found = date;
                                //printf("Password trovata yuppi: %s\n", found.c_str()); //TODO: scommentare
                                found_flag.store(true, std::memory_order_release);  // IMPORTANT set flag to true

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
        found_flag.store(false, std::memory_order_release);  // IMPORTANT set flag to true
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

