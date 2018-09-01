#ifndef __TEMPLATE_H
#define __TEMPLATE_H
#include <stm32f10x.h>

typedef enum
{    
    FALSE = (u8)0,
    TRUE = !FALSE
}bool;

#define Is_BOOL(bTest)  ((bTest == TRUE) || (bTest == FALSE))


#endif
