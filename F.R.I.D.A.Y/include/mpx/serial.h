#ifndef MPX_SERIAL_H
#define MPX_SERIAL_H

#include "sys_req.h"
#include <stddef.h>
#include "mpx/pcb.h"
#include <mpx/device.h>

/**
 @file mpx/serial.h
 @brief Kernel functions and constants for handling serial I/O
*/

///An enumeration of all possible results for an IO request.
typedef enum {
    ///If one or more of the provided parameters were invalid.
    INVALID_PARAMS,
    ///Returned if the requested device isn't open.
    DEVICE_CLOSED,
    ///Returned if the device is currently busy.
    DEVICE_BUSY,
    ///Returned if the request was only partially serviced.
    PARTIALLY_SERVICED,
    ///Returned if the request was fully serviced.
    SERVICED,
} io_req_result;

/**
 * @brief Checks for any completed PCBs that were doing IO operations.
 * @return a PCB to load, or NULL.
 */
struct pcb *check_completed(void);

/**
 * @brief Performs an IO operation on the given device, returning the result.
 *
 * @param pcb the PCB requesting this operation.
 * @param operation the operation.
 * @param dev the device.
 * @param buffer the buffer.
 * @param length the amount of characters to transfer.
 * @return the result of the operation.
 */
io_req_result io_request(struct pcb *pcb, op_code operation, device dev, char *buffer, size_t length);

/**
 Initializes devices for user input and output
 @param device A serial port to initialize (COM1, COM2, COM3, or COM4)
 @return 0 on success, non-zero on failure
*/
int serial_init(device dev);

/**
 Initializes devices for user input and output
 @param device A serial port to initialize (COM1, COM2, COM3, or COM4)
 @return 0 on success, non-zero on failure
*/
int serial_open(device dev, int speed);

/**
 * @brief Closes the given device.
 *
 * @param dev the device to close.
 * @return 0 on success, negative values on error.
 */
int serial_close(device dev);

/**
 Writes a buffer to a serial port
 @param device The serial port to output to
 @param buffer A pointer to an array of characters to output
 @param len The number of bytes to write
 @return The number of bytes written
*/
int serial_out(device dev, const char *buffer, size_t len);

/**
 * @brief Writes len bytes from the given buffer to the device.
 * @param dev the device to write to.
 * @param buffer the buffer to read from.
 * @param len the amount of bytes to write.
 * @return the number of bytes written.
 */
int serial_write(device dev, char *buffer, size_t len);

/**
 * @brief Reads input on the given device.
 *
 * @param dev the device to read on.
 * @param buf the buffer to read with.
 * @param len the length of the buffer.
 * @return 0 on success, negative values on error.
 */
int serial_read(device dev, char *buf, size_t len);

/**
 Reads a string from a serial port
 @param device The serial port to read data from
 @param buffer A buffer to write data into as it is read from the serial port
 @param count The maximum number of bytes to read
 @return The number of bytes read on success, a negative number on failure
*/   		   

int serial_poll(device dev, char *buffer, size_t len);

#endif
