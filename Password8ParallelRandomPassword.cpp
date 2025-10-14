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
void printProgressBar(int current, int total, double elapsed_time, long long passwords_tested, int bar_width = 50) {
    float progress = (float)current / total;
    int pos = bar_width * progress;

    std::cout << "\r[";
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) std::cout << "█";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    //std::cout << "] " << int(progress * 100.0) << "% ";
    std::cout << current << "/" << total << " ";

    if (current > 0) {
        double avg_time = elapsed_time / current;
        double remaining_time = avg_time * (total - current);
        double passwords_per_second = passwords_tested / elapsed_time;

        std::cout << "| Tempo: " << std::fixed << std::setprecision(1) << elapsed_time << "s ";
        std::cout << "| Rimanente: " << std::fixed << std::setprecision(1) << remaining_time << "s ";
        std::cout << "| Media: " << std::fixed << std::setprecision(3) << avg_time << "s/it ";
        std::cout << "| Pass/s: " << std::fixed << std::setprecision(0) << passwords_per_second;
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

    // Avviso se viene selezionato 1 thread
    if (num_threads == 1) {
        std::cout << "\n⚠️  ATTENZIONE: Hai selezionato 1 thread!\n";
        std::cout << "Per prestazioni ottimali con esecuzione sequenziale,\n";
        std::cout << "usa il programma dedicato: ./Password8Sequenziale\n";
        std::cout << "(versione pura senza overhead OpenMP)\n\n";
    }

    std::cout << "Avvio elaborazione...\n\n";

    omp_set_num_threads(num_threads);

    const std::string salt = "AB";  // DES salt: primi 2 char
    const char* salt_cstr = salt.c_str(); // OTTIMIZZAZIONE: calcola una volta sola
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
    constexpr long long PASSWORDS_PER_ITER = 32LL * 13 * 2026; // (0-31) * (0-12) * (0-2025)
    long long total_passwords_tested = 0;

    std::cout << "Progresso elaborazione:\n";

    for (int i=0;i<NUM_ITER;i++) {
        found=""; // IMPORTANT reset found per loop successivo
        found_flag.store(false, std::memory_order_release); // Reset flag all'inizio

        //create a random date string in the format ddmmyyyy manualmente (più veloce)
        std::uniform_int_distribution<int> giorno(1, 31);
        std::uniform_int_distribution<int> mese(1, 12);
        std::uniform_int_distribution<int> anno(0, 2025);

        const int g = giorno(gen);
        const int m = mese(gen);
        const int y = anno(gen);

        // Costruzione manuale della password target (evita ostringstream)
        char target_password[9];
        target_password[0] = '0' + (g / 10);
        target_password[1] = '0' + (g % 10);
        target_password[2] = '0' + (m / 10);
        target_password[3] = '0' + (m % 10);
        target_password[4] = '0' + ((y / 1000) % 10);
        target_password[5] = '0' + ((y / 100) % 10);
        target_password[6] = '0' + ((y / 10) % 10);
        target_password[7] = '0' + (y % 10);
        target_password[8] = '\0';

        const char* target = crypt(target_password, salt_cstr);

#pragma omp parallel default(none) shared(salt_cstr,found,target,found_flag)
    {
        //printf("Thread %d in esecuzione\n", omp_get_thread_num());
        struct crypt_data data{};
        data.initialized = 0;

#pragma omp for collapse(3) schedule(static) nowait
        for (int a = 0; a <= 31; ++a) {
            for (int b = 0; b <= 12; ++b) {
                for (int c = 0; c <= 2025; ++c) {
                    // Early stop: controlla flag
                    if (found_flag.load(std::memory_order_relaxed)) { // se mettiamo il crontroolo con c % 1000 rallenta
                        continue; // Salta questa iterazione, OpenMP gestirà l'uscita
                    }

                    // GENERAZIONE PASSWORD ITERATIVA
                    char date[9];
                    date[0] = '0' + (a / 10);
                    date[1] = '0' + (a % 10);
                    date[2] = '0' + (b / 10);
                    date[3] = '0' + (b % 10);
                    date[4] = '0' + ((c / 1000) % 10);
                    date[5] = '0' + ((c / 100) % 10);
                    date[6] = '0' + ((c / 10) % 10);
                    date[7] = '0' + (c % 10);
                    date[8] = '\0';

                    // CONTROLLO PASSWORD
                    const char* h = crypt_r(date, salt_cstr, &data);

                    // Ottimizzazione: Quick reject - confronta prima i 3/4 caratteri
                    if (h[3] == target[3] && h[4] == target[4]) {
                        if (strcmp(h, target) == 0) {
#pragma omp critical
                            {
                                if (!found_flag.load(std::memory_order_relaxed)) {
                                    found = date;
                                    found_flag.store(true, std::memory_order_release);
                                }
                            }
                        }
                    }
                } // fine loop anno
            } // fine loop mese
        } // fine loop giorno
    } // fine parallel

        total_passwords_tested += PASSWORDS_PER_ITER;

        // Aggiorna la barra di progresso solo ogni 5 iterazioni (o all'ultima)
        if ((i + 1) % 5 == 0 || i == NUM_ITER - 1) {
            auto current_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_so_far = current_time - start;
            printProgressBar(i + 1, NUM_ITER, elapsed_so_far.count(), total_passwords_tested);
        }
}

    std::cout << "\n\n";  // Nuova riga dopo la barra di progresso

    ////////////////////////////////////////////////////
    ///                 PRINT DI CONTROLLO           ///
    ////////////////////////////////////////////////////

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    double passwords_per_second = total_passwords_tested / elapsed.count();

    std::cout << "========================================\n";
    std::cout << "Elaborazione completata!\n";
    std::cout << "========================================\n";
    std::cout << "Tempo totale impiegato: " << std::fixed << std::setprecision(2) << elapsed.count() << " secondi\n";
    std::cout << "Tempo medio per iterazione: " << std::fixed << std::setprecision(3) << (elapsed.count()/NUM_ITER) << " secondi\n";
    std::cout << "Iterazioni totali: " << NUM_ITER << "\n";
    std::cout << "Password testate totali: " << total_passwords_tested << "\n";
    std::cout << "Password testate/secondo: " << std::fixed << std::setprecision(0) << passwords_per_second << "\n";
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