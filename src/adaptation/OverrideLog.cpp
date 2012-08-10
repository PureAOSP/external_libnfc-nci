/*****************************************************************************
**
**  Name:           OverrideLog.cpp
**
**  Description:    Override the ALOGD(), ALOGE(), and other logging macros
**                  from /system/core/include/cutils/log.h
**
**  Copyright (c) 2012, Broadcom Corp., All Rights Reserved.
**  Proprietary and confidential.
**
*****************************************************************************/
#include "OverrideLog.h"
#include <cutils/properties.h>
#include "config.h"


/*******************************************************************************
**
** Function:        initializeGlobalAppLogLevel
**
** Description:     Initialize and get global logging level from .conf or
**                  Android property nfc.app_log_level.  The Android property
**                  overrides .conf variable.
**
** Returns:         Global log level:
**                  BT_TRACE_LEVEL_NONE    0        * No trace messages to be generated
**                  BT_TRACE_LEVEL_ERROR   1        * Error condition trace messages
**                  BT_TRACE_LEVEL_WARNING 2        * Warning condition trace messages
**                  BT_TRACE_LEVEL_API     3        * API traces
**                  BT_TRACE_LEVEL_EVENT   4        * Debug messages for events
**                  BT_TRACE_LEVEL_DEBUG   5        * Debug messages (general)
**
*******************************************************************************/
unsigned char initializeGlobalAppLogLevel ()
{
    unsigned long num = 0;
    char valueStr [PROPERTY_VALUE_MAX] = {0};

    GetNumValue (NAME_APPL_TRACE_LEVEL, &num, sizeof(num));
    appl_trace_level = (unsigned char) num;

    int len = property_get ("nfc.app_log_level", valueStr, "");
    if (len > 0)
    {
        //let Android property override .conf variable
        sscanf (valueStr, "%lu", &num);
        appl_trace_level = (unsigned char) num;
    }
    ALOGD ("%s: level=%u", __FUNCTION__, appl_trace_level);
    return appl_trace_level;
}
