#ifndef __BT24_APPLICATION_H__
#define __BT24_APPLICATION_H__

#include "asrpro_application.h"
#include "cJSON.h"
#include "globals.h"
#include "log.h"
#include "pwm_application.h"
#include "settings_handler.h"
#include "sys.h"

// 额外命令缓冲区大小
#define EXTRA_CMD_BUFF 24

uint8_t BT24_FindValidJson(char *buffer, uint16_t length, char *json_str);
void BT24_PubData(PubDataTypeDef *pub_data);
int8_t BT24_ParseCmd(const char *json_str);
uint8_t BT24_ParseData(const char *json_str, PubDataTypeDef *pub_data, char *extra_cmd);
void BT24_ProcessCommand(void);

#endif