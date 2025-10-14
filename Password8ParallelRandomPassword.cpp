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



// Funzione per mostrare la barra di progresso
void printProgressBar(int current, int total, double elapsed_time, int bar_width = 50) {
    float progress = (float)current / total;
    int pos = bar_width * progress;

    std::cout << "\r[";
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "█";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "% ";
    std::cout << current << "/" << total << " ";

    if (current > 0) {
        double avg_time = elapsed_time / current;
        double remaining_time = avg_time * (total - current);
        std::cout << "| Tempo: " << std::fixed << std::setprecision(1) << elapsed_time << "s ";
        std::cout << "| Rimanente: " << std::fixed << std::setprecision(1) << remaining_time << "s ";
        std::cout << "| Media: " << std::fixed << std::setprecision(3) << avg_time << "s/it";
    }

    std::cout << std::flush;
}


int main(int argc, char* argv[]) {
    ////////////////////////////////////////////////////
    ///                 CREAZIONE HASH PASSWORD      ///
    ////////////////////////////////////////////////////

    // Gestione numero di thread da linea di comando o input interattivo
    int num_threads;
    int max_threads = omp_get_max_threads();

    if (argc > 1) {
        // Modalità da linea di comando
        num_threads = std::atoi(argv[1]);

        if (num_threads <= 0 || num_threads > max_threads) {
            std::cerr << "ERRORE: Numero di thread non valido!\n";
            std::cerr << "Richiesto: " << num_threads << "\n";
            std::cerr << "Massimo disponibile sul sistema: " << max_threads << "\n";
            std::cerr << "Uso: " << argv[0] << " [num_threads]\n";
            return 1;
        }
    } else {
        // Modalità interattiva - richiedi input all'utente
        std::cout << "=================================================\n";
        std::cout << "  Password Decryption - Brute Force Parallel\n";
        std::cout << "=================================================\n";
        std::cout << "Massimo numero di thread disponibili: " << max_threads << "\n";
        std::cout << "Inserisci il numero di thread da utilizzare (1-" << max_threads << "): ";

        std::cin >> num_threads;

        // Validazione input
        if (std::cin.fail() || num_threads <= 0 || num_threads > max_threads) {
            std::cerr << "\nERRORE: Numero di thread non valido!\n";
            std::cerr << "Deve essere un numero tra 1 e " << max_threads << "\n";
            return 1;
        }

        std::cout << "\n";
    }

    std::cout << "Utilizzo " << num_threads << " thread su " << max_threads << " disponibili\n";
    std::cout << "Avvio elaborazione...\n\n";

    omp_set_num_threads(num_threads);

    const std::string salt = "AB";  // DES salt: primi 2 char
    std::string found;
    std::atomic<bool> found_flag(false);
    bool correct=true;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    const auto start = std::chrono::high_resolution_clock::now();
    ////////////////////////////////////////////////////
    ///                 INIZIO ITERAZIONI            ///
    ////////////////////////////////////////////////////
    constexpr int NUM_ITER=100;

    std::cout << "Progresso elaborazione:\n";

    for (int i=0;i<NUM_ITER;i++) {
        found=""; // IMPORTANT reset found per loop successivo

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
#pragma omp parallel default(none) shared(salt,found,gen,target,found_flag)
    {
        //printf("Thread %d in esecuzione\n", omp_get_thread_num());
        struct crypt_data data{};
        data.initialized = 0;


#pragma omp for collapse(3) schedule(dynamic)
        for (int a = 0; a <= 31; ++a) {
            for (int b = 0; b <= 12; ++b) {
                for (int c = 0; c <= 2025; ++c) {
                    // Early stop: controlla se un altro thread ha trovato la password
                    if (found_flag.load(std::memory_order_acquire)) {
                        continue; // Salta le iterazioni rimanenti
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
        found_flag.store(false, std::memory_order_release);  // IMPORTANT set flag to true
        if (found.empty()) {
            correct=false;
        }
        // Aggiorna la barra di progresso
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_so_far = current_time - start;
        //printProgressBar(i + 1, NUM_ITER, elapsed_so_far.count());
}

    std::cout << "\n\n";  // Nuova riga dopo la barra di progresso

    ////////////////////////////////////////////////////
    ///                 PRINT DI CONTROLLO           ///
    ////////////////////////////////////////////////////

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "========================================\n";
    std::cout << "Elaborazione completata!\n";
    std::cout << "========================================\n";
    std::cout << "Tempo totale impiegato: " << elapsed.count() << " secondi\n";
    std::cout << "Tempo medio per iterazione: " << (elapsed.count()/NUM_ITER) << " secondi\n";
    std::cout << "Iterazioni totali: " << NUM_ITER << "\n";
    std::cout << "Thread utilizzati: " << num_threads << "\n";
    std::cout << "========================================\n";
    if (!found.empty()) {
        printf("✓ Password trovata: %s\n", found.c_str());
    } else {
        printf("✗ Password non trovata\n");
    }
    std::cout << "========================================\n";
    if (correct) {
        std::cout << "✓ Tutte le password generate sono corrette\n";
    } else {
        std::cout << "✗ Alcune password generate non sono corrette\n";
    }

    return 0;
}

//media=0.7679
//media senza puntatore const è uguale
//media senza private è minore 0.75...
//fallimento totale
//dynamic velocizza  0.74
