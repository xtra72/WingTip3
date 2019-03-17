#ifndef SHELL_DEVICE_H_
#define SHELL_DEVICE_H_

int         SHELL_COMMAND_device(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE   SHELL_DEVICE_printConfig(DEVICE_CONFIG* config);

#endif