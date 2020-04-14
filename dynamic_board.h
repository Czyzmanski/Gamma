/** @file
 * Interfejs klasy reprezentującej dynamiczną tablicę zawierającą napis
 * opisujący aktualny stan planszy
 *
 * @author Szymon Czyżmański 417797
 * @date 13.04.2020
 */

#ifndef DYNAMIC_BOARD_H
#define DYNAMIC_BOARD_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Struktura przechowująca stan dynamicznego bufora zawierającego napis
 * opisujący aktualny stan planszy.
 */
typedef struct dynamic_board dyn_board_t;

/** @brief Tworzy strukturę przechowującą opis aktualnego stanu planszy.
 * Alokuje pamięć na nową strukturę przechowującą napis opisujący aktualny
 * stan planszy.
 * Inicjalizuje tę strukturę tak, aby reprezentowała pusty napis.
 * @param[in] capacity  – początkowy rozmiar wewnętrznego bufora nowo tworzonej
 *                        struktury, zawierającego opis aktualnego stanu planszy,
 *                        liczba całkowita dodatnia.
 * @return Wskaźnik na nowo utworzoną strukturę lub NULL, jeśli nie udało się
 * zaalokować pamięci.
 */
dyn_board_t *dynamic_board_new(uint64_t capacity);

/** @brief Dodaje znak do struktury przechowującej opis aktualnego stanu planszy.
 * Dodaje znak @p c do struktury wskazywanej przez @p board przechowującej napis
 * opisujący aktualny stan planszy.
 * W miarę potrzeby, zwiększa pojemność wewnętrznego bufora tak, by jego nowa
 * pojemność była równa starej pojemności pomnożonej przez @p GROWTH_FACTOR.
 * @param[in,out] board – wskaźnik na strukturę przechowującą napis zawierający
 *                        opis aktualnego stanu planszy,
 * @param[in] c         – znak do dodania do struktury wskazywanej przez
 *                        @p board.
 * @return Wartość @p true, jeśli dodano znak pomyślnie, a @p false
 * w przeciwnym przypadku.
 */
bool dynamic_board_add_char(dyn_board_t *board, char c);

/** @brief Dodaje numer gracza.
 * Dodaje cyfry numeru gracza @p player do struktury przechowującej opis
 * aktualnego stanu planszy.
 * Dodaj znak @p c do struktury wskazywanej przez @p board przechowującej napis
 * opisujący aktualny stan planszy.
 * W miarę potrzeby, zwiększa dwukrotnie pojemność wewnętrznego bufora.
 * @param[in,out] board – wskaźnik na strukturę przechowującą napis zawierający
 *                        opis aktualnego stanu planszy,
 * @param[in] player    – numer gracza, który ma zostać dodany do struktury
 *                        wskazywanej przez @p board, liczba dodatnia
 *                        niewiększa niż wartość @p players z funkcji
 *                        @ref gamma_new.
 * @return Wartość @p true, jeśli dodano znak pomyślnie, a @p false
 * w przeciwnym przypadku.
 */
bool dynamic_board_add_player(dyn_board_t *board, uint32_t player);

/** @brief Daje napis opisujący stan planszy.
 * Alokuje w pamięci bufor długości liczby dodanych znaków do bufora
 * przechowywanego w strukturze wskazywanej przez @p board. Kopiuje wszystkie
 * dodane znaki z tego bufora do nowo zaalokowanego bufora, tworząc w ten sposób
 * napis zawierający tekstowy opis aktualnego stanu planszy.
 * @param[in] board     – wskaźnik na strukturę przechowującą napis zawierający
 *                        opis aktualnego stanu planszy.
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
char *dynamic_board_fitted_array(dyn_board_t *board);

/** @brief Usuwa strukturę przechowującą opis planszy.
 * Usuwa strukturę wskazywaną przez @p board, zawierającą bufor przechowujący
 * opis aktualnego stanu planszy.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] board     – wskaźnik na strukturę przechowującą bufor zawierający
 *                        opis aktualnego stanu planszy.
 */
void dynamic_board_delete(dyn_board_t *board);

#endif // DYNAMIC_BOARD_H
