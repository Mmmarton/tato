#ifndef CONFIGURATION_MENU_H
#define CONFIGURATION_MENU_H

#define CONFIGURATION_MENU_PROPERTIES_COUNT 13 /*!< total number of user-editable configurations */
#define CONFIGURATION_MENU_BUTTON_THRESHOLD 2  /*!< number of seconds needed to press the button to enter the menu */

/** @brief This enum contains page identifiers */
typedef enum {
    CONFIGURATION_MENU_PAGE_NFC,
    CONFIGURATION_MENU_PAGE_SD_CARD,
    CONFIGURATION_MENU_PAGE_MAIN,
    CONFIGURATION_MENU_PAGE_FINISH,
} configuration_menu_pages_e;

/**
 * @brief This function is used to put the badge in the configuration menu.
 *
 * @return (void)
 */
void configuration_menu_loop(void);

#endif