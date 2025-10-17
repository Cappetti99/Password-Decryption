#include <chrono>
#include <crypt.h>
#include <iostream>
#include <string>
#include <cstring>
#include <iomanip>
#include <random>



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


int main() {
    ////////////////////////////////////////////////////
    ///                 INIZIALIZZAZIONE             ///
    ////////////////////////////////////////////////////

    std::cout << "=================================================\n";
    std::cout << "  Password Decryption - Brute Force Sequenziale\n";
    std::cout << "=================================================\n";
    std::cout << "Versione ottimizzata per esecuzione single-thread\n";
    std::cout << "Avvio elaborazione...\n\n";

    const std::string salt = "AB";  // DES salt: primi 2 char
    const char* salt_cstr = salt.c_str();
    std::string found;
    bool found_flag = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    const auto start = std::chrono::high_resolution_clock::now();

    ////////////////////////////////////////////////////
    ///                 INIZIO ITERAZIONI            ///
    ////////////////////////////////////////////////////
    constexpr int NUM_ITER = 100;
    constexpr long long PASSWORDS_PER_ITER = 32LL * 13 * 2026; // (0-31) * (0-12) * (0-2025)
    long long total_passwords_tested = 0;
    int correct_matches = 0;
    int incorrect_matches = 0;

    std::cout << "Progresso elaborazione:\n";

    for (int i = 0; i < NUM_ITER; i++) {
        found = "";
        found_flag = false;

        // Crea password random nel formato ddmmyyyy
        std::uniform_int_distribution<int> giorno(1, 31);
        std::uniform_int_distribution<int> mese(1, 12);
        std::uniform_int_distribution<int> anno(0, 2025);

        const int g = giorno(gen);
        const int m = mese(gen);
        const int y = anno(gen);

        // Costruzione manuale della password target
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
        const std::string target_string = target;

        // Loop sequenziale di brute force
        for (int a = 0; a <= 31 && !found_flag; ++a) {
            for (int b = 0; b <= 12 && !found_flag; ++b) {
                for (int c = 0; c <= 2025; ++c) {
                    // Early stop: se trovato, esci
                    if (found_flag) {
                        break;
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
                    const char* h = crypt(date, salt_cstr);

                    // Ottimizzazione: Quick reject - confronta prima i primi 2 caratteri
                    if (h[2] == target[2] && h[3] == target[3]) {
                        if (strcmp(h, target_string.c_str()) == 0) {
                            found = date;
                            found_flag = true;
                            break;
                        }
                    }
                } // fine loop anno
            } // fine loop mese
        } // fine loop giorno

        total_passwords_tested += PASSWORDS_PER_ITER;

        // Verifica correttezza: controlla se la password trovata è quella target
        if (!found.empty()) {
            if (strcmp(found.c_str(), target_password) == 0) {
                correct_matches++;
            } else {
                incorrect_matches++;
            }
        }

        // Aggiorna la barra di progresso ogni 5 iterazioni (o all'ultima)
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
    std::cout << "Thread utilizzati: 1 (sequenziale)\n";
    std::cout << "========================================\n";
    std::cout << "VERIFICA CORRETTEZZA:\n";
    std::cout << "Password corrette trovate: " << correct_matches << "/" << NUM_ITER << "\n";
    std::cout << "Password errate trovate: " << incorrect_matches << "/" << NUM_ITER << "\n";

    if (correct_matches == NUM_ITER) {
        std::cout << "✓ SUCCESSO: Tutte le password sono state trovate correttamente!\n";
    } else {
        std::cout << "✗ ERRORE: Alcune password non sono state trovate o sono errate!\n";
    }
    std::cout << "========================================\n";
    if (!found.empty()) {
        printf("✓ Ultima password trovata: %s\n", found.c_str());
    } else {
        printf("✗ Ultima password non trovata\n");
    }
    std::cout << "========================================\n";

    return 0;
}