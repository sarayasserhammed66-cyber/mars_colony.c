#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int health;
    int energy;
    int oxygen;
    int water;
    int spareParts;
    int moduleDamage;
    int airlockSealed;
    int day;
    int turns;
    int difficulty;
    int running;
    int won;
} Game;

static int clamp(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static void normalize(Game* g) {
    g->health = clamp(g->health, 0, 100);
    g->energy = clamp(g->energy, 0, 100);
    g->oxygen = clamp(g->oxygen, 0, 100);
    g->moduleDamage = clamp(g->moduleDamage, 0, 100);
    g->water = g->water < 0 ? 0 : g->water;
    g->spareParts = g->spareParts < 0 ? 0 : g->spareParts;
}

static int read_int(const char* prompt, int* value) {
    char line[128];
    char* end;
    long parsed;

    printf("%s", prompt);
    if (!fgets(line, sizeof line, stdin)) return 0;

    parsed = strtol(line, &end, 10);

    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') {
        end++;
    }

    if (end == line || *end != '\0') return 0;

    *value = (int)parsed;
    return 1;
}

static char read_command(void) {
    char line[128];

    printf("Komutunuz: ");

    if (!fgets(line, sizeof line, stdin)) {
        return 'Q';
    }

    return (char)toupper((unsigned char)line[0]);
}

static void show_status(const Game* g) {
    printf("\n--- DURUM RAPORU ---\n");
    printf("Gun: %d/15 | Tur: %d\n", g->day, g->turns);
    printf("Saglik: %d | Enerji: %d | Oksijen: %d\n",
        g->health, g->energy, g->oxygen);
    printf("Su: %d | Yedek parca: %d | Modul hasari: %d\n",
        g->water, g->spareParts, g->moduleDamage);
    printf("Hava kilidi: %s\n",
        g->airlockSealed ? "Muhurlu" : "Acik");

    if (g->oxygen < 25) {
        puts("UYARI: Oksijen kritik seviyede!");
    }

    if (g->health < 30) {
        puts("UYARI: Saglik kritik seviyede!");
    }

    if (g->moduleDamage > 70) {
        puts("UYARI: Modul ciddi hasarli!");
    }

    if (g->health >= 30 && g->oxygen >= 25 && g->moduleDamage <= 70) {
        puts("TUM SISTEMLER NORMAL");
    }

    puts("--------------------\n");
}

static void advance_turn(Game* g) {
    g->turns++;

    if (g->turns % 3 == 0) {
        g->day++;
        g->oxygen -= 8;

        printf("\nYeni gun: %d. Gun baslangicinda oksijen 8 azaldi.\n",
            g->day);

        if (g->moduleDamage > 70) {
            g->health -= 5;
            puts("Yuksek modul hasari nedeniyle 5 saglik kaybettiniz.");
        }

        if (g->day >= 15) {
            g->won = 1;
            g->running = 0;
            puts("15. gune ulastiniz! Kurtarma ekibi Mars'a geldi.");
        }
    }

    normalize(g);
}

static void explore(Game* g) {
    int roll = rand() % 100;

    g->energy -= 15;
    g->oxygen -= 5;

    printf("Kesif zar: %d / Esik: 40 ve 70\n", roll);

    if (roll < 40) {
        g->water += 4;
        printf("Su buzu buldunuz: +4 su (IF: %d < 40).\n", roll);
    }
    else if (roll < 70) {
        g->spareParts++;
        printf("Yedek parca buldunuz: +1 (ELSE IF: %d < 70).\n", roll);
    }
    else if (roll >= 90 || g->moduleDamage > 70) {
        g->health -= 10;
        g->moduleDamage += 5;
        printf("Kesif tehlikeli cikti: -10 saglik, +5 hasar "
            "(IF: %d >= 90 veya hasar > 70).\n", roll);
    }
    else {
        printf("Kayda deger kaynak bulunamadi (ELSE: zar = %d).\n", roll);
    }
}

static void produce_oxygen(Game* g) {
    int requested;
    int usable;

    if (!read_int("Kullanilacak su miktari (birim): ", &requested)) {
        puts("Gecersiz miktar; komut iptal edildi.");
        return;
    }

    if (requested <= 0 || requested > g->water) {
        puts("Bu miktarda suyunuz yok; komut iptal edildi.");
        return;
    }

    usable = requested;

    if (usable % 2 != 0) {
        usable--;
        printf("Su miktari cift olmali; %d birim kullanilacak.\n", usable);
    }

    if (usable == 0) {
        puts("En az 2 birim su gerekli; komut iptal edildi.");
        return;
    }

    if (g->energy < 5) {
        puts("Yetersiz enerji; oksijen uretimi yapilamadi.");
        return;
    }

    {
        int oxygenMade = (usable / 2) * 3;
        int room = 100 - g->oxygen;
        int pairsToUse = room / 3;
        int actualPairs = usable / 2;

        if (actualPairs > pairsToUse) {
            actualPairs = pairsToUse;
        }

        if (actualPairs <= 0) {
            puts("Oksijen kapasitesinde 3 birimlik yer yok; "
                "su ve enerji harcanmadi.");
            return;
        }

        oxygenMade = actualPairs * 3;

        if (oxygenMade > room) {
            oxygenMade = room;
        }

        g->water -= actualPairs * 2;
        g->oxygen += oxygenMade;
        g->energy -= 5;

        printf("%d su harcandi, %d oksijen uretildi. "
            "(Her 2 su -> 3 oksijen)\n",
            actualPairs * 2, oxygenMade);
    }
}

static int repair_module(Game* g) {
    int repairAmount;

    if (g->spareParts <= 0) {
        puts("Yedek parcaniz olmadigi icin tamir yapilamadi.");
        return 0;
    }

    if (g->moduleDamage <= 0) {
        puts("Modul zaten hasarsiz.");
        return 0;
    }

    if (g->moduleDamage <= 30) {
        repairAmount = 10;
        printf("IF: Hasar %d (0-30, hafif).\n", g->moduleDamage);
    }
    else if (g->moduleDamage <= 70) {
        repairAmount = 20;
        printf("ELSE IF: Hasar %d (31-70, orta).\n", g->moduleDamage);
    }
    else {
        repairAmount = 30;
        printf("ELSE: Hasar %d (71-100, agir).\n", g->moduleDamage);
    }

    g->moduleDamage -= repairAmount;
    g->spareParts--;

    printf("Modul %d puan onarildi; 1 yedek parca kullanildi.\n",
        repairAmount);

    return 1;
}

static void rest(Game* g) {
    g->energy += 20;
    g->oxygen -= 5;

    if (g->oxygen < 20) {
        g->health -= 5;
        puts("Oksijen 20'nin altinda; dinlenirken saglik 5 azaldi.");
    }
    else {
        g->health += 8;
        puts("Dinlenme: enerji +20, saglik +8, oksijen -5.");
    }
}

static void sandstorm(Game* g) {
    int rounds = g->difficulty == 0 ? 3 :
        (g->difficulty == 1 ? 5 : 7);
    int i;

    printf("Kum firtinasi basladi: %d tur.\n", rounds);

    for (i = 1; i <= rounds && g->running; i++) {
        int roll = rand() % 100;
        int multiplier = (i % 2 == 0) ? 2 : 1;

        printf("Firtina turu %d, zar: %d / Esik: 50\n", i, roll);

        if (i % 2 == 0) {
            printf("Cift numarali tur: ruzgar ve hasar %d kat "
                "uygulanacak (tur %% 2 == 0).\n", multiplier);
        }

        if (g->airlockSealed) {
            printf("Hava kilidi muhurlu: oksijen sizintisi bu tur "
                "atlanacak.\n");
        }

        if (roll < 40) {
            if (g->airlockSealed) {
                g->health -= 3 * multiplier;
                g->moduleDamage += 3 * multiplier;
                printf("Hafif ruzgar hasari: saglik ve modul hasari "
                    "%d kat uygulandi.\n", multiplier);
            }
            else {
                g->health -= 5 * multiplier;
                g->moduleDamage += 3 * multiplier;
                g->oxygen -= 3 * multiplier;

                printf("IF (zar < 40): hafif hasar, saglik -%d, "
                    "oksijen -%d.\n",
                    5 * multiplier, 3 * multiplier);
            }
        }
        else if (roll < 80) {
            g->health -= 12 * multiplier;
            g->moduleDamage += 10 * multiplier;

            if (!g->airlockSealed) {
                g->oxygen -= 8 * multiplier;
            }

            printf("ELSE IF (zar < 80): siddetli ruzgar, saglik -%d, "
                "oksijen -%d.\n",
                12 * multiplier,
                g->airlockSealed ? 0 : 8 * multiplier);
        }
        else {
            puts("ELSE (zar >= 80): guvenli bolge bulundu, hasar alinmadi.");
        }

        normalize(g);
        advance_turn(g);

        if (g->health == 0 || g->oxygen == 0) {
            g->running = 0;
        }

        if (!g->running && (g->health == 0 || g->oxygen == 0)) {
            break;
        }
    }

    puts("Kum firtinasi sona erdi.");
}

static void seal_airlock(Game* g) {
    int attempts = 0;
    char expected = (char)('A' + (g->day % 26));
    char guess;
    char line[128];

    if (g->airlockSealed) {
        puts("Hava kilidi zaten muhurlu.");
        return;
    }

    puts("Ipucu: Sifre, alfabenin A harfinden bugunun gun sayisi "
        "kadar ileridedir.");

    do {
        printf("Hava kilidi sifresi (A-Z): ");

        if (!fgets(line, sizeof line, stdin)) {
            g->running = 0;
            return;
        }

        guess = (char)toupper((unsigned char)line[0]);
        attempts++;

        if (guess == expected) {
            g->airlockSealed = 1;
            puts("Sifre dogru. Hava kilidi muhurlendi.");
        }
        else {
            g->energy -= 5;

            printf("Yanlis deneme. Deneme %d/3. Enerji -5. ", attempts);

            if (guess < expected) {
                puts("Ipucu: girdiginiz harften ileri gidin.");
            }
            else {
                puts("Ipucu: girdiginiz harften geri gidin.");
            }
        }
    } while (attempts < 3 && guess != expected);

    if (guess != expected) {
        g->moduleDamage += 20;
        puts("3 deneme de basarisiz; modul hasari 20 artti.");
    }
}

static int choose_difficulty(Game* g) {
    int choice;

    for (;;) {
        puts("Zorluk secin: 1) Kolay  2) Normal  3) Zor");

        if (!read_int("Seciminiz: ", &choice)) {
            choice = 0;
        }

        switch (choice) {
        case 1:
            g->difficulty = 0;
            g->health = 100;
            g->energy = 100;
            g->oxygen = 90;
            g->water = 12;
            g->spareParts = 3;
            g->moduleDamage = 10;
            break;

        case 2:
            g->difficulty = 1;
            g->health = 85;
            g->energy = 80;
            g->oxygen = 75;
            g->water = 8;
            g->spareParts = 2;
            g->moduleDamage = 25;
            break;

        case 3:
            g->difficulty = 2;
            g->health = 70;
            g->energy = 65;
            g->oxygen = 60;
            g->water = 6;
            g->spareParts = 1;
            g->moduleDamage = 40;
            break;

        default:
            puts("Gecersiz seviye; lutfen tekrar secin.");
            continue;
        }

        break;
    }

    g->day = 1;
    g->turns = 0;
    g->airlockSealed = 0;
    g->running = 1;
    g->won = 0;

    return 1;
}

static void print_menu(void) {
    puts("\nKomutlar: G=Kesif | O=Oksijen | T=Tamir | D=Durum");
    puts("          U=Dinlen | K=Kum firtinasi | H=Hava kilidi | Q=Cikis");
}

static void print_final_report(const Game* g) {
    int score = (g->day * 100) + g->health + g->oxygen - g->moduleDamage;

    puts("\n========== OYUN SONU ==========");
    printf("Sonuc: %s\n",
        g->won ? "KAZANDINIZ - Kurtarma ekibi geldi" :
        "KAYBETTINIZ / Oyundan cikildi");
    printf("Hayatta kalinan gun: %d | Toplam tur: %d\n",
        g->day, g->turns);
    printf("Saglik: %d | Enerji: %d | Oksijen: %d\n",
        g->health, g->energy, g->oxygen);
    printf("Su: %d | Yedek parca: %d | Modul hasari: %d\n",
        g->water, g->spareParts, g->moduleDamage);
    printf("Puan = (gun x 100) + saglik + oksijen - modul hasari = %d\n",
        score);
    puts("===============================");
}

int main(void) {
    Game game = { 0 };

    srand((unsigned int)time(NULL));

    puts("MARS KOLONISI YASAM DESTEK SIMULATORU");
    puts("Amac: Saglik ve oksijeni sifirin ustunde tutarak 15. gune ulasmak.");

    choose_difficulty(&game);
    show_status(&game);

    while (game.running) {
        char command;

        print_menu();
        command = read_command();

        switch (command) {
        case 'G':
            if (game.energy < 15) {
                puts("Yetersiz enerji; kesif iptal edildi.");
            }
            else {
                explore(&game);
            }

            advance_turn(&game);
            break;

        case 'O':
            produce_oxygen(&game);
            advance_turn(&game);
            break;

        case 'T':
            if (game.energy < 5) {
                puts("Yetersiz enerji; tamir iptal edildi.");
            }
            else if (repair_module(&game)) {
                game.energy -= 5;
            }

            advance_turn(&game);
            break;

        case 'D':
            show_status(&game);
            break;

        case 'U':
            rest(&game);
            advance_turn(&game);
            break;

        case 'K':
            if (game.energy < 20) {
                puts("Yetersiz enerji; firtina gorevi iptal edildi.");
                advance_turn(&game);
            }
            else {
                game.energy -= 20;
                sandstorm(&game);
            }
            break;

        case 'H':
            if (game.energy < 5) {
                puts("Yetersiz enerji; sifre denemesi iptal edildi.");
                advance_turn(&game);
            }
            else {
                seal_airlock(&game);
                advance_turn(&game);
            }
            break;

        case 'Q':
            game.running = 0;
            break;

        default:
            puts("Komut taninmadi. G, O, T, D, U, K, H veya Q girin. "
                "Tur harcanmadi.");
            break;
        }

        normalize(&game);

        if (game.health == 0 || game.oxygen == 0) {
            game.running = 0;
            game.won = 0;
            puts("Saglik veya oksijen sifira dustu. Gorev basarisiz.");
        }
    }

    print_final_report(&game);
    return 0;
}