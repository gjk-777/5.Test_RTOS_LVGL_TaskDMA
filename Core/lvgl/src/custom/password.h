#ifndef __PASSWORD_H__
#define __PASSWORD_H__

#include "string.h"
#include "gui_guider.h"
extern char pwstr[7];
extern uint8_t  light_high;
#define PWcorrect "1234"
void remove_last_char(char *str);

void input_password(uint32_t id, const char *arg);

#endif
