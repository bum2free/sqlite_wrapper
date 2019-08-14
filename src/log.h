#ifndef __LOG_H__
#define __LOG_H__

#ifdef USE_TB_LOG

#include "base_library.h"

#else

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

#define STMT(stuff) do { stuff; } while(0)

#define TB_EMERG      0
#define TB_ERROR      3
#define TB_WARNING    4
#define TB_INFO       6
#define TB_DEBUG      7

#define TB_LOG(level, msg, args...) \
    STMT(fprintf(stderr, "<%d>" __FILENAME__ ":%d - " msg "\n", level, __LINE__, ## args))

#define TB_LOG_EMERG(msg,args...) STMT(TB_LOG(LOG_EMERG, msg, ## args);)
#define TB_LOG_ERROR(msg,args...) STMT(TB_LOG(TB_ERROR, msg, ## args);)
#define TB_LOG_WARNING(msg,args...) STMT(TB_LOG(TB_WARNING, msg, ## args);)
#define TB_LOG_INFO(msg,args...) STMT(TB_LOG(TB_INFO, msg, ## args);)
#define TB_LOG_DEBUG(msg,args...)  STMT(TB_LOG(TB_DEBUG, msg, ## args);)

#endif  //ifdef USE_TB_LOG

#endif
