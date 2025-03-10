/*
    see copyright notice in squirrel.h
*/
#ifndef _SQUIRRELEX_H_
#define _SQUIRRELEX_H_

#include <squirrel.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API SQRESULT sq_throwerrorf(HSQUIRRELVM v,const SQChar *err,...);
SQUIRREL_API SQRESULT sq_pushstringf(HSQUIRRELVM v,const SQChar *s,...);
SQUIRREL_API SQRESULT sq_vpushstringf(HSQUIRRELVM v,const SQChar *s,va_list l);
SQUIRREL_API SQRESULT sq_getnativeclosurepointer(HSQUIRRELVM v,SQInteger idx,SQFUNCTION *f);
SQUIRREL_API SQRESULT sq_arrayreserve(HSQUIRRELVM v,SQInteger idx,SQInteger newcap);
SQUIRREL_API void sq_newarrayex(HSQUIRRELVM v,SQInteger capacity);
SQUIRREL_API SQInteger sq_cmpr(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_SQUIRRELEX_H_*/
