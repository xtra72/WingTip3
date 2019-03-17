#ifndef SHELL_LOG_H_
#define SHELL_LOG_H_

int         SHELL_COMMAND_log(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE   SHELL_LOG_printConfig(LOG_CONFIG* config);

#endif