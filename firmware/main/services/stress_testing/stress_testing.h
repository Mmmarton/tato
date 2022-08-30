#ifndef STRESS_TESTING_H
#define STRESS_TESTING_H

#include "stdint.h"

/**
 * @brief This function is used to initialize the stress testing module
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t stress_testing_init(void);

/**
 * @brief This function is used to start the stress testing loop that handles the entire logic.
 *
 * @return (status_t)
 * @retval 0 success, others for errors
 */
status_t stress_testing_main(void);

#endif