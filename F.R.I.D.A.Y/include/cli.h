//
// Created by Andrew Bowie on 1/27/23.
//

#ifndef F_R_I_D_A_Y_CLI_H
#define F_R_I_D_A_Y_CLI_H

/**
 * @file cli.h
 * @brief Contains useful commands for interfacing with the CLI.
 */

/**
 * @brief Sets the CLI prompt to be used when prompting input.
 *        Can be set to NULL if no prompt should be printed.
 * @param prompt the prompt to use.
 */
void set_cli_prompt(const char *prompt);

/**
 * @brief Sets if the CLI is enabled.
 * @param enabled if the CLI should be enabled.
 */
void set_cli_history(bool enabled);

/**
 * @brief If command color formatting should be enabled.
 * @param enabled if it should be enabled.
 */
void set_command_formatting(bool enabled);

/**
 * @brief Sets if the input for the line should be invisible.
 * @param enabled if it's enabled or not.
 */
void set_invisible(bool enabled);

/**
 * @brief Sets if the input should use tab completions.
 * @param enabled if it's enabled or not.
 */
void set_tab_completions(bool enabled);

#endif //F_R_I_D_A_Y_CLI_H
