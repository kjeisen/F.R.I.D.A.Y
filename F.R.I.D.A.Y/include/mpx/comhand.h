
#ifndef F_R_I_D_A_Y_COMHAND_H
#define F_R_I_D_A_Y_COMHAND_H

#define CMD_PROMPT ">> "

/**
 * @file comhand.h
 * @brief This file controls command running for the OS. Calling the @code comhand(void) function
 * will initialize the command loop. The loop will run until it errors or it is signaled to shutdown.
 */

/**
 * @brief Signals to the command handler that it should stop whenever it
 * gets the chance.
 */
void signal_shutdown(void);

/**
 * @brief Starts the command handler.
 */
void comhand(void);

#endif //F_R_I_D_A_Y_COMHAND_H
