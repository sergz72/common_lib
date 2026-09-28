#include "board.h"
#include <shell.h>
#include <string.h>
#include <getstring.h>
#include <stdio.h>
#ifdef SHELL_HISTORY_SIZE
#include <fixed_queue.h>
#endif

static const ShellCommand *commands[MAX_SHELL_COMMANDS];
static int registered_commands;
static char argvv[MAX_SHELL_COMMAND_PARAMETERS][MAX_SHELL_COMMAND_PARAMETER_LENGTH+1], *argv[MAX_SHELL_COMMAND_PARAMETERS];
static printf_func pfunc;
#ifdef SHELL_HISTORY_SIZE
static char history[SHELL_HISTORY_ITEM_LENGTH*SHELL_HISTORY_SIZE];
static char history_temp[SHELL_HISTORY_ITEM_LENGTH];
static FixedQueue history_q;
static int history_offset;
#endif

static unsigned char *rx_buffer_write_p, *rx_buffer_read_p;
static unsigned char rx_buffer[SHELL_RX_BUFFER_LENGTH];
char command_line[COMMAND_LINE_LENGTH];

void shell_init(printf_func _pfunc)
{
  int i;
  pfunc = _pfunc;
  registered_commands = 0;
  for (i = 0; i < MAX_SHELL_COMMAND_PARAMETERS; i++)
    argv[i] = argvv[i];

  rx_buffer_write_p = rx_buffer_read_p = rx_buffer;

#ifdef SHELL_HISTORY_SIZE
  fixed_queue_init(&history_q, SHELL_HISTORY_SIZE, SHELL_HISTORY_ITEM_LENGTH, history);
  history_offset = 0;
#endif
}

int shell_register_command(const ShellCommand *command)
{
  if (registered_commands >= MAX_SHELL_COMMANDS)
    return 1;

  commands[registered_commands++] = command;

  return 0;
}

#define MODE_PARAMETER 1
#define MODE_STRING 2
#define MODE_SPACE 0

static int buildCommandParameters(const char *command)
{
  int length, mode, parameter_count;
  const char *argp = command;
  length = mode = parameter_count = 0;
  while (*command)
  {
    switch (mode)
    {
      case MODE_SPACE:
        if (*command == '"')
        {
          if (parameter_count >= MAX_SHELL_COMMAND_PARAMETERS)
            return -1;
          argp = command + 1;
          mode = MODE_STRING;
        }
        else if (*command > ' ')
        {
          if (parameter_count >= MAX_SHELL_COMMAND_PARAMETERS)
            return -1;
          argp = command;
          mode = MODE_PARAMETER;
        }
        command++;
        break;
      case MODE_PARAMETER:
        if (*command <= ' ')
        {
          if (length > MAX_SHELL_COMMAND_PARAMETER_LENGTH)
            return -1;
          memcpy(argv[parameter_count++], argp, command - argp);
          mode = 0;
        }
        command++;
        break;
      default: // MODE_STRING
        if (*command == '"')
        {
          if (length > MAX_SHELL_COMMAND_PARAMETER_LENGTH)
            return -1;
          memcpy(argv[parameter_count++], argp, command - argp);
          mode = 0;
        }
        command++;
        break;
    }
  }
  switch (mode)
  {
    case MODE_SPACE:
      return parameter_count;
    case MODE_PARAMETER:
      if (length > MAX_SHELL_COMMAND_PARAMETER_LENGTH)
        return -1;
      memcpy(argv[parameter_count], argp, command - argp);
      return parameter_count + 1;
    default:
      return -1;
  }
}

static const char * get_help_text(const ShellCommand *current_command)
{
  if (current_command->help)
    return current_command->help;
  return current_command->name;
}

int shell_execute(const char *command)
{
  if (!command)
    return 1;

  int parameter_count = buildCommandParameters(command);
  if (!parameter_count)
    return 0;
  if (parameter_count < 0)
    return parameter_count;

#ifdef SHELL_HISTORY_SIZE
  int idx = fixed_queue_get_index(&history_q, (void*)command, (int (*)(void*, void*))strcmp);
  if (idx < 0)
    fixed_queue_push(&history_q, (void*)command);
  else
    fixed_queue_move_to_top(&history_q, idx, history_temp);
  history_offset = 0;
#endif

  const ShellCommand **current_command = commands;

  if (!strcmp(argv[0], "help"))
  {
    pfunc("usage:");
    for (int i = 0; i < registered_commands; i++)
    {
      pfunc("  %s\n", get_help_text(*current_command));
      current_command++;
    }
    return 0;
  }

  parameter_count--;
  for (int i = 0; i < registered_commands; i++)
  {
    const ShellCommand *cmd = *current_command++;
    if (!strcmp(cmd->name, argv[0]))
    {
      if ((cmd->parameter_mask & (1 << parameter_count)) != 0)
        return cmd->handler(pfunc, parameter_count, &argv[1]);
      pfunc("incorrect number of parameters\n");
      return -1;
    }
  }

  pfunc("unknown command\n");
  return 1;
}

const char *shell_get_prev_from_history(void)
{
#ifdef SHELL_HISTORY_SIZE
  int s = fixed_queue_size(&history_q);
  if (history_offset < s)
    return fixed_queue_peekn(&history_q, history_offset++);
  return NULL;
#else
  return NULL;
#endif
}

const char *shell_get_next_from_history(void)
{
#ifdef SHELL_HISTORY_SIZE
  if (history_offset)
  {
    history_offset--;
    return fixed_queue_peekn(&history_q, history_offset);
  }
  return NULL;
#else
  return NULL;
#endif
}

int shell_getch(void)
{
  if (rx_buffer_write_p != rx_buffer_read_p)
  {
    char c = (char)*rx_buffer_read_p++;
    if (rx_buffer_read_p == rx_buffer + SHELL_RX_BUFFER_LENGTH)
      rx_buffer_read_p = rx_buffer;
    return c;
  }
  return EOF;
}

void shell_handler(void)
{
  int rc;

  if (!getstring_next())
  {
    switch (command_line[0])
    {
    case SHELL_UP_KEY:
      PUTS_FUNC("\r\33[2K$ ");
      getstring_buffer_init(shell_get_prev_from_history());
      break;
    case SHELL_DOWN_KEY:
      PUTS_FUNC("\r\33[2K$ ");
      getstring_buffer_init(shell_get_next_from_history());
      break;
    default:
      rc = shell_execute(command_line);
      if (rc == 0)
        PUTS_FUNC("OK\r\n$ ");
      else if (rc < 0)
        PUTS_FUNC("Invalid command line\r\n$ ");
      else
        PRINTF_FUNC("shell_execute returned %d\n$ ", rc);
      break;
    }
  }
}

void shell_process_char(char c)
{
  *rx_buffer_write_p++ = c;
  if (rx_buffer_write_p == rx_buffer + SHELL_RX_BUFFER_LENGTH)
    rx_buffer_write_p = rx_buffer;
}