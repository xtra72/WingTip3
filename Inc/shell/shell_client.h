#ifndef SHELL_CLIENT_H_
#define SHELL_CLIENT_H_

int         SHELL_COMMAND_client(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE   SHELL_CLIENT_printConfig(CLIENT_CONFIG* config);

#endif