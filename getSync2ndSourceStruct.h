
/*=================================================================================
* @file					epm_register_handler.h
* @brief				itk user exits function declation, to register custom handlers
* @date					2009/2/13
* @author				Ray Li
* @history
* ===================================================================================
* Date					Name				Description
* 13-Feb-2009			Ray				created
*===================================================================================*/

#ifndef GETSYNC2NDSOURCESTRUCT
#define GETSYNC2NDSOURCESTRUCT
#include <string>
#include <iostream>
#include "search.h"
#include "util.h"
#ifdef __cplusplus
extern "C" {
#endif


    extern __declspec(dllexport) string getSync2ndSourceStruct(string params);

#ifdef __cplusplus
}
#endif

#endif 