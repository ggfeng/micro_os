//#include  <lib/shell/shell.h>


#include <yodalite_autoconf.h>

#ifndef CONFIG_LIBC_ENABLE
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#else
#include <lib/libc/yodalite_libc.h>
#endif

#include <lib/list/list.h>
#include  <lib/shell/shell.h>

#define CTL_CH(c)		    ((c) - 'a' + 1)
#define CTL_BACKSPACE		    ('\b')
#define DEL			    ((int8_t)255)
#define DEL7			    ((int8_t)127)
#define _EOF                    (0xff)

#define BEGINNING_OF_LINE()                   \
{			                      \
  while (num)                                 \
  {                                           \
     getcmd_putch(CTL_BACKSPACE);	      \
     num--;				      \
  }					      \
}

#define ERASE_TO_EOL()                                       \
{				                             \
    if (num < eol_num)                                       \
    {                                                        \
        int32_t i,n=(int32_t)(eol_num - num);                \
        for(i=0;i<n;i++)                                     \
          getcmd_putch(' ');                                 \
	do                                                   \
        {                                                    \
	  getcmd_putch(CTL_BACKSPACE);	                     \
	} while (--eol_num > num);		             \
    }						             \
}

#define REFRESH_TO_EOL()                                     \
{                                                            \
  if (num < eol_num)                                         \
  {                                                          \
		wlen = eol_num - num;		             \
		putnstr(buf + num, wlen);	             \
		num = eol_num;			             \
   }					                     \
}

#define putnstr(str,n)	                            \
do{                                                 \
   int i = (int)n;                                  \
   int8_t *p = (int8_t *)str;                       \
for(i=0;i<n;i++){                                   \
   getcmd_putch(*p++);}} while(0)

static cmd_tbl_t g_cmd_head;
static int8_t   console_buffer[CONFIG_SYS_CBSIZE + 1];/* console I/O buffer	*/



static shell_hook_t shell_hook;

int yodalite_register_shell_hook(shell_hook_t func)
{
    if(func)
     shell_hook = func;

    return 0;
}

void  yodalite_unregister_shell_hook(void)
{
      shell_hook  = NULL;
}

extern int __wrap_getchar(void);
extern int __wrap_putchar(int c);

static int32_t getcmd_getch(void)
{
    int8_t c;

#if CONFIG_PLATFORM_UART_INTERFACE == 1
    while((c = __wrap_getchar()) == _EOF)
#else
    while((c = getchar()) == _EOF)
#endif
    { 
       if(shell_hook)
          shell_hook(); 
    }

    return c;
}


static int32_t getcmd_putch(int8_t ch)
{
#if CONFIG_PLATFORM_UART_INTERFACE == 1
   return __wrap_putchar(ch);
#else
   return putchar(ch);
#endif
}

static void cread_add_char(int8_t ichar, int32_t insert, uint32_t *num,uint32_t *eol_num,int8_t * buf,uint32_t len)
{
    uint32_t wlen;

    if (insert || *num == *eol_num)
    {
        if (*eol_num > len - 1)
        {
            return;
        }
        (*eol_num)++;
    }

    if (insert)
    {
        wlen = *eol_num - *num;
        if (wlen > 1)
        {
            memmove ((void*)&buf[*num + 1],(void*)&buf[*num],(size_t) (wlen - 1));
        }

        buf[*num] = ichar;
        putnstr(buf + *num, wlen);
         
        (*num)++;

        while (--wlen)
        {
            getcmd_putch(CTL_BACKSPACE);
        }

    }
    else
    {
        wlen = 1;
        buf[*num] = ichar;
        putnstr(buf + *num, wlen);
        (*num)++;
    }
}

static void cread_add_str(int8_t *str,int32_t strsize,int32_t insert, uint32_t *num,uint32_t * eol_num,int8_t * buf,uint32_t  len)
{
    while (strsize--)
    {
        cread_add_char(*str, insert, num, eol_num, buf, len);
        str++;
    }
}

static int32_t cread_line(const int8_t *const prompt, int8_t * buf,uint32_t *len)
{
    uint32_t num = 0;
    uint32_t eol_num = 0;
    uint32_t wlen;
    int8_t  ichar;
    int32_t insert = 1;
    int32_t esc_len = 0;
    int8_t  esc_save[8];
    int32_t init_len = strlen (buf);
//    int32_t first = 1;

    if (init_len)
        cread_add_str(buf, init_len, 1, &num, &eol_num, buf, *len);

    while (1)
    {
        ichar = getcmd_getch();

        if ((ichar == '\n') || (ichar == '\r'))
        {
            getcmd_putch('\n');
            break;
        }

        if (esc_len != 0)
        {
            if (esc_len == 1)
            {
                if (ichar == '[')
                {
                    esc_save[esc_len] = ichar;
                    esc_len = 2;
                }
                else
                {
                    cread_add_str(esc_save, esc_len, insert,
                                  &num, &eol_num, buf, *len);
                    esc_len = 0;
                }
                continue;
            }

            switch (ichar)
            {

            case 'D':
                ichar = CTL_CH('b');
                esc_len = 0;
                break;
            case 'C':
                ichar = CTL_CH('f');
                esc_len = 0;
                break;
            case 'H':
                ichar = CTL_CH('a');
                esc_len = 0;
                break;
            case 'A':
                ichar = CTL_CH('p');
                esc_len = 0;
                break;
            case 'B':
                ichar = CTL_CH('n');
                esc_len = 0;
                break;
            default:
                esc_save[esc_len++] = ichar;
                cread_add_str(esc_save, esc_len, insert,
                              &num, &eol_num, buf, *len);
                esc_len = 0;
                continue;
            }
        }
        switch (ichar)
        {
        case 0x1b:
            if (esc_len == 0)
            {
                esc_save[esc_len] = ichar;
                esc_len = 1;
            }
            else
            {
                printf ("impossible condition #876\n");
                esc_len = 0;
            }
            break;

        case CTL_CH('a'):
            BEGINNING_OF_LINE();
            break;
        case CTL_CH('c'):
            *buf = '\0';
            return (-1);
        case CTL_CH('f'):
            if (num < eol_num)
            {
                getcmd_putch(buf[num]);
                num++;
            }
            break;
        case CTL_CH('b'):
            if (num)
            {
                getcmd_putch(CTL_BACKSPACE);
                num--;
            }
            break;
        case CTL_CH('d'):
            if (num < eol_num)
            {
                wlen = eol_num - num - 1;
                if (wlen)
                {
                    memmove (&buf[num], &buf[num + 1], wlen);
                    putnstr(buf + num, wlen);
                }

                getcmd_putch(' ');
                do
                {
                    getcmd_putch(CTL_BACKSPACE);
                }
                while (wlen--);
                eol_num--;
            }
            break;
        case CTL_CH('k'):
            ERASE_TO_EOL();
            break;
        case CTL_CH('e'):
            REFRESH_TO_EOL();
            break;
        case CTL_CH('o'):
            insert = !insert;
            break;
        case CTL_CH('x'):
        case CTL_CH('u'):
            BEGINNING_OF_LINE();
            ERASE_TO_EOL();
            break;
        case DEL:
        case DEL7:
        case 8:
            if (num)
            {
                wlen = eol_num - num;
                num--;
                memmove (&buf[num], &buf[num + 1], wlen);
                getcmd_putch(CTL_BACKSPACE);
                putnstr(buf + num, wlen);
                getcmd_putch(' ');
                do
                {
                    getcmd_putch(CTL_BACKSPACE);
                }
                while (wlen--);
                eol_num--;
            }
            break;
        default:
            cread_add_char(ichar, insert, &num, &eol_num, buf, *len);
            break;
        }
    }
    *len = eol_num;
    buf[eol_num] = '\0';

    return 0;
}

static int32_t readline_into_buffer(const int8_t *const   prompt,int8_t* buffer,uint32_t size)
{
    int32_t rc;
    int8_t *p = buffer;
    uint32_t len = size;

    if (prompt)
        printf ("%s", prompt);


    rc = cread_line(prompt, p, &len);
    return rc < 0 ? rc : len;
}

int8_t* readline (const int8_t *const prompt)
{
    int32_t ret = 0;

    memset (console_buffer, 0, sizeof(console_buffer));
    ret = readline_into_buffer(prompt, console_buffer,sizeof(console_buffer)-1);
    return (ret < 0) ? NULL : console_buffer;
}

static int32_t cmd_init_head(cmd_tbl_t* head)
{
    INIT_LIST_HEAD(&(head->list));
    return OK;
}

static int32_t cmd_add(cmd_tbl_t* head, cmd_tbl_t* cmd)
{
    list_add(&(cmd->list), &(head->list));
    return OK;
}

static int32_t cmd_add_prev(cmd_tbl_t* head, cmd_tbl_t* cmd)
{
    list_add_tail(&(cmd->list), &(head->list));
    return OK;
}

static cmd_tbl_t *cli_find_cmd (const int8_t *name)
{
    cmd_tbl_t* pos;
    const cmd_tbl_t* head = &g_cmd_head;

    list_for_each_entry(pos, &(head->list), list){
        if(strcmp (pos->name, name)==0)
          return pos;
    }

    return NULL;
}

static int32_t cli_cmd_usage(const cmd_tbl_t *cmdtp)
{
      printf ("\t%s - %s\r\n", cmdtp->name, cmdtp->help);
  //  printf ("\r\t%s - %s\r\n", cmdtp->name, cmdtp->help);

    return OK;
}

static int32_t cli_allcmd_print(void)
{
    cmd_tbl_t* pos;
    const cmd_tbl_t* head = &g_cmd_head;

    list_for_each_entry(pos, &(head->list), list){
        if(cli_cmd_usage(pos)== ERROR)
           return ERROR;
    }
    return OK;
}

static int32_t cli_cmd_call(cmd_tbl_t*  cmdtp,int32_t argc, int8_t* const argv[])
{
    int32_t result;

    result = (cmdtp->cmd)(argc, argv);

    return result;
}

static int  cli_cmd_process(int32_t argc, int8_t* const     argv[])
{
    int rc = 0;
    cmd_tbl_t *cmdtp;

    cmdtp = cli_find_cmd(argv[0]);

    if (cmdtp == NULL) {
        cli_allcmd_print();
        return ERROR;
    }

    if (argc > cmdtp->maxargs)
        rc = cli_cmd_usage(cmdtp);
    else
        rc = cli_cmd_call(cmdtp,argc, argv);

    return rc;
}

static int32_t cli_parse_line (int8_t *line, int8_t *argv[])
{
    int32_t nargs = 0;

    while (nargs < CONFIG_SYS_MAXARGS)
    {
        while (isblank((int)(*line)))
            ++line;

        if (*line == '\0') 
        {
            argv[nargs] = NULL;
            return (nargs);
        }

        argv[nargs++] = line; 

    
        while (*line && !isblank((int)(*line)))
            ++line;

        if (*line == '\0')
        {
            argv[nargs] = NULL;
            return (nargs);
        }

        *line++ = '\0';  
    }

    return (nargs);
}

static int32_t builtin_run_cmd(int8_t *cmd)
{
    int32_t argc;
    int8_t  *argv[CONFIG_SYS_MAXARGS + 1];

    if (!cmd || !*cmd){
        return ERROR;
    }

    if (strlen (cmd) >= CONFIG_SYS_CBSIZE){
        return ERROR;
    }

    if ((argc = cli_parse_line(cmd, argv)) == 0){
        return ERROR;
    }

    return cli_cmd_process(argc, argv);
}

int32_t yodalite_reg_cmd(cmd_tbl_t* cmd)
{
    cmd_tbl_t* pos;
    int32_t ret = 0;

    cmd_tbl_t* head = &g_cmd_head;

    if(list_empty(&(head->list)) == TRUE){
        return(cmd_add(head, cmd));
    }

    list_for_each_entry(pos, &(head->list), list)
    {
      ret = strcmp (cmd->name, pos->name);

      if(ret == 0){
          return ERROR;
      }
      else if(ret > 0){
          if(list_is_last(&(pos->list), &(head->list)) == TRUE){
              return(cmd_add(pos,cmd));
          }
          else{
              continue;
          }
      }
      else{
          return(cmd_add_prev(pos, cmd));
      }
    }
   return 0;
}

static int32_t do_help(int32_t argc, int8_t* const argv[])
{
    if(strcmp(argv[0], "help") == 0){
        if(argc == 1)
            cli_allcmd_print();
        else{
            if(cli_cmd_usage(cli_find_cmd(argv[1])) != OK){
                printf("Command: %s - Not found!\r\n", argv[1]);
            }
        }
    }
    else{
       printf("Command Error!\r\n");
    }
    return OK;
}

int32_t yodalite_cmd_init(void)
{
   cmd_init_head(&g_cmd_head);

   YODALITE_REG_CMD(help, 2, do_help, "help Command");

   return OK;
}

int32_t yodalite_run_cmd(void)
{
  char* cmd = NULL;

  while(TRUE){
    cmd = readline(CONFIG_SYS_PROMPT);

    if(cmd != NULL){
        builtin_run_cmd(cmd);
    }
  }

  return 0;
}
