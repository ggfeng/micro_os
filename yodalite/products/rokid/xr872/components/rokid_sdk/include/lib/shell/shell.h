#ifndef _SHELL_H_
#define _SHELL_H_
#include <lib/list/list.h>

#ifndef int8_t
#define int8_t char
#endif

#ifndef int32_t
#define int32_t   int
#endif

#ifndef TRUE 
#define TRUE (1)
#endif

#ifndef ERROR
#define ERROR -1
#endif

#ifndef OK
#define OK 0
#endif

#ifndef isblank
#define isblank(c)	(c == ' ' || c == '\t')
#endif

#define CONFIG_SYS_PROMPT		"yodalite#" 
#define CONFIG_SYS_CBSIZE		 512
#define CONFIG_SYS_MAXARGS               16

typedef int32_t (*cmd_func_t)(int32_t argc, int8_t* const argv[]);

typedef struct cmd_tbl_s
{
    int8_t*               name;       /* Command Name                 */
    int32_t               maxargs;    /* maximum number of arguments  */
    cmd_func_t            cmd;        /* command call function        */
    int8_t*               help;       /* Help  message    (short)     */
    struct list_head      list;       /** Command List                */
}cmd_tbl_t;


#define YODALITE_REG_NAME_TYP(_NAME) CMD_LIST_##_NAME
#define YODALITE_REG_NAME_STR(_NAME) #_NAME


#define YODALITE_REG_CMD(_name, _maxargs, _cmd, _help)		            \
do{                                                                         \
   static cmd_tbl_t YODALITE_REG_NAME_TYP(_name) =                               \
   {                                                                        \
    .name       = YODALITE_REG_NAME_STR(_name),                                  \
    .maxargs    = _maxargs,                                                 \
    .cmd        = _cmd,                                                     \
    .help       = _help,                                                    \
   };                                                                       \
   yodalite_reg_cmd(&(YODALITE_REG_NAME_TYP(_name)));                                \
}while(0)

typedef int (*shell_hook_t)(void);

extern  void  yodalite_unregister_shell_hook(void);
extern int    yodalite_register_shell_hook(shell_hook_t func);

extern int32_t yodalite_run_cmd(void);
extern int32_t yodalite_cmd_init (void);
extern int32_t yodalite_reg_cmd(cmd_tbl_t* cmd);

extern int8_t* readline (const int8_t *const prompt);
#endif
