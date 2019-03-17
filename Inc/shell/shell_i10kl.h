#ifndef SHELL_I10KL_H_
#define SHELL_I10KL_H_

int         SHELL_COMMAND_i10kl(char *argv[], uint32_t argc, struct _SHELL_COMMAND const* command);
RET_VALUE   SHELL_I10KL_printConfig(ME_I10KL_CONFIG* config);

#endif