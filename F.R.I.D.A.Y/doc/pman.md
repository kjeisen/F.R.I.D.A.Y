@mainpage F.R.I.D.A.Y.

NOTE: Run 
``
doxygen Doxyfile
``
to generate full documentation.

# F.R.I.D.A.Y. Programmer Manual R3/R4

### Contents
1. Overview
2. OS Lifecycle
3. Extending Systems
   1. `kmain()` and Startup
   2. The Command Handler
   3. Registering a Command
      * Command Function
      * Adding Command to Help
4. Conclusion

## 1. Overview

F.R.I.D.A.Y. is a light-weight OS built to run on QEMU. 
You can use this documentation to extend the existing systems and
add more functionality.

## 2. OS Lifecycle

When the OS kernel is booted, the first function `kmain()` is called. This function
bootstraps most of the core functionality of the OS. Once bootstrapping is done, control
is passed to `comhand()` for the command handler. 

Once the command handler has finished, `comhand()` will return, thus giving control back to
`kmain()`. `kmain()` then begins the shutdown process and exits.

## 3. Extending Systems

### 3.i. kmain() and Startup

`kmain()` is the first function called after the bootloader for the OS.
This function is located in [kmain.c](../kernel/kmain.c) and is responsible for bootstrapping most of the OS' core functionality.
After all core systems have been initialized, full control is passed to the `comhand()` function in [comhand.c](../kernel/comhand.c).
If something needs to be initialized, put the method call for it before the call to `comhand()`.

### 3.ii. The Command Handler

`comhand()` is what defines the OS' command handling system.
When `kmain()` calls this function, the command handler welcomes the user and begins listening for user input.
The command handler requests user input via a `sys_req()` call. 
The input gathered from this method is then used to run the command that matches the input, if any.

### 3.iii. Registering a Command

All commands are 'registered' via the `comm_funcs` array inside [comhand.c](../kernel/comhand.c).
This array contains pointers to functions that follow the format:

    bool cmd_((COMMAND_NAME))(const char *command);

Note that the name of the method is **not** required to be followed, but should to maintain convention.
Any new command **should** be placed in user space, preferably in the [commands.c](../user/commands.c) file. 
The return value of the function should signify if the command matched the **label** of the command. 
i.e. the command `help junk-option1 junk-option2` should still return true for the help command, even though the options are not valid. 

##### Command Function

The start of a command function should resemble:

    bool cmd_name(const char *comm)
    {
        const char *label = "name";

        if (!matches_cmd(comm, label))
            return false;

        //cmd logic
        return true;
    }

Use the `matches_cmd(const char *cmd, const char *label)` function to check if the command's label matches.

##### Adding Command to Help

Once you've added a command, you should add a help message for it. 
Use the `help_messages` array to add an instance of the `help_info` struct.
Doing so should resemble:

    {.str_label = "name", .help_message = "The %s command does X and then does Y.\nYou should include Z arguments in THIS format!"}

After adding this, running `help name` command will then recognize the added struct and
return the `help_message` formatted with the command's name.

### 4. Conclusion

The information above covers most important information on how to extend F.R.I.D.A.Y. 
Please use the included Doxygen documentation for more information on how the internal systems work.
If you'd like to learn how to use the system from a user's perspective, please refer to our [User Manual](UserManual.docx.pdf)
