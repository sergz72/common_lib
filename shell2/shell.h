#ifndef _SHELL_H_
#define _SHELL_H_

#define SHELL_UP_KEY   0x1F
#define SHELL_DOWN_KEY 0x1E

typedef int (*printf_func)(const char *, ...);

typedef struct {
  const char *name, *help;
  unsigned int parameter_mask;
  int (*handler)(printf_func pfunc, int argc, char** argv);
} ShellCommand;

#ifdef __cplusplus
extern "C" {
#endif

void shell_init(printf_func pfunc);
int shell_register_command(const ShellCommand *command);
int shell_execute(const char *command);
const char *shell_get_prev_from_history(void);
const char *shell_get_next_from_history(void);
void shell_handler(void);
int shell_getch(void);
void shell_process_char(char c);

#ifdef __cplusplus
}
#endif

extern char command_line[];

#endif
