#include <mpx/io.h>
#include <mpx/serial.h>
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "ctype.h"
#include "linked_list.h"
#include "memory.h"
#include "commands.h"
#include "color.h"
#include "mpx/interrupts.h"
#include "sys_req.h"
#include "cli.h"
#include "commands.h"
#define RING_BUFFER_LEN 150

#define ERROR_101 "invalid (null) event flag pointer"
#define ERROR_102 "Invalid baud rate divisor"
#define ERROR_103 "Port Already Open"
#define ERROR_201 "Serial port not open"
#define ERROR_301 "Serial port not open"
#define ERROR_302 "Invalid buffer address"
#define ERROR_303 "Invalid count address or count value"
#define ERROR_304 "Device is busy"
#define ERROR_401 "Serial port not open"
#define ERROR_402 "Invalid buffer address"
#define ERROR_403 "Invalid count address or count value"
#define ERROR_404 "Device is busy"

enum uart_registers
{
    RBR = 0,    // Receive Buffer
    THR = 0,    // Transmitter Holding
    DLL = 0,    // Divisor Latch LSB
    IER = 1,    // Interrupt Enable
    DLM = 1,    // Divisor Latch MSB
    IIR = 2,    // Interrupt Identification
    FCR = 2,    // FIFO Control
    LCR = 3,    // Line Control
    MCR = 4,    // Modem Control
    LSR = 5,    // Line Status
    MSR = 6,    // Modem Status
    SCR = 7,    // Scratch
};
typedef struct code{
    char* err_msg;
    int err_code;
} code;

int code_selection(int error_num){
    code error;
    
    switch(error_num){
        case -101:
            error.err_msg = ERROR_101;
            error.err_code = -101;
            break;
        case -102:
            error.err_msg = ERROR_102;
            error.err_code = -102;
            break;
        case -103:
            error.err_msg = ERROR_103;
            error.err_code = -103;
            break;
        case -201:
            error.err_msg = ERROR_201;
            error.err_code = -201;
            break;
        case -301:
            error.err_msg = ERROR_301;
            error.err_code = -301;
            break;
        case -302:
            error.err_msg = ERROR_302;
            error.err_code = -302;
            break;
        case -303:
            error.err_msg = ERROR_303;
            error.err_code = -303;
            break;
        case -304:
            error.err_msg = ERROR_304;
            error.err_code = -304;
            break;
        case -401:
            error.err_msg = ERROR_401;
            error.err_code = -401;
            break;
        case -402:
            error.err_msg = ERROR_402;
            error.err_code = -402;
            break;
        case -403:
            error.err_msg = ERROR_403;
            error.err_code = -403;
            break;
        case -404:
            error.err_msg = ERROR_404;
            error.err_code = -404;
            break;
    }
       char buffer[200] = {0};
       sprintf("Error code %d: %s\n",buffer, 200, error.err_code, error.err_msg);
       serial_out(COM1, buffer, strlen(buffer));
   return error_num;
}


static int initialized[4] = {0};

static int serial_devno(device dev)
{
    switch (dev)
    {
        case COM1:
            return 0;
        case COM2:
            return 1;
        case COM3:
            return 2;
        case COM4:
            return 3;
    }
    return -1;
}

int serial_init(device dev)
{
    int dno = serial_devno(dev);
    if (dno == -1)
    {
        return -1;
    }
    outb(dev + IER, 0x00);    //disable interrupts
    outb(dev + LCR, 0x80);    //set line control register
    outb(dev + DLL, 115200 / 9600);    //set bsd least sig bit
    outb(dev + DLM, 0x00);    //brd most significant bit
    outb(dev + LCR, 0x03);    //lock divisor; 8bits, no parity, one stop
    outb(dev + FCR, 0xC7);    //enable fifo, clear, 14byte threshold
    outb(dev + MCR, 0x0B);    //enable interrupts, rts/dsr set
    (void) inb(dev);        //read bit to reset port
    initialized[dno] = 1;
    return 0;
}

int serial_out(device dev, const char *buffer, size_t len)
{
    int dno = serial_devno(dev);
    if (dno == -1 || initialized[dno] == 0)
    {
        return -1;
    }
    for (size_t i = 0; i < len; i++)
    {
        outb(dev, buffer[i]);
    }
    return (int) len;
}

#define ANSI_CODE_READ_LEN 15
#define MAX_CLI_HISTORY_LEN (5)

///Used to store a specific line previously entered.
struct line_entry
{
    ///These are a hacky way to use linked lists without excessive allocation (temp until R5)
    void *_dont_use_1;
    ///These are a hacky way to use linked lists without excessive allocation (temp until R5)
    void *_dont_use_2;

    /**
     * The line that was entered. Does not include null terminator.
     */
    char *line;
    /**
     * The line's length, not including the null terminator.
     */
    size_t line_length;
};

///Contains constants for useful or common keycodes.
enum key_code
{
    BACKSPACE = 8,
    TAB = 9,
    NEWLINE = 10,
    CARRIAGE_RETURN = 13,
    ESCAPE = 27,
    SPACE = 32, //The minimum keycode value for normal characters.
    TILDA = 126, //The maximum keycode value for normal characters.
    DELETE = 127,
};


///A direction registry for line navigation.
enum direction
{
    LEFT = 0,
    RIGHT = 1,
};

///If CLI history should be enabled.
static bool cli_history_enabled = false;
///If CLI command color formatting should be enabled.
static bool command_formatting_enabled = false;
///If the CLI input should be invisible.
static bool cli_invisible = false;
///If the CLI should implement tab completions.
static bool tab_completions = false;
///The prompt to print when requesting input.
static const char *prompt = NULL;

/**
 * @brief Sets the output color using serial_out instead of printf. (Avoids sys_req call)
 * @param color the color to set.
 */
void internal_soc(const color_t *color)
{
    static const char format_arr[2] = {27, '['};
    char color_arr[3] = {0};
    itoa(color->color_num, color_arr, 3);

    serial_out(COM1, format_arr, 2);
    serial_out(COM1, color_arr, strlen(color_arr));
    serial_out(COM1, "m", 1);
}

void set_cli_prompt(const char *str)
{
    if(str != NULL)
    {
        size_t len = strlen(str);
        //Don't allow prompts longer than 40 chars.
        if(len > 40)
            return;
    }

    prompt = str;
}

void set_cli_history(bool hist_enabled)
{
    cli_history_enabled = hist_enabled;
}

void set_command_formatting(bool enabled)
{
    command_formatting_enabled = enabled;
}

void set_invisible(bool enabled)
{
    cli_invisible = enabled;
}

void set_tab_completions(bool enabled)
{
    tab_completions = enabled;
}

/**
 * @brief Moves the text cursor back the given amount of spaces.
 * @param dev the device to print to.
 * @param direc 1 if we should move it direc, 0 if left.
 * @param spaces the amount to move the cursor back.
 */
void move_cursor(device dev, enum direction direc, int spaces)
{
    char full_len_str[20] = {0};
    itoa((int) spaces, full_len_str, 19);
    size_t str_len = strlen(full_len_str);

    char m_left_prefix[3] = {
            ESCAPE,
            '[',
            '\0'
    };
    serial_out(dev, m_left_prefix, 2);
    serial_out(dev, full_len_str, str_len);
    serial_out(dev, direc == RIGHT ? "C" : "D", 1);
}

/**
 * @brief Finds the next word index or the given direction.
 * @param direc the direction to move.
 * @param cursor_index the current cursor index.
 * @param str the string to check in.
 * @param str_len the length of the string.
 * @return the index of the next word.
 */
int find_next_word(enum direction direc, int cursor_index, const char *str, int str_len)
{
    int characters_found = 0;
    int move_dir = direc == RIGHT ? 1 : -1;

    int index = cursor_index + move_dir;

    //Iterate over the string and find the next word index.
    for (; index >= 0 && index < str_len; index += move_dir)
    {
        char c = str[index];
        if (isspace(c))
        {
            if (characters_found > 0)
            {
                if (direc == LEFT && index > 0)
                    index++;
                break;
            }
        } else
        {
            characters_found++;
        }
    }

    //Coerce the number.
    index = index < 0 ? 0 : index;
    index = index > str_len ? str_len : index;

    return index;
}

///An enumeration of possible DCB statuses
typedef enum {
    IDLING,
    READING,
    WRITING,
//    READ_AND_WRITE,
} dcb_status_t;

///A descriptor for a device.
typedef struct {
    ///The device this control block is describing.
    device dev;
    ///Whether or not the control block is allocated.
    bool allocated;
    ///The operation this device is currently doing.
    dcb_status_t operation;
    ///Whether or not there is an event to be handled.
    bool event;
    ///The PCB currently using this DCB.
    struct pcb *pcb;
    ///The amount of bytes in the IO operation.
    size_t io_bytes;
    ///The amount of bytes requested.
    size_t io_requested;
    ///The line position for the cursor, used for read ops.
    size_t line_pos;
    ///The active IO buffer for this DCB.
    char *io_buffer;
    ///A buffer used specifically for handling ASCII escape characters.
    char escape_buffer[6];
    ///The position in the escape buffer.
    int escape_buf_pos;
    ///The total length of the ring buffer.
    size_t r_buffer_len;
    ///The current size of the ring buffer.
    size_t r_buffer_size;
    ///The beginning of the ring buffer.
    char *r_buffer_start;
    ///The read index for the ring buffer.
    int read_index;
    ///The write index for the ring buffer.
    int write_index;
    ///This list contains all pending operations.
    linked_list *pending_iocb;
} dcb_t;

///A descriptor for pending IO operations.
typedef struct {
    ///A pointer to the device this IOCB belongs to.
    dcb_t *device;
    ///A pointer to the process this IOCB belongs to.
    struct pcb *pcb;
    ///The operation this IOCB is attempting.
    dcb_status_t operation;
    ///The length of the buffer.
    size_t buf_len;
    ///The buffer.
    char *buffer;
} iocb_t;

///The container for all device control blocks.
static dcb_t device_controllers[4] = {
        {.dev = COM1},
        {.dev = COM2},
        {.dev = COM3},
        {.dev = COM4}
};

/**
 * @brief Checks if the given character is a new line character.
 *
 * @param c the character
 * @return true if the character is a new line or carriage return, false if not.
 */
bool is_newline(char c)
{
    return c == '\n' || c == '\r';
}

/**
 * @brief Handles the new character to be inserted into the buffer.
 *        This function also handles control characters such as delete/arrow keys.
 *        Note that this will NOT handle new lines.
 *
 * @param read the character read.
 * @param buffer the buffer.
 */
void handle_new_char(char read, dcb_t *dcb)
{
    //Check if we're in 'escape' mode.
    if(dcb->escape_buf_pos > 0)
    {
        if((size_t) dcb->escape_buf_pos + 1 >= sizeof(dcb->escape_buffer))
        {
            memset(dcb->escape_buffer, 0, sizeof (dcb->escape_buffer));
            dcb->escape_buf_pos = 0;
            return;
        }

        //Just throw away brackets here.
        if(read == '[')
            return;

        dcb->escape_buffer[dcb->escape_buf_pos++] = read;

        //Try to find a match for operation.
        if(dcb->escape_buffer[1] == 'C' || dcb->escape_buffer[1] == 'D')
        {
            int diff = dcb->escape_buffer[1] == 'C' ? 1 : -1;
            if(dcb->line_pos > 0 || (diff != -1))
            {
                dcb->line_pos += diff;
                dcb->line_pos = dcb->line_pos < 0 ? 0 : dcb->line_pos;
                dcb->line_pos = (dcb->line_pos > dcb->io_bytes) ? dcb->io_bytes : dcb->line_pos;
            }
        }

        memset(dcb->escape_buffer, 0, sizeof (dcb->escape_buffer));
        dcb->escape_buf_pos = 0;
        return;
    }

    if (read >= SPACE && read <= TILDA)
    {
        //Copy the current characters forward.
        for (size_t i = dcb->io_bytes; i > dcb->line_pos; --i)
        {
            dcb->io_buffer[i] = dcb->io_buffer[i - 1];
        }

        dcb->io_buffer[dcb->line_pos++] = read;
        dcb->io_bytes++;
    }

    if(read == BACKSPACE || read == DELETE)
    {
        if(dcb->line_pos == 0 || dcb->io_bytes == 0)
            return;

        dcb->io_buffer[--dcb->line_pos] = '\0';
        dcb->io_bytes--;

        memcpy(dcb->io_buffer + dcb->line_pos, dcb->io_buffer + dcb->line_pos + 1, dcb->io_bytes - dcb->line_pos);
        dcb->io_buffer[dcb->io_bytes] = '\0';
    }

    if(read == TAB)
    {
        //Find the best match.
        const char *best = find_best_match(dcb->io_buffer);
        if(best != NULL)
        {
            size_t best_len = strlen(best);
            dcb->io_bytes = best_len;
            dcb->line_pos = (int) dcb->io_bytes;

            //Empty the buffer.
            memset(dcb->io_buffer, 0, dcb->io_requested);
            strcpy(dcb->io_buffer, best, -1);
        }
    }

    //If we've reached an escape character, start placing things into the buffer.
    if(read == ESCAPE)
    {
        dcb->escape_buf_pos = 1;
        dcb->escape_buffer[0] = ESCAPE;
    }
}

/**
 * @brief Echos the line to the output.
 * @param line the line to echo.
 * @param dcb the DCB the line belongs to.
 * @param line_pos_beginning the line position at the beginning of the read.
 */
void echo_line(char *line, dcb_t *dcb, int line_pos_beginning)
{
    if(line_pos_beginning > 0)
        move_cursor(dcb->dev, LEFT, line_pos_beginning);

    //Move it back one more.
    char clear_action[5] = {
            ESCAPE,
            '[',
            '0',
            'K',
            '\0'
    };

    serial_out(dcb->dev, clear_action, 4);

    //Get the current color.
    const color_t *clr = get_output_color();
    bool cmd_exists = false;
    if(command_formatting_enabled)
    {
        cmd_exists = command_exists(dcb->io_buffer);
        if(cmd_exists)
        {
            internal_soc(get_color("bright-green"));
        }
        else
        {
            internal_soc(get_color("red"));
        }
    }

    serial_out(dcb->dev, line, dcb->io_bytes);

    if(command_formatting_enabled)
    {
        internal_soc(clr);
    }

    if (dcb->io_bytes > 0)
        move_cursor(dcb->dev, LEFT, (int) dcb->io_bytes);

    //Get the string amount to move the cursor.
    if (dcb->line_pos > 0)
        move_cursor(dcb->dev, RIGHT, dcb->line_pos);
}

/**
 * @brief The second level input handler, used for inputs.
 *
 * @param dcb the device control block in use.
 * @return 0 if the DCB should no longer be reading. Otherwise, the amount of bytes read so far.
 */
int input_isr(dcb_t *dcb)
{
    char read = inb(dcb->dev);
    if(dcb->operation != READING)
    {
        //Full? Discard the thing then.
        if(dcb->r_buffer_len == dcb->r_buffer_size)
            return 0;

        dcb->r_buffer_start[dcb->write_index] = read;
        dcb->write_index = (dcb->write_index + 1) % (int) dcb->r_buffer_len;
        dcb->r_buffer_size++;
        return 0;
    }

    if(is_newline(read)) //End of line.
    {
        outb(dcb->dev, '\n');
        dcb->operation = IDLING;
        dcb->event = true;
        return 0;
    }

    size_t original = dcb->line_pos;
    handle_new_char(read, dcb);

    //Echo the character.
    echo_line(dcb->io_buffer, dcb, (int) original);
    if(dcb->io_bytes < dcb->io_requested)
        return 0;

    dcb->operation = IDLING;
    dcb->event = true;
    return (int) dcb->io_bytes;
}

int output_isr(dcb_t *dcb)
{
    if(dcb->operation != WRITING)
        return 0;

    if(dcb->io_bytes < dcb->io_requested)
    {
        char out = dcb->io_buffer[dcb->io_bytes++];
        outb(dcb->dev, out);
        return 0;
    }

    dcb->event = true;
    dcb->operation = IDLING;
    return (int) dcb->io_requested;
}

struct pcb *check_completed(void)
{
    //Look for DCBs that were using this PCB.
    for (int i = 0; i < 4; ++i)
    {
        dcb_t *dcb = device_controllers + i;
        struct pcb *active_pcb = dcb->pcb;
        if(!dcb->event || active_pcb == NULL) //This signifies being done, or no activity at all.
            continue;

        //Check for a pending io operation.
        if(list_size(dcb->pending_iocb) == 0)
        {
            dcb->event = false;
            dcb->pcb = NULL;
            return active_pcb;
        }

        iocb_t *iocb = (iocb_t *) remove_item_unsafe(dcb->pending_iocb, 0);

        int bytes_transferred = -1;
        if(iocb->operation == READING)
            bytes_transferred = serial_read(dcb->dev, iocb->buffer, iocb->buf_len);
        else
            bytes_transferred = serial_write(dcb->dev, iocb->buffer, iocb->buf_len);

        dcb->pcb = iocb->pcb;
        (void) bytes_transferred;
        sys_free_mem(iocb);
        return active_pcb; // This is the PCB that needs to now run as its operation was completed.
    }
    return NULL;
}

io_req_result io_request(struct pcb *pcb, op_code operation, device dev, char *buffer, size_t length)
{
    int dcb_ind = serial_devno(dev);
    if(dcb_ind == -1)
        return INVALID_PARAMS;

    if(buffer == NULL || length <= 0)
        return INVALID_PARAMS;

    if(operation != WRITE && operation != READ)
        return INVALID_PARAMS;

    dcb_t *dcb = device_controllers + dcb_ind;
    if(!dcb->allocated)
        return DEVICE_CLOSED;

    if(dcb->operation != IDLING)
    {
        //Create an IOCB and add it to the pending list.
        iocb_t *iocb = sys_alloc_mem(sizeof (iocb_t));
        memset(iocb, 0, sizeof (iocb_t));
        iocb->buf_len = length;
        iocb->buffer = buffer;
        iocb->device = dcb;
        iocb->operation = operation == WRITE ? WRITING : READING;
        iocb->pcb = pcb;

        add_item(dcb->pending_iocb, iocb);
        return DEVICE_BUSY;
    }

    dcb->pcb = pcb;
    if(operation == READ)
    {
        int read = serial_read(dev, buffer, length);
        return read < (int) length ? PARTIALLY_SERVICED : SERVICED;
    }
    else if(operation == WRITE)
    {
        int written = serial_write(dev, buffer, length);
        return written < (int) length ? PARTIALLY_SERVICED : SERVICED;
    }
    return -1; //Should never happen.
}

extern void serial_isr(void*);

/**
 * @brief The first level interrupt service routine for serial interrupts.
 */
void serial_isr_intern(void)
{
    device dev = COM1; //FIXME make this actually work with other COM types.
    int dcb_ind = serial_devno(dev);
    dcb_t *dcb = device_controllers + dcb_ind;

    //Get and switch on the interrupt ID.
    int interrupt_id = inb(dev + IIR) & 0b111;
    if((interrupt_id & 1) != 0) //Not caused by us in this case.
        return;
    interrupt_id >>= 1;

    switch (interrupt_id)
    {
        case 0b00: //Binary 00 = 0
        {
            inb(dev + MSR);
            break;
        }
        case 0b01: //Binary 01 = 1
        {
            output_isr(dcb);
            break;
        }
        case 0b10: //Binary 10 = 2
        {
            input_isr(dcb);
            break;
        }
        case 0b11: //Binary 11 = 3
        {
            inb(dev + LSR); //Read and throw away.
            break;
        }
        default: {} //No other interrupts should happen.
    }

    outb(0x20, 0x20);
}

/**
 * @brief Finds the appropriate IRQ for the given device.
 * @param dev the device.
 * @return the IRQ number.
 */
int find_com_irq(device dev)
{
    return dev == COM1 || dev == COM3 ? 4 : 3;
}

/**
 * @brief Finds the device interrupt vector.
 * @param dev the device vector.
 * @return the IV number.
 */
int find_com_iv(device dev)
{
    return dev == COM1 || dev == COM3 ? 0x24 : 0x23;
}

int serial_open(device dev, int speed)
{
    int dcb_index = serial_devno(dev);
    if(dcb_index == -1)
        return -1;

    dcb_t *dcb = device_controllers + dcb_index;

    if(dcb->allocated){
        return code_selection(-103);
    }

    if (speed == 0){
        return code_selection(-102);
    }

    dcb->dev = dev;
    dcb->allocated = true;
    dcb->event = false;
    dcb->operation = IDLING;
    dcb->r_buffer_len = RING_BUFFER_LEN;
    dcb->r_buffer_size = 0;
    dcb->r_buffer_start = sys_alloc_mem(RING_BUFFER_LEN);
    dcb->read_index = dcb->write_index = 0;
    dcb->pending_iocb = nl_unbounded();

    int com_irq = find_com_irq(dev);
    int com_iv = find_com_iv(dev);

    idt_install(com_iv, serial_isr);

    int brd = 115200 / speed; //Standard is 19200 Generally

    //Install the DCB to the PIC.
    outb(dev + LCR, 0x80);    //set line control register
    outb(dev + DLL, brd & (0xFF));    //set bsd least sig bit
    outb(dev + DLM, brd & (0xFF00));    //brd most significant bit
    outb(dev + LCR, 0x03);    //lock divisor; 8bits, no parity, one stop

    cli();
    int mask = inb(0x21);
    mask &= ~(1 << (com_irq));
    outb(0x21, mask);
    sti();
    //Note to Later --IMPLEMENT ERROR CODES 101,102,103
    outb(dev + MCR, 0x08);
    outb(dev + IER, 0x01);
    initialized[dcb_index] = 1;
    return 0;
}

int serial_close(device dev)
{
    int dev_ind = serial_devno(dev);
    if(dev_ind == -1)
        return -1;

    dcb_t *dcb = device_controllers + dev_ind;
    if(!dcb->allocated)
        return code_selection(-201); //Throw Error Serial port not open

    destroy_list(dcb->pending_iocb, true);
    dcb->allocated = 0;
    cli();
    int mask = inb(0x21);
    mask |= (1 << (find_com_irq(dev)));
    outb(0x21, mask);
    sti();

    //Disable modem control and interrupt enable.
    outb(dev + MCR, 0x00);
    outb(dev + IER, 0x00);
    return 0;
}

int serial_read(device dev, char *buf, size_t len)
{
    int dcb_ind = serial_devno(dev);
    if(dcb_ind == -1)
        return -1;

    //Get all the controllers and check all errors.
    dcb_t *dcb = device_controllers + dcb_ind;
    // ensure the port is open, if not return error -301
    if(!dcb->allocated)
        return code_selection(-301);

    if(buf == NULL)
        return code_selection(-302);

    if(len <= 0)
        return code_selection(-303);

    // ensure the status is idle, if not return error -304
    if(dcb->operation != IDLING)
        return code_selection(-304);

    //Initialize values for reading, but not the ring buffer
    dcb->event = false;
    dcb->io_buffer = buf;
    dcb->io_bytes = dcb->line_pos = 0;
    dcb->io_requested = len;
    // setting status to 'reading'
    dcb->operation = READING;
    
    //Read all available things from ring buffer.
    while(dcb->r_buffer_size > 0 &&
          dcb->io_bytes < dcb->io_requested &&
          !is_newline(dcb->io_buffer[dcb->io_bytes]))
    {
        char read = dcb->r_buffer_start[dcb->read_index];
        dcb->read_index = (dcb->read_index + 1) % RING_BUFFER_LEN;

        handle_new_char(read, dcb);
        dcb->r_buffer_size--;
    }

    if(dcb->io_bytes > 0)
        echo_line(dcb->io_buffer, dcb, 0);

    //Check if we're done.
    if(dcb->io_bytes == dcb->io_requested || is_newline(dcb->io_buffer[dcb->io_bytes]))
    {
        dcb->operation = IDLING;
        dcb->event = true;
        return (int) dcb->io_bytes;
    }

    return 0; //Signifies we need more characters.
}

int serial_write(device dev, char *buf, size_t len)
{
    int dcb_ind = serial_devno(dev);
    if(dcb_ind == -1)
        return -1;

    //Get all the controllers and check all errors.
    dcb_t *dcb = device_controllers + dcb_ind;
    // ensure port is currently open
    if(!dcb->allocated)
        return code_selection(-401);

    if(buf == NULL)
        return code_selection(-402);

    if(len <= 0)
        return code_selection(-403);

    //ensure port is idle
    if(dcb->operation != IDLING){
        return code_selection(-404);
    }
        

    // install buffer pointer and counter, and set current status to writing
    dcb->io_buffer = buf;
    dcb->io_bytes = 1;
    dcb->io_requested = len;
    dcb->event = false;
    dcb->operation = WRITING;
   
    // get first character from request buff and store it in output register
    outb(dev, buf[0]);

    //Enable the interrupts.
    int previous = inb(dev + IER);
    outb(dev + IER, previous | 0x02);
    return 0;
}

///The CLI history from the serial_poll function.
static linked_list *cli_history = NULL;

int serial_poll(device dev, char *buffer, size_t len)
{
    if (cli_history == NULL && cli_history_enabled)
    {
        cli_history = nl_unbounded();
    }

    //Check if the CLI should be forcefully disabled.
    if(cli_history_enabled && list_size(cli_history) > MAX_CLI_HISTORY_LEN)
    {
        //Free the oldest CLI history object.
        struct line_entry *item = remove_item_unsafe(cli_history, 0);
        sys_free_mem(item->line);
        sys_free_mem(item);
    }

    //Keeps track of the current line entry. Used when command line
    //history needs to swap.
    char swap[len];
    memset(swap, 0, len);
    struct line_entry current_entry = {
            .line = swap,
            .line_length = len
    };

    int cli_index = cli_history != NULL && cli_history_enabled
            ? list_size(cli_history) : 0;
    size_t bytes_read = 0;
    int line_pos = 0;

    if(prompt != NULL)
    {
        serial_out(COM1, prompt, strlen(prompt));
    }

    while (bytes_read < len)
    {
        //Check the LSR.
        //A null buffer indicates the user simply wants to poll.
        if ((inb(dev + LSR) & 1) == 0)
        {
            if(buffer == NULL)
                return 0;
            continue;
        }

        int beginning_pos = line_pos;

        char read_char = inb(dev);

        //Get the keycode and check it against known characters.
        int keycode = (int) read_char;
        char k_str[20] = {0};
        itoa(keycode, k_str, 20);

        //Check if the buffer is null.
        if(buffer == NULL)
        {
            if(keycode >= SPACE && keycode <= TILDA)
            {
                return keycode;
            }
            else
            {
                return 0;
            }
        }

        if (keycode >= SPACE && keycode <= TILDA)
        {
            //Copy the current characters forward.
            for (int i = (int) bytes_read; i > line_pos; --i)
            {
                buffer[i] = buffer[i - 1];
            }

            buffer[line_pos++] = read_char;
            bytes_read++;
        }

        if (keycode == CARRIAGE_RETURN || keycode == NEWLINE)
        {
            buffer[bytes_read] = '\0';
            break;
        }

        //Handle backspace and delete.
        if (keycode == BACKSPACE || keycode == DELETE)
        {
            if (line_pos == 0)
                continue;

            //Delete the character.
            buffer[--line_pos] = '\0';

            //Copy down the new characters.
            for (int i = 0; i < (int) bytes_read; ++i)
            {
                buffer[line_pos + i] =
                        buffer[line_pos + i + 1];
            }
            bytes_read--;
        }

        //Handle the ASCII escape function.
        if (keycode == ESCAPE)
        {
            //Get the ascii action_arr.
            char action_arr[ANSI_CODE_READ_LEN] = {0};

            //Continuously read until something matches.
            int matched = 0;
            int read_pos = 0;
            while (!matched)
            {
                //Check for more data.
                //We check every loop as ALL data from an ANSI escape isn't immediately available.
                //For example, a read might look like: <esc>000000[000A
                //All data isn't immediately available, so we have to continuously look for more data.
                if(read_pos >= ANSI_CODE_READ_LEN)
                    break;

                if ((inb(dev + LSR) & 1) != 0)
                {
                    char in = inb(dev);

                    //Throw away the bracket.
                    if(in == '[')
                        continue;

                    //If we get another escape, reset.
                    if (in == ESCAPE)
                    {
                        read_pos = 0;
                        memset(action_arr, 0, ANSI_CODE_READ_LEN);
                    }
                    else
                    {
                        action_arr[read_pos++] = in;
                    }
                }

                //Movement right or left
                if (action_arr[0] == 'C' || action_arr[0] == 'D')
                {
                    matched = 1;

                    //Adjust value then coerce.
                    line_pos += action_arr[0] == 'C' ? 1 : -1;
                    line_pos = line_pos < 0 ? 0 : line_pos;
                    line_pos = line_pos > (int) bytes_read ? (int) bytes_read : line_pos;
                }
                //Word movement right or left. (Different codes for Unix/Windows)
                else if ((action_arr[0] == 'f' || action_arr[3] == 'C') ||
                        (action_arr[0] == 'b' || action_arr[3] == 'D'))
                {
                    matched = 1;
                    int next_index =
                            find_next_word(action_arr[0] == 'b' || action_arr[3] == 'D' ? LEFT : RIGHT,
                                           line_pos,
                                           buffer,
                                           (int) bytes_read);

                    line_pos = next_index;
                }
                //Movement up and down
                else if (action_arr[0] == 'A' || action_arr[0] == 'B')
                {
                    matched = 1;

                    if(!cli_history_enabled)
                        continue;

                    int l_size = list_size(cli_history);
                    //Check if we can move in the history.
                    if (cli_history == NULL || (cli_index <= 0 && action_arr[0] == 'A')
                        || (cli_index >= l_size && action_arr[0] == 'B'))
                        continue;

                    //Get previous or future line.
                    int delta = action_arr[0] == 'A' ? -1 : 1;
                    cli_index += delta;
                    struct line_entry *l_entry = cli_index >= l_size ?
                                                 &current_entry :
                                                 get_item(cli_history, cli_index);
                    size_t copy_len = l_entry->line_length > len
                                      ? len :
                                      l_entry->line_length;

                    //Save current, load old line.
                    if (cli_index == l_size - 1 && delta == -1)
                    {
                        memcpy(current_entry.line, buffer, len);
                        current_entry.line_length = bytes_read;
                    }

                    //Zero out buffer, then copy in new string.
                    memset(buffer, 0, len);
                    memcpy(buffer, l_entry->line, copy_len);
                    buffer[copy_len] = '\0';
                    line_pos = (int) copy_len;
                    bytes_read = copy_len;
                }
                //The 'delete' key
                else if (action_arr[0] == '3' && action_arr[1] == '~')
                {
                    matched = 1;

                    if(line_pos >= (int) bytes_read)
                        continue;

                    //Delete the character.
                    buffer[line_pos] = '\0';

                    //Copy down the new characters.
                    for (int i = 0; i < (int) bytes_read; ++i)
                    {
                        buffer[line_pos + i] =
                                buffer[line_pos + i + 1];
                    }
                    bytes_read--;
                }
                //Word deletion
                else if (action_arr[0] == DELETE)
                {
                    matched = 1;

                    int delete_index = find_next_word(LEFT,
                                                      line_pos,
                                                      buffer,
                                                      (int) bytes_read);
                    int deleted = line_pos - delete_index;

                    //Copy the string down.
                    for (int i = delete_index; i < (int) line_pos; ++i)
                    {
                        buffer[i] = buffer[i + deleted];
                        buffer[i + deleted] = '\0';
                    }

                    line_pos -= deleted;
                    bytes_read -= deleted;
                }
                //This acts as a 'catch-all' for all input-able ansi escape codes.
                else if((action_arr[0] >= 'A' && action_arr[0] <= 'Z' && action_arr[0] != 'O') ||
                        action_arr[read_pos - 1] == '~' ||
                        (action_arr[0] == 'O' && action_arr[1] != 0))
                {
                    matched = 1;
                }
            }
        }

        //Check if it was a TAB completion.
        if(keycode == TAB && tab_completions)
        {
            //Find the best match.
            const char *best = find_best_match(buffer);
            if(best != NULL)
            {
                size_t best_len = strlen(best);
                bytes_read = best_len;
                line_pos = (int) bytes_read;

                //Empty the buffer.
                memset(buffer, 0, len);
                strcpy(buffer, best, -1);
            }
        }

        if(cli_invisible)
            continue;

        //Reset the line.
        if (beginning_pos > 0)
            move_cursor(dev, LEFT, beginning_pos);

        //Move it back one more.
        char clear_action[5] = {
                ESCAPE,
                '[',
                '0',
                'K',
                '\0'
        };

        serial_out(dev, clear_action, 4);

        //Get the current color.
        const color_t *clr = get_output_color();
        bool cmd_exists = false;
        if(command_formatting_enabled)
        {
            cmd_exists = command_exists(buffer);
            if(cmd_exists)
            {
                internal_soc(get_color("bright-green"));
            }
            else
            {
                internal_soc(get_color("red"));
            }
        }

        serial_out(dev, buffer, bytes_read);

        if (bytes_read > 0)
            move_cursor(dev, LEFT, (int) bytes_read);

        //Get the string amount to move the cursor.
        if (line_pos > 0)
            move_cursor(dev, RIGHT, line_pos);

        if(command_formatting_enabled)
        {
            internal_soc(clr);
        }
    }

    //Allocate the line for storage.
    if (cli_history != NULL && cli_history_enabled)
    {
        //Allocate memory and store string.
        struct line_entry *to_store = sys_alloc_mem(sizeof(struct line_entry));
        char *store_line = sys_alloc_mem(bytes_read);
        memcpy(store_line, buffer, bytes_read);

        to_store->line = store_line;
        to_store->line_length = bytes_read;
        add_item_index(cli_history, list_size(cli_history), to_store);
    }
    serial_out(dev, "\n", 1);
    return (int) bytes_read;
}
