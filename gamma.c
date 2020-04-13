/** @file
 * Implementacja klasy przechowującej stan gry gamma
 *
 * @author Szymon Czyżmański 417797
 * @date 11.04.2020
 */

#include <stdlib.h>

#include "gamma.h"
#include "field.h"
#include "mem_alloc_check.h"

/**
 * Maksymalna liczba pól, z jakimi pole może sąsiadować.
 */
#define MAX_NEIGHBOURS 4

/**
 * Struktura przechowująca stan gry.
 */
struct gamma {
    uint32_t width;         /**< Szerokość planszy, liczba dodatnia równa wartości
                             *   @p width z funkcji @ref gamma_new. */
    uint32_t height;        /**< Wysokość planszy, liczba dodatnia równa wartości
                             *   @p height z funkcji @ref gamma_new. */
    uint32_t players;       /**< Liczba graczy, liczba dodatnia równa wartości
                             *   @p players z funkcji @ref gamma_new. */
    uint32_t areas;         /**< Maksymalna liczba obszarów, jakie może posiadać
                             *   gracz, liczba dodatnia równa wartości @p areas
                             *   z funkcji @ref gamma_new. */
    uint32_t busy_fields;   /**< Liczba wszystkich zajętych pól na planszy. */
    field_t ***board;       /**< Wskaźnik do tablicy o @p height wierszach
                             *   i @p width kolumnach reprezentującej planszę
                             *   na której rozgrywana jest gra, przechowującej
                             *   w wierszu @p y i kolumnie @p x wskaźnik do
                             *   struktury przechowującej stan pola (@p x, @p y)
                             *   lub NULL, jeśli pole to nie zostało zajęte przez
                             *   żadnego z graczy. */
    player_t **players_arr; /**< Tablica wskaźników do struktur przechowujących
                             *   stan graczy biorących udział w rozgrywce,
                             *   o długości równej wartości o 1 większej niż wartość
                             *   @p players z funkcji @ref gamma_new.
                             *   Wartość wskaźnika @p players_arr[0] jest równa NULL,
                             *   ponieważ nie ma gracza o numerze 0.
                             *   Pod indeksem i, dla i dodatniego oraz niewiększego
                             *   od wartości @p players z funkcji @ref gamma_new,
                             *   znajduje się adres struktury przechowującej stan
                             *   gracza o numerze i lub NULL, jeśli gracz ten jeszcze
                             *   nie postawił żadnego pionka na którymś polu. */
};

/** @name Obszar
 * Wykorzystanie struktury Find-Union z kompresją ścieżki oraz łączeniem według
 * rangi do efektywnego utrzymania informacji o obszarach zajętych przez gracza.
 */
///@{

/** @brief Tworzy nowy obszar.
 * Czyni pole wskazywane przez @p f korzeniem nowego obszaru: ustawia składową
 * @p rank pola wskazywanego przez @p f na 0 raz ustawia składową @p root pola
 * wskazywanego przez @p f na NULL. Zwiększa o 1 wartość składowej @p areas
 * gracza wskazywanego przez @p owner będącego właścicielem wyżej wspomnianego
 * pola.
 * @param[in,out] f – wskaźnik na strukturę przechowującą stan pola.
 */
static void area_new(field_t *f) {
    field_set_rank(f, 0);
    field_set_parent(f, NULL);

    player_t *owner = field_owner(f);
    player_set_areas(owner, player_areas(owner) + 1);
}

/** @brief Znajduje korzeń obszaru.
 * Rekurencyjnie znajduje korzeń @p root obszaru do którego należy pole
 * wskazywane przez @p f.
 * Dokonuje kompresji ścieżki od @p f do @p root, ustawiając składową @p parent
 * w każdym polu na tej ścieżce na @p root, z wyjątkiem samego korzenia @p root.
 * @param[in,out] f – wskaźnik na strukturę przechowującą stan pola.
 * @return Wskaźnik na strukturę przechowującą stan pola będącego korzeniem
 * obszaru do którego należy pole wskazywane przez @p f.
 */
static field_t *area_find_root(field_t *f) {
    if (field_parent(f) == NULL) {
        return f;
    }
    else {
        field_t *root = area_find_root(field_parent(f));
        field_set_parent(f, root);
        return root;
    }
}

/** @brief Łączy dwa obszary w jeden według rangi.
 * Ustawia wskaźnik @p parent pola wskazywanego przez @p f1_root, będącego korzeniem
 * pierwszego obszaru, do którego należy pole wskazywane przez @p f1, na pole
 * wskazywane przez @p f2_root, będące korzeniem drugiego obszaru, do którego należy
 * pole wskazywane przez @p f2, jeśli ranga @p f1_root_rank korzenia pierwszego
 * obszaru jest mniejsza od rangi @p f2_root_rank korzenia drugiego obszaru.
 * W przeciwnym razie, ustawia wskaźnik @p parent pola wskazywanego przez @p f2_root
 * na @p f1_root. Jeśli wartość @p f1_root_rank jest równa wartości @p f2_root_rank,
 * zwiększa rangę korzenia pierwszego obszaru, dodając 1 do wartości składowej
 * @p rank pola wskazywanego przez @p f1_root.
 * @param[in,out] f1 – wskaźnik na strukturę przechowującą stan pola
 *                     należącego do pierwszego obszaru,
 * @param[in,out] f2 – wskaźnik na strukturę przechowującą stan pola
 *                     należącego do drugiego obszaru.
 * @return Wartość @p false, gdy pola wskazywane przez @p f1 oraz @p f2 należą do
 * tego samego obszaru i nie wykonano połączenia, a @p true w przeciwnym przypadku.
 */
static bool area_merge(field_t *f1, field_t *f2) {
    field_t *f1_root = area_find_root(f1);
    field_t *f2_root = area_find_root(f2);

    uint32_t f1_root_rank = field_rank(f1_root);
    uint32_t f2_root_rank = field_rank(f2_root);

    if (f1_root == f2_root) {
        return false;
    }
    else {
        if (f1_root_rank < f2_root_rank) {
            field_set_parent(f1_root, f2_root);
        }
        else {
            field_set_parent(f2_root, f1_root);
            if (f1_root_rank == f2_root_rank) {
                field_set_rank(f1_root, f1_root_rank + 1);
            }
        }
        return true;
    }
}

///@}

/** @name Pole
 * Sprawdzanie czy pole lub powiązany z nim element spełnia określony predykat.
 * Zliczanie pól spełniających określony predykat.
 * Polem poprawnym jest pole, którego współrzędne są poprawne.
 */
///@{

/** @brief Sprawdza poprawność numeru kolumny.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli numer kolumny jest poprawny,
 * a @p false w przeciwnym przypadku.
 */
static inline bool valid_x(gamma_t *g, int64_t x) {
    return 0 <= x && x < g->width;
}

/** @brief Sprawdza poprawność numeru wiersza.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli numer wiersza jest poprawny,
 * a @p false w przeciwnym przypadku.
 */
static inline bool valid_y(gamma_t *g, int64_t y) {
    return 0 <= y && y < g->height;
}

/** @brief Sprawdza, czy pole (@p x, @p y) jest poprawne i wolne.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * Sprawdza, czy pole nie zostało jeszcze stworzone, tzn. czy wartość wskaźnika
 * @p g->board[y][x] jest równa NULL.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli współrzędne pola są poprawne oraz pole
 * nie jest zajęte przez pewnego gracza, a @p false w przeciwnym przypadku.
 */
static inline bool valid_free_field(gamma_t *g, int64_t x, int64_t y) {
    return valid_x(g, x) && valid_y(g, y)
           && (g->board[y][x] == NULL || field_owner(g->board[y][x]) == NULL);
}

/** @brief Sprawdza, czy pole (@p x, @p y) jest poprawne i zajęte.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * Sprawdza, czy pole zostało już stworzone, tzn. czy wartość wskaźnika
 * @p g->board[y][x] jest różna od NULL.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli współrzędne pola są poprawne oraz pole
 * jest zajęte przez pewnego gracza, a @p false w przeciwnym przypadku.
 */
static inline bool valid_busy_field(gamma_t *g, int64_t x, int64_t y) {
    return valid_x(g, x) && valid_y(g, y)
           && g->board[y][x] != NULL && field_owner(g->board[y][x]) != NULL;
}

/** @brief Sprawdza, czy pole (@p x, @p y) jest poprawne i należy do gracza @p p.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * Sprawdza, czy pole zostało już stworzone, tzn. czy wartość wskaźnika
 * @p g->board[y][x] jest różna od NULL oraz czy właścicielem pola jest gracz @p p,
 * tzn. czy wskaźnik @p owner będący składową pola na które wskazuje wskaźnik
 * @p g->board[y][x] jest równy @p p.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli współrzędne pola są poprawne oraz pole
 * należy do gracza @p p.
 */
static inline bool player_valid_field(gamma_t *g, player_t *p,
                                      int64_t x, int64_t y) {
    return valid_x(g, x) && valid_y(g, y)
           && g->board[y][x] != NULL && field_owner(g->board[y][x]) == p;
}

/** @brief Zlicza sąsiednie pola zajęte przez danego gracza.
 * Zlicza pola sąsiadujące z polem (@p x, @p y), które zostały zajęte przez gracza
 * wskazywanego przez @p p.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new,
 * @return Liczba pól sąsiadujących z polem (@p x, @p y), które zostały zajęte
 * przez gracza wskazywanego przez @p p.
 */
static uint8_t player_adjacent_fields(gamma_t *g, player_t *p,
                                      uint32_t x, uint32_t y) {
    uint8_t fields = 0;

    fields += player_valid_field(g, p, x - 1, y);
    fields += player_valid_field(g, p, x + 1, y);
    fields += player_valid_field(g, p, x, y - 1);
    fields += player_valid_field(g, p, x, y + 1);

    return fields;
}

/** @brief Sprawdza, czy pole (@p x, @p y) jest poprawne, wolne i nie sąsiaduje
 * z żadnym polem zajętym przez gracza wskazywanego przez @p owner.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * Sprawdza, czy pole nie zostało jeszcze stworzone, tzn. czy wartość wskaźnika
 * @p g->board[y][x] jest równa NULL, lub, w przeciwnym razie, czy pole nie ma
 * przypisanego właściciela, tzn. czy wskaźnik @p owner będący składową pola
 * na które wskazuje @p g->board[y][x] jest równy NULL.
 * Sprawdza, czy liczba pól sąsiadujących z polem (@p x, @p y), zajętych przez
 * gracza wskazywanego przez @p owner jest równa 0.
 * @param[in] g     – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] owner – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] x     – numer kolumny, liczba nieujemna mniejsza od wartości
 *                    @p width z funkcji @ref gamma_new,
 * @param[in] y     – numer wiersza, liczba nieujemna mniejsza od wartości
 *                    @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jezeli pole (@p x, @p y) jest poprawne, wolne
 * i nie sąsiaduje z żadnym polem zajętym przez gracza wskazywanego przez @p owner,
 * a @p false w przeciwnym przypadku.
 */
static inline bool player_valid_free_single_field(gamma_t *g, player_t *owner,
                                                  int64_t x, int64_t y) {
    return valid_free_field(g, x, y) && player_adjacent_fields(g, owner, x, y) == 0;
}

/** @brief Zlicza sąsiednie, wolne pola, nie sąsiadujące z żadnym polem zajętym
 * przez danego gracza.
 * Zlicza pola sąsiadujące z polem (@p x, @p y), które nie zostały zajęte przez
 * żadnego gracza i które nie sąsiadują z żadnym polem zajętym przez gracza
 * wskazywanego przez @p owner.
 * @param[in] g     – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] owner – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] x     – numer kolumny, liczba nieujemna mniejsza od wartości
 *                    @p width z funkcji @ref gamma_new,
 * @param[in] y     – numer wiersza, liczba nieujemna mniejsza od wartości
 *                    @p height z funkcji @ref gamma_new.
 * @return Liczba pól sąsiadujących z polem (@p x, @p y), które nie zostały zajęte
 * przez żadnego gracza i które nie sąsiadują z żadnym polem zajętym przez gracza
 * wskazywanego przez @p owner.
 */
static uint8_t player_adjacent_free_single_fields(gamma_t *g, player_t *owner,
                                                  uint32_t x, uint32_t y) {
    uint8_t fields = 0;

    fields += player_valid_free_single_field(g, owner, x - 1, y);
    fields += player_valid_free_single_field(g, owner, x + 1, y);
    fields += player_valid_free_single_field(g, owner, x, y - 1);
    fields += player_valid_free_single_field(g, owner, x, y + 1);

    return fields;
}

///@}

/** @name Gracz
 * Sprawdzanie czy gracz lub związany z nim element spełnia określony predykat.
 * Zmiana stanu gracza lub związanych z nim elementów przy wykonywaniu ruchów
 * przez tego gracza podczas zwykłego ruchu lub innego gracza podczas złotego ruchu.
 */
///@{

/** @brief Sprawdza poprawność numeru gracza.
 * Sprawdza, czy numer gracza @p player jest liczbą całkowitą dodatnią
 * nie większą od wartości @p players z funkcji @ref gamma_new.
 * @param[in] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player – numer gracza, liczba dodatnia niewiększa od wartości
 *                     @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli numer gracza jest poprawny,
 * a @p false w przeciwnym przypadku.
 */
static inline bool valid_player(gamma_t *g, int64_t player) {
    return 1 <= player && player <= g->players;
}

/** Dodaje gracza, jeśli ten nie postawił jeszcze żadnego pionka.
 * Jeśli struktura przechowująca stan gracza o numerze @p player nie została
 * jeszcze stworzona, ponieważ gracz ten nie postawił jeszcze żadnego pionka,
 * alokuje pamięć na tę strukturę i zapisuje jej adres w pamięci w tablicy
 * @p players_arr pod indeksem @p player.
 * Nic nie robi, jeśli taka struktura została już stworzona.
 * @param[in,out] g  – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player – numer gracza, liczba dodatnia niewiększa od wartości
 *                     @p players z funkcji @ref gamma_new.
 */
static inline void gamma_add_player(gamma_t *g, uint32_t player) {
    if (g->players_arr[player] == NULL) {
        g->players_arr[player] = player_new(player);
        check_for_successful_alloc(g->players_arr[player]);
    }
}

/** @brief Aktualizuje obwód gracza po wykonaniu przez niego ruchu.
 * Aktualizuje obwód gracza będącego właścicielem pola wskazywanego przez @p f,
 * po wykonaniu przez niego ruchu na pole wskazywane przez @p f.
 * @param[in] g           – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] f           – wskaźnik na strukturę przechowującą stan pola właśnie
 *                          zajętego przez gracza,
 * @param[in] golden_move – wartość @p true, jeżeli funkcja została wywołana
 *                          w wyniku wykonania złotego ruchu przez gracza,
 *                          a @p false, jeżeli funkcja została wywołana w wyniku
 *                          wykonania zwykłego ruchu przez gracza.
 */
static void player_update_perimeter(gamma_t *g, field_t *f, bool golden_move) {
    uint32_t x = field_x(f);
    uint32_t y = field_y(f);
    player_t *owner = field_owner(f);
    uint32_t perimeter = player_perimeter(owner);

    field_set_owner(f, NULL);

    if (!golden_move && player_adjacent_fields(g, owner, x, y) > 0) {
        perimeter--;
    }

    perimeter += player_valid_free_single_field(g, owner, x - 1, y);
    perimeter += player_valid_free_single_field(g, owner, x + 1, y);
    perimeter += player_valid_free_single_field(g, owner, x, y - 1);
    perimeter += player_valid_free_single_field(g, owner, x, y + 1);

    field_set_owner(f, owner);
    player_set_perimeter(owner, perimeter);
}

/** @brief Łączy obszary gracza po wykonaniu przez niego ruchu na pole
 * wskazywane przez @p f.
 * Łączy obszar gracza, będącego właścicielem pola wskazywanego przez @p f,
 * do którego należy pole wskazywane przez @p f, z obszarem tego gracza,
 * do którego należy pole (@p x, @p y).
 * Zmniejsza o 1 liczbę obszarów gracza, jeżeli pole (@p x, @p y) jest poprawne 
 * i należy do tego gracza oraz połączono obszary.
 * Nic nie robi, jeżeli pole (@p x, @p y) jest niepoprawne lub nie należy do gracza,
 * lub oba pola należą do tego samego obszaru.
 * @param[in] g           – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] f           – wskaźnik na strukturę przechowującą stan pola właśnie
 *                          zajętego przez gracza,
 * @param[in] golden_move – wartość @p true, jeżeli funkcja została wywołana
 *                          w wyniku wykonania złotego ruchu przez gracza,
 *                          a @p false, jeżeli funkcja została wywołana w wyniku
 *                          wykonania zwykłego ruchu przez gracza.
 */
static void player_merge_adjacent_areas(gamma_t *g, field_t *f,
                                        int64_t x, int64_t y) {
    player_t *owner = field_owner(f);

    if (player_valid_field(g, owner, x, y) && area_merge(f, g->board[y][x])) {
        player_set_areas(owner, player_areas(owner) - 1);
    }
}

/** @brief Modyfikuje obszary gracza po wykonaniu przez niego ruchu na pole
 * wskazywane przez @p f.
 * Tworzy nowy obszar składający się tylko z pola wskazywanego przez @p f,
 * po czym próbuje łączyć ten obszar z sąsiednimi obszarami należącymi do
 * właściciela tego pola, jeżeli takie istnieją.
 * @param[in] g           – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] f           – wskaźnik na strukturę przechowującą stan pola właśnie
 *                          zajętego przez gracza.
 */
static void player_modify_areas(gamma_t *g, field_t *f) {
    area_new(f);

    uint32_t x = field_x(f);
    uint32_t y = field_y(f);

    player_merge_adjacent_areas(g, f, x - 1, y);
    player_merge_adjacent_areas(g, f, x + 1, y);
    player_merge_adjacent_areas(g, f, x, y - 1);
    player_merge_adjacent_areas(g, f, x, y + 1);
}

///@}

/** @name Sąsiedzi
 * Tworzenie zbioru graczy @p neighbours posiadających co najmniej jedno pole
 * sąsiadujące z polem, na który stawiany jest pionek.
 * Aktualizacja obwodów graczy należących do utworzonego zbioru reprezentowanego
 * przez tablicę @p neighbours z funkcji @ref unique_neighbours i przekazywaną
 * do pozostałych funkcji z tej grupy.
 */
///@{

/** @brief Sprawdza, czy gracz nie został jeszcze dodany do zbioru @p neighbours.
 * Sprawdza, czy adres struktury gracza wskazywanej przez @p p nie został jeszcze
 * dodany do tablicy @p neighbours, przechowującej niepowtarzające się adresy
 * struktur graczy zajmujących co najmniej jedno pole w sąsiedztwie pewnego pola,
 * na które został postawiony pionek.
 * @param[in] p          – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] neighbours – tablica wskaźników na dodanych już graczy,
 * @param[in] added      – liczba dodanych już graczy.
 * @return Wartość @p true, jeśli wskaźnik @p p nie został jeszcze dodany
 * do tablicy @p neighbours, a @p false w przeciwnym przypadku.
 */
static bool unique_neighbour(player_t *p, player_t **neighbours, uint8_t added) {
    uint8_t i = 0;

    while (i < added && p != neighbours[i]) {
        i++;
    }

    return i == added;
}

/** @brief Sprawdza, czy pole (@p x, @p y) jest zajęte i należy do gracza, który
 * nie został jeszcze dodany do zbioru @p neighbours.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * Sprawdza, czy pole zostało już stworzone, tzn. czy wartość wskaźnika
 * @p g->board[y][x] jest różna od NULL.
 * Sprawdza, czy wskaźnik na gracza będącego właścicielem pola (@p x, @p y)
 * nie został jeszcze dodany do tablicy @p neighbours.
 * @param[in] g          – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                         @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new,
 * @param[in] neighbours – tablica wskaźników na dodanych już graczy,
 * @param[in] added      – liczba dodanych już graczy.
 * @return Wartość @p true, jeśli pole (@p x, @p y) jest poprawne oraz jest zajęte
 * przez gracza, którego adres nie został jeszcze dodany do tablicy @p neighbours,
 * a @p false w przeciwnym przypadku.
 */
static bool neighbour_valid_unique_field(gamma_t *g, int64_t x, int64_t y,
                                         player_t **neighbours, uint8_t added) {
    if (!valid_x(g, x) || !valid_y(g, y)) {
        return false;
    }
    else {
        field_t *f = g->board[y][x];
        return f != NULL && unique_neighbour(field_owner(f), neighbours, added);
    }
}

/** @brief Dodaje adres gracz posiadającego pionek na polu (@p x, @p y), jeżeli
 * nie został on jeszcze dodany do zbioru @p neighbours.
 * Sprawdza, czy pole (@p x, @p y) jest poprawne i zajęte oraz należy do gracza,
 * który nie został jeszcze dodany do zbioru @p neighbours. Jeżeli tak,
 * to dodaje adres gracza posiadającego pionek na polu (@p x, @p y) do tablicy
 * @p neighbours i zwiększa liczbę @p added dodanych adresów do tej tablicy o 1.
 * Zwraca wartość zmiennej @p added.
 * @param[in] g              – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x              – numer kolumny, liczba nieujemna mniejsza od wartości
 *                             @p width z funkcji @ref gamma_new,
 * @param[in] y              – numer wiersza, liczba nieujemna mniejsza od wartości
 *                             @p height z funkcji @ref gamma_new,
 * @param[in,out] neighbours – tablica wskaźników na dodanych już graczy,
 * @param[in,out] added      – liczba dodanych już graczy.
 * @return Wartość zmiennej @p added, przechowującej liczbę dodanych adresów graczy
 * do tablicy @p neighbours.
 */
static uint8_t add_neighbour_if_unique(gamma_t *g, int64_t x, int64_t y,
                                       player_t **neighbours, uint8_t added) {
    if (neighbour_valid_unique_field(g, x, y, neighbours, added)) {
        neighbours[added] = field_owner(g->board[y][x]);
        added++;
    }
    return added;
}

/** @brief Tworzy zbiór graczy posiadających pionek na co najmniej jednym z pól
 * sąsiadujących z polem (@p x, @p y).
 * Alokuje pamięć na tablicę o długości @p MAX_NEIGHBOURS wskaźników do struktur
 * przechowujących stany graczy posiadających pionek na co najmniej jednym z pól
 * sąsiadujących z polem (@p x, @p y). Funkcja wywołująca musi ją zwolnić.
 * Inicjuje każdą z komórek tej tablicy wartością NULL, tak, aby reprezentowała
 * zbiór pusty.
 * Dodaje niepowtarzające się adresy struktur przechowujących stany takich graczy
 * do tej tablicy.
 * @param[in] g              – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x              – numer kolumny, liczba nieujemna mniejsza od wartości
 *                             @p width z funkcji @ref gamma_new,
 * @param[in] y              – numer wiersza, liczba nieujemna mniejsza od wartości
 *                             @p height z funkcji @ref gamma_new,
 * @return Wskaźnik na tablicę @p neighbours reprezentującą zbiór graczy
 * posiadających pionek na co najmniej jednym z pól sąsiadujących
 * z polem (@p x, @p y) lub NULL, jeśli nie udało się zaalokować pamięci.
 */
static player_t **unique_neighbours(gamma_t *g, uint32_t x, uint32_t y) {
    uint8_t added = 0;
    player_t **neighbours = calloc(MAX_NEIGHBOURS, sizeof(player_t *));

    if (neighbours != NULL) {
        added = add_neighbour_if_unique(g, x - 1, y, neighbours, added);
        added = add_neighbour_if_unique(g, x + 1, y, neighbours, added);
        added = add_neighbour_if_unique(g, x, y - 1, neighbours, added);
        add_neighbour_if_unique(g, x, y + 1, neighbours, added);
    }

    return neighbours;
}

/** @brief Zmniejsza o 1 obwód każdego gracza posiadającego pionek na co namniej
 * jednym polu sąsiadującym z polem wskazywanym przez @p f.
 * Wywołuje funkcję @ref unique_neighbours tworzącą zbiór graczy posiadających
 * pionek na co najmniej jednym polu sąsiadującym z polem wskazywanym przez @p f,
 * które było dotąd wolne i na którym postawiono właśnie pionek, reprezentowanym
 * przez tablicę wskaźników @p neighbours.
 * Zmniejsza o 1 wartość składowej @p perimeter każdego z graczy, którego adres
 * został dodany do tablicy @p neighbours.
 * Zwalnia tablicę @p neighbours.
 * @param[in] g              – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] f              – wskaźnik na strukturę przechowującą stan pola.
 */
static void neighbours_update_perimeter(gamma_t *g, field_t *f) {
    player_t *owner = field_owner(f);
    player_t **neighbours = unique_neighbours(g, field_x(f), field_y(f));
    check_for_successful_alloc(neighbours);

    for (uint8_t i = 0; i < MAX_NEIGHBOURS; i++) {
        if (neighbours[i] != NULL && neighbours[i] != owner) {
            player_set_perimeter(neighbours[i], player_perimeter(neighbours[i]) - 1);
        }
    }

    free(neighbours);
}

///@}

/** @name Ruch
 * Sprawdzanie czy dany ruch może zostać przez gracza wykonany oraz realizacja
 * ruchu gracza.
 */
///@{

/** @brief Sprawdza, czy gracz może wykonać ruch.
 * Sprawdza, czy gracz wskazywany przez @p p może postawić pionek
 * na polu (@p x, @p y).
 * Sprawdza, czy pole (@p x, @p y) jest poprawne oraz wolne.
 * Sprawdza, czy wykonanie ruchu nie przekroczy maksymalnej dopuszczalnej liczby
 * obszarów zajętych przez jednego gracza.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] p – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości @p width
 *                z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości @p height
 *                z funkcji @ref gamma_new.
 * @return Wartość @p true, jeżeli gracz wskazywany przez @p p może postawić
 * pionek na polu (@p x, @p y), a @p false w przeciwnym przypadku.
 */
static bool player_move_legal(gamma_t *g, player_t *p, uint32_t x, uint32_t y) {
    if (!valid_free_field(g, x, y)) {
        return false;
    }
    else if (p == NULL || player_areas(p) < g->areas) {
        return true;
    }
    else {
        return player_adjacent_fields(g, p, x, y) > 0;
    }
}

/** @brief Wykonuje ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y).
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 */
static void gamma_move_update(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    gamma_add_player(g, player);

    g->board[y][x] = field_new(x, y, g->players_arr[player]);
    check_for_successful_alloc(g->board[y][x]);

    player_t *p = g->players_arr[player];
    field_t *f = g->board[y][x];

    g->busy_fields++;
    player_set_busy_fields(p, player_busy_fields(p) + 1);

    player_modify_areas(g, f);
    neighbours_update_perimeter(g, f);
    player_update_perimeter(g, f, false);
}

///@}

/** @name Złoty ruch
 * Sprawdzanie czy złoty ruch jest legalny zarówno ze strony gracza,
 * który stawia swój pionek na polu zajętym przez przeciwnika, nazywanym
 * ofiarą (@p victim), jak i ze strony tracącego pole.
 * Implementacja algorytmu przeszukiwania w głąb (DFS), wykorzystywanego
 * do sprawdzenia, czy usunięcie pola nie zwiększy liczby obszarów zajętych
 * przez ofiarę ponad dopuszczalny limit.
 */
///@{

/** @brief Przeszukuje obszar zajęty przez gracza.
 * Wykonuje przeszukiwanie w głąb (DFS) obszaru zajętego przez gracza wskazywanego
 * przez @p owner, zaczynając od pola (@p x, @p y) i ustawiając status każdego
 * odwiedzonego pola w tym obszarze na wartość @p desired, równą jednej z wartości
 * zdefiniowanych w wyliczeniu @ref status.
 * @param[in,out] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] owner      – wskaźnik na strukturę przechowującą stan gracza,
 *                         będącego właścicielem pola (@p x, @p y),
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                         @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new,
 * @param[in] desired    – status na jaki ma się zmienić status każdego
 *                         odwiedzonego pola, jedna z wartości zdefiniowanych
 *                         w wyliczeniu @ref status.
 * @return Wartość @p true, jeżeli pole (@p x, @p y) jest poprawne i należy do
 * gracza wskazywanego przez @p owner oraz w momencie wywołania funkcji nie miało
 * jeszcze żądanego statusu, a @p false w przeciwnym przypadku.
 */
static bool area_search(gamma_t *g, player_t *owner,
                        int64_t x, int64_t y, status_t desired) {
    if (!player_valid_field(g, owner, x, y)) {
        return false;
    }
    else if (field_status(g->board[y][x]) == desired) {
        return false;
    }
    else {
        field_set_status(g->board[y][x], desired);

        area_search(g, owner, x - 1, y, desired);
        area_search(g, owner, x + 1, y, desired);
        area_search(g, owner, x, y - 1, desired);
        area_search(g, owner, x, y + 1, desired);

        return true;
    }
}

/** @brief Daje liczbę obszarów zajętych przez gracza po utracie pola (@p x, @p y).
 * Oblicza, ile obszarów będzie zajmować gracz wskazywany przez @p victim
 * po utracie pola (@p x, @p y).
 * @param[in,out] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] victim     – wskaźnik na strukturę przechowującą stan gracza,
 *                         będącego właścicielem pola (@p x, @p y),
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                         @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new.
 * @return Liczba obszarów, jakie będzie zajmować gracz wskazywany przez @p victim
 * po utracie pola (@p x, @p y).
 */
static uint32_t victim_new_areas(gamma_t *g, player_t *victim,
                                 uint32_t x, uint32_t y) {
    field_set_status(g->board[y][x], COUNTED);

    uint32_t areas = player_areas(victim) - 1;

    areas += area_search(g, victim, x - 1, y, COUNTED);
    areas += area_search(g, victim, x + 1, y, COUNTED);
    areas += area_search(g, victim, x, y - 1, COUNTED);
    areas += area_search(g, victim, x, y + 1, COUNTED);

    return areas;
}

/** @brief Sprawdza, czy złoty ruch jest legalny ze strony gracza, który traci pole.
 * Sprawdza, czy złoty ruch jest legalny ze strony gracza, który traci pionek
 * z pola (@p x, @p y).
 * Zakłada, że pole (@p x, @p y) jest poprawnym, zajętym polem, ponieważ sprawdzenie
 * tych warunków następuje w funkcji @ref gamma_golden_possible.posiadaniu
 * Sprawdza, czy po utracie pola (@p x, @p y) przez jego właściciela liczba zajętych
 * przez niego obszarów nie przekroczy maksymalnej dozwolonej liczby obszarów
 * zajętych przez jednego gracza.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] p – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości @p width
 *                z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości @p height
 *                z funkcji @ref gamma_new.
 * @return Wartość @p true, jeżeli po utracie pola (@p x, @p y) przez jego
 * właściciela liczba zajętych przez niego obszarów nie będzie przekraczać
 * maksymalnej dozwolonej liczby obszarów zajętych przez jednego gracza,
 * a @p false w przeciwnym przypadku.
 */
static bool victim_golden_move_legal(gamma_t *g, uint32_t x, uint32_t y) {
    player_t *victim = field_owner(g->board[y][x]);
    uint8_t mx_new_areas = player_adjacent_fields(g, victim, x, y) - 1;

    if (player_areas(victim) + mx_new_areas <= g->areas) {
        return true;
    }
    else if (victim_new_areas(g, victim, x, y) <= g->areas) {
        return true;
    }
    else {
        area_search(g, victim, x, y, UNCHECKED);
        return false;
    }
}

/** @brief Sprawdza, czy złoty ruch jest legalny ze strony gracza, który
 * stawia pionek na polu zajętym przez przeciwnika.
 * Sprawdza, czy złoty ruch jest legalny ze strony gracza wskazywanego
 * przez @p p, który stawia pionek na polu (@p x, @p y) zajętym przez przeciwnika.
 * Zakłada, że gracz wskazywany przez @p p jeszcze nie wykonał w rozgrywce złotego
 * ruchu oraz że pole (@p x, @p y) jest poprawne, ponieważ sprawdzenie tych warunków
 * następuje w funkcji @ref gamma_golden_possible.
 * Sprawdza, czy pole (@p x, @p y) nie jest zajęte przez gracza wskazywanego przez
 * @p p oraz czy po postawieniu pionka na tym polu nie zostanie przekroczona
 * maksymalna liczba obszarów, jakie może gracz zajmować.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] p – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości @p width
 *                z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości @p height
 *                z funkcji @ref gamma_new.
 * @return Wartość @p true, jeżeli pole (@p x, @p y) nie jest zajęte przez gracza
 * wskazywanego przez @p p oraz po postawieniu pionka na tym polu nie zostanie
 * przekroczona maksymalna liczba obszarów, jakie może gracz zajmować.
 */
static bool player_golden_move_legal(gamma_t *g, player_t *p,
                                     uint32_t x, uint32_t y) {
    if (p == NULL) {
        return true;
    }
    else if (field_owner(g->board[y][x]) == p) {
        return false;
    }
    else if (player_areas(p) < g->areas) {
        return true;
    }
    else {
        return player_adjacent_fields(g, p, x, y) > 0;
    }
}

/** @brief Zmienia rangę oraz rodzica w każdym polu obszaru.
 * Wykonuje przeszukiwanie w głąb (DFS) obszaru zajętego przez gracza wskazywanego
 * przez @p owner, zaczynając od pola (@p x, @p y) i ustawiając składową @p rank
 * każdego odwiedzonego pola na 0 oraz składową @p parent na wartość równą
 * zmiennej @p parent będącej parametrem procedury.
 * Ustawia status @p status każdego odwiedzonego pola na @p MODIFIED.
 * @param[in,out] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] owner      – wskaźnik na strukturę przechowującą stan gracza,
 *                         będącego właścicielem pola (@p x, @p y),
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                         @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new,
 * @param[in] parent     – wskaźnik na pole będące nowym rodzicem każdego
 *                         odwiedzonego pola.
 */
static void area_update_parent_and_rank(gamma_t *g, player_t *owner,
                                        int64_t x, int64_t y, field_t *parent) {
    if (player_valid_field(g, owner, x, y)
        && field_status(g->board[y][x]) != MODIFIED) {

        field_t *f = g->board[y][x];

        field_set_rank(f, 0);
        field_set_parent(f, parent);
        field_set_status(f, MODIFIED);

        area_update_parent_and_rank(g, owner, x - 1, y, parent);
        area_update_parent_and_rank(g, owner, x + 1, y, parent);
        area_update_parent_and_rank(g, owner, x, y - 1, parent);
        area_update_parent_and_rank(g, owner, x, y + 1, parent);
    }
}

/** @brief Wyodrębnia nowy obszar ze starego po usunięciu z niego pola w wyniku
 * złotego ruchu.
 * Jeżeli pole (@p x, @p y) należy do gracza wskazywanego przez @p old_owner,
 * będącego graczem który w wyniku złotego ruchu wykonanego przez przeciwnika
 * stracił sąsiadujące pole, czyni pole (@p x, @p y) korzeniem nowego obszaru,
 * ustawiając jego rangę @p rank na 0 oraz rodzica @p parent na NULL, po czym
 * wywołuje funkcję @ref area_update_parent_and_rank dla każdego sąsiedniego pola,
 * przekazując jako parametr @p parent tej funkcji wskaźnik do struktury
 * przechowującej stan pola (@p x, @p y).
 * Oblicza liczbę obszarów zajętych przez gracza wskazywanego przez @p old_owner
 * po wyodrębnieniu wszystkich nowych obszarów.
 * @param[in,out] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] old_owner  – wskaźnik na strukturę przechowującą stan gracza,
 *                         będącego właścicielem pola (@p x, @p y),
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                         @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new,
 * @param[in,out] areas  – liczba obszarów zajętych przez gracza wskazywanego
 *                         przez @p old_owner.
 * @return Liczba obszarów zajętych przez gracza wskazywanego przez @p old_owner
 * po ewentualnym wyobrębnieniu nowego obszaru z korzeniem w polu (@p x, @p y).
 */
static uint32_t area_set_component(gamma_t *g, player_t *old_owner,
                                   uint32_t x, uint32_t y, uint32_t areas) {
    if (player_valid_field(g, old_owner, x, y)) {
        if (field_status(g->board[y][x]) != MODIFIED) {
            areas++;

            field_t *root = g->board[y][x];
            field_set_status(root, MODIFIED);
            field_set_rank(root, 0);
            field_set_parent(root, NULL);

            area_update_parent_and_rank(g, old_owner, x - 1, y, root);
            area_update_parent_and_rank(g, old_owner, x + 1, y, root);
            area_update_parent_and_rank(g, old_owner, x, y - 1, root);
            area_update_parent_and_rank(g, old_owner, x, y + 1, root);
        }
    }
    return areas;
}

/** @brief Modyfikuje obszary gracza po utracie przez niego pola (@p x, @p y).
 * Dla każdego pola sąsiadującego z (@p x, @p y), należącego do gracza wskazywanego
 * przez @p old_owner i nie należącego do żadnego nowo stworzonego obszaru, tworzy
 * nowy obszar z korzeniem w tym polu.
 * Odpowiednio aktualizuje liczbę obszarów zajętych przez gracza wskazywanego przez
 * @p old_owner.
 * @param[in,out] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] old_owner  – wskaźnik na strukturę przechowującą stan gracza,
 *                         który utracił pole (@p x, @p y),
 * @param[in] x          – numer kolumny, liczba nieujemna mniejsza od wartości
 *                         @p width z funkcji @ref gamma_new,
 * @param[in] y          – numer wiersza, liczba nieujemna mniejsza od wartości
 *                         @p height z funkcji @ref gamma_new.
 */
static void old_owner_modify_areas(gamma_t *g, player_t *old_owner,
                                   uint32_t x, uint32_t y) {
    uint32_t areas = player_areas(old_owner) - 1;

    areas = area_set_component(g, old_owner, x - 1, y, areas);
    areas = area_set_component(g, old_owner, x + 1, y, areas);
    areas = area_set_component(g, old_owner, x, y - 1, areas);
    areas = area_set_component(g, old_owner, x, y + 1, areas);

    player_set_areas(old_owner, areas);

    area_search(g, old_owner, x - 1, y, UNCHECKED);
    area_search(g, old_owner, x + 1, y, UNCHECKED);
    area_search(g, old_owner, x, y - 1, UNCHECKED);
    area_search(g, old_owner, x, y + 1, UNCHECKED);
}

/** @brief Wykonuje złoty ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y) zajętym przez innego
 * gracza, usuwając pionek innego gracza.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 */
static void gamma_golden_move_update(gamma_t *g, uint32_t player,
                                     uint32_t x, uint32_t y) {
    gamma_add_player(g, player);

    player_t *new_owner = g->players_arr[player];
    field_t *f = g->board[y][x];
    player_t *old_owner = field_owner(f);

    field_set_owner(f, new_owner);
    field_set_status(f, UNCHECKED);

    player_modify_areas(g, f);
    player_update_perimeter(g, f, true);
    player_set_golden_possible(new_owner, false);
    player_set_busy_fields(new_owner, player_busy_fields(new_owner) + 1);

    old_owner_modify_areas(g, old_owner, x, y);
    player_set_busy_fields(old_owner, player_busy_fields(old_owner) - 1);
    player_set_perimeter(old_owner,
                         player_perimeter(old_owner) -
                         player_adjacent_free_single_fields(g, old_owner, x, y));
}

///@}

/** @name Plansza
 * Tworzenie, usuwanie oraz wypisywanie planszy.
 */
///@{

/** @brief Usuwa wiersze planszy.
 * Usuwa @p num_of_rows początkowych wierszy tablicy @p boards wskaźników
 * do struktur przechowujących stany pól, reprezentującej planszę, na której
 * toczy się rozgrywka.
 * Zwalnia pamięć po każdym polu, którego adres był zapisany w jednym z usuwanych
 * wierszy.
 * Zwalnia pamięć po każdym z @p num_of_rows początkowych wierszy.
 * @param board[in,out]       – tablica wskaźników do pól, o liczbie wierszy
 *                              równej wartości @p height z funkcji
 *                              @ref gamma_new oraz liczbie kolumn równej
 *                              wartości @p width z funkcji @ref gamma_new,
 *                              reprezentująca planszę na której odbywa się
 *                              rozgrywka,
 * @param width[in]           – długość każdego z wierszy tablicy @p board,
 *                              równa wartości @p width z funkcji @ref gamma_new,
 * @param num_of_rows[in]     – liczba wierszy tablicy @p board, które należy
 *                              usunąć.
 */
static void board_remove_rows(field_t ***board, uint32_t width,
                              uint32_t num_of_rows) {
    for (uint32_t i = 0; i < num_of_rows; i++) {
        for (uint32_t j = 0; j < width; j++) {
            field_delete(board[i][j]);
        }
        free(board[i]);
    }
}

/** @brief Tworzy planszę.
 * Alokuje pamięć na tablicę wskaźników do struktur przechowujących stan pól,
 * o @p height wierszach i @p width kolumnach.
 * Przypisuje każdej komórce tej tablicy wartość NULL.
 * @param width[in]           – szerokość tworzonej planszy, długość każdego
 *                              z wierszy tworzonej tablicy, równa wartości
 *                              @p width z funkcji @ref gamma_new,
 * @param height[in]          – wysokość tworzonej planszy, liczba wierszy
 *                              tworzonej tablicy, wartość równa @p height
 *                              z funkcji @ref gamma_new.
 * @return Wskaźnik na utworzoną tablicę lub NULL, jeśli nie udało się zaalokować
 * pamięci.
 */
static field_t ***board_new(uint32_t width, uint32_t height) {
    field_t ***board = malloc(height * sizeof(field_t ***));

    if (board == NULL) {
        return NULL;
    }
    else {
        bool mem_error = false;
        uint32_t added_rows = 0;

        for (uint32_t i = 0; i < height && !mem_error; i++, added_rows++) {
            board[i] = calloc(width, sizeof(field_t **));
            if (board[i] == NULL) {
                mem_error = true;
                board_remove_rows(board, width, added_rows);
            }
        }

        if (!mem_error) {
            return board;
        }
        else {
            free(board);
            return NULL;
        }
    }
}

/** @brief Wypełnia bufor opisujący stan planszy.
 * Wypełnia bufor @p board opisujący stan planszy w przypadku, kiedy liczba
 * graczy jest niewiększa niż 9.
 * @param g[in]               – wskaźnik na strukturę przechowującą stan gry,
 * @param board[in,out]       – wskaźnik na bufor będący opisem stanu planszy.
 */
static void board_fill_string(gamma_t *g, char *board) {
    uint64_t filled = 0;

    for (int64_t y = g->height - 1; y >= 0; y--, filled++) {
        for (uint32_t x = 0; x < g->width; x++, filled++) {
            if (g->board[y][x] == NULL) {
                board[filled] = '.';
            }
            else {
                board[filled] = player_number(field_owner(g->board[y][x])) + '0';
            }
        }
        board[filled] = '\n';
    }

    board[filled] = '\0';
}

///@}

static bool gamma_init(gamma_t *g, uint32_t width, uint32_t height,
                       uint32_t players, uint32_t areas) {
    g->board = board_new(width, height);

    if (g->board == NULL) {
        return false;
    }
    else {
        g->players_arr = calloc(players + 1, sizeof(player_t *));

        if (g->players_arr == NULL) {
            return false;
        }
        else {
            g->width = width;
            g->height = height;
            g->players = players;
            g->areas = areas;
            g->busy_fields = 0;
            return true;
        }
    }
}

gamma_t *gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas) {
    if (width == 0 || height == 0 || players == 0 || areas == 0) {
        return NULL;
    }
    else {
        gamma_t *g = malloc(sizeof(gamma_t));
        if (g == NULL) {
            return NULL;
        }
        else if (gamma_init(g, width, height, players, areas)) {
            return g;
        }
        else {
            gamma_delete(g);
            return NULL;
        }
    }
}

void gamma_delete(gamma_t *g) {
    if (g != NULL) {
        if (g->board != NULL) {
            board_remove_rows(g->board, g->width, g->height);
            free(g->board);
        }

        if (g->players_arr != NULL) {
            for (uint32_t i = 0; i <= g->players; i++) {
                player_delete(g->players_arr[i]);
            }
            free(g->players_arr);
        }

        free(g);
    }
}

bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (g == NULL || !valid_player(g, player)) {
        return false;
    }
    else if (!player_move_legal(g, g->players_arr[player], x, y)) {
        return false;
    }
    else {
        gamma_move_update(g, player, x, y);
        return true;
    }
}

bool gamma_golden_possible(gamma_t *g, uint32_t player) {
    if (g == NULL || !valid_player(g, player)) {
        return false;
    }
    else if (g->players_arr[player] == NULL) {
        return g->busy_fields > 0;
    }
    else {
        player_t *p = g->players_arr[player];
        return player_golden_possible(p) && player_busy_fields(p) < g->busy_fields;
    }
}

bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (!gamma_golden_possible(g, player)) {
        return false;
    }
    else if (!valid_busy_field(g, x, y)) {
        return false;
    }
    else if (!player_golden_move_legal(g, g->players_arr[player], x, y)) {
        return false;
    }
    else if (!victim_golden_move_legal(g, x, y)) {
        return false;
    }
    else {
        gamma_golden_move_update(g, player, x, y);
        return true;
    }
}

uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
    if (g == NULL || !valid_player(g, player) || g->players_arr[player] == NULL) {
        return 0;
    }
    else {
        return player_busy_fields(g->players_arr[player]);
    }
}

uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
    if (g == NULL || !valid_player(g, player)) {
        return 0;
    }
    else {
        player_t *p = g->players_arr[player];

        if (p == NULL || player_areas(p) < g->areas) {
            return g->width * g->height - g->busy_fields;
        }
        else {
            return player_perimeter(p);
        }
    }
}

char *gamma_board(gamma_t *g) {
    if (g == NULL) {
        return NULL;
    }
    else {
        char *board = malloc(((g->width + 1) * g->height + 1) * sizeof(char));

        if (board != NULL) {
            board_fill_string(g, board);
        }

        return board;
    }
}
