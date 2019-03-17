#ifndef SHELL_SYSTEM_H_
#define SHELL_SYSTEM_H_

int         SHELL_COMMAND_system(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE   SHELL_SYSTEM_printConfig(SYSTEM_CONFIG* config);

#endif