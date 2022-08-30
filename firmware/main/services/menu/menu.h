/*! @file menu.h
 *
 * @brief This module is used to create a menu instance.
 */

/*!
 * @defgroup MENU
 * @{*/

#ifndef MENU_H
    #define MENU_H

    #include <stdbool.h>
    #include <stdint.h>

    #define BUTTON_WIDTH  180 /*!< width of a button */
    #define BUTTON_HEIGHT 40  /*!< height of a button */

    #define MENU_X_COORD 40 /*!< X-coordinate of the menu (upper-left corner) */
    #define MENU_Y_COORD 40 /*!< Y-coordinate of the menu (upper-left corner) */

    #define MENU_BUTTON_OFFSET   50 /*!< offset between buttons (includes the button height) */
    #define MAXIMUM_MENU_ENTRIES 20 /*!< maximum allowed number of menu entries */

/** @brief Pointer to a function. */
typedef void (*function)(void);

/** @brief This structure contains the menu entry properties. */
typedef struct {
    uint8_t *text;   /*!< name of the entry (will be rendered as button) */
    uint16_t x;      /*!< x-coordinate of the entry */
    uint16_t y;      /*!< y-coordinate of the entry */
    function action; /*!< pointer to the function that will be executed when the entry is selected */
} menu_entry_t;

/** @brief This structure contains the menu properties. */
typedef struct {
    menu_entry_t entries[MAXIMUM_MENU_ENTRIES];
    uint8_t      size;        /*!< total entry count */
    uint8_t      entry_count; /*!< number of entries to be rendered on the screen at the same time */
    uint16_t     menu_x;      /*!< x-coordinate of the menu (upper left) */
    uint16_t     menu_y;      /*!< x-coordinate of the menu (upper left) */

    int8_t cursor; /*!< current position on the menu */
    int8_t upper_menu_bound;
    int8_t lower_menu_bound;
} menu_t;

/**
 * @brief This function initializes the menu page and renders the eye-candy.
 *
 * @param[in]   menu    User-defined menu.
 *
 * @return      (void)
 */
void menu_init(menu_t *menu);

/**
 * @brief This function acts as the main loop of a user-defined menu.
 *
 * @param[in]   menu    User-defined menu.
 *
 * @return      (void)
 */
void menu_loop(menu_t *menu);

/**
 * @brief This function sets the `should_redraw_menu` flag which triggers a rendering of the menu when exiting a page.
 *
 * @param[in]   should_redraw   New flag value.
 *
 * @return      (void)
 */
void menu_set_should_redraw_menu(bool should_redraw);

#endif
/** @}*/