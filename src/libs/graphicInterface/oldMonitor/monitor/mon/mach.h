#ifndef mach_h
#define mach_h

#include <include/bstring.h>
#include <libs/support/misc/general.h>

#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/graphicInterface/support/graphics/rect_list.h>

#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event_codes.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event.h>

typedef FUNCTION_POINTER(void,CharHandlerFunc,(char));
typedef FUNCTION_POINTER(void,RedrawFunc,(RectList));
typedef FUNCTION_POINTER(void,ResizeFunc,(Point));

EXTERN(void,startScreenEvents,(ResizeFunc, RedrawFunc, CharHandlerFunc));
EXTERN(void,stopScreenEvents,(void));
EXTERN(void,getScreenEvent,(void));
EXTERN(Boolean,readyScreenEvent,(void));
EXTERN(Boolean, flushScreenEvents,(void));

#endif 
