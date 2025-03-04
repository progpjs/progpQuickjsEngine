#include "quickjs.h"
#include <stdbool.h>

#ifndef QUICKJS_ENGINE_CPP
#define QUICKJS_ENGINE_CPP

// -- Doc --
// https://bellard.org/quickjs/quickjs.html
// https://blogs.igalia.com/compilers/2023/06/12/quickjs-an-overview-and-guide-to-adding-a-new-feature/

#define DEBUG_PRINT(m) printf("C-DEBUG - %s\n", m)
#define DEBUG_PRINT_KeepLine(m) printf("C-DEBUG - %s", m)

#define AnyValueTypeUndefined   0
#define AnyValueTypeNull        1
#define AnyValueTypeError       2
#define AnyValueTypeNumber      3
#define AnyValueTypeString      4
#define AnyValueTypeBoolean     5
#define AnyValueTypeBuffer      6
#define AnyValueTypeFunction    7
#define AnyValueTypeJson        8
#define AnyValueTypeInt32       9

typedef struct s_quick_anyValue {
    int valueType;
    double number;
    void* voidPtr;
    int size;

    // Used when returning a value in order to
    // declare an error. The value is automatically
    // deleted after consumed.
    //
    char* errorMessage;

    // mustFree is used from the Go side.
    // Allows knowing if the value voidPtr must be free.
    //
    int mustFree;
} s_quick_anyValue;

typedef struct s_quick_ctx s_quick_ctx;

typedef struct s_quick_error {
    s_quick_ctx* pCtx;
    const char* errorTitle;
    const char* errorStackTrace;
} s_quick_error;

typedef struct s_quick_ctx {
    int refCount;
    void* userData;

    JSContext* ctx;
    JSRuntime *runtime;

    bool hasException;
    s_quick_error execException;

    s_quick_anyValue* jsToGoValues;
    int jsToGoValuesCount;

    s_quick_anyValue* goToJsValues;
    int goToJsValuesCount;

} s_quick_ctx;

typedef struct s_quick_result {
    s_quick_anyValue returnValue;
    s_quick_error* error;
} s_quick_result;

typedef void (*f_quickjs_OnContextDestroyed)(s_quick_ctx* ctx);
typedef void (*f_quickjs_OnResourceReleased)(void* resource);

//region Config

void quickjs_setEventOnContextReleased(f_quickjs_OnContextDestroyed callback);
void quickjs_setEventOnAutoDisposeResourceReleased(f_quickjs_OnResourceReleased h);

//endregion

//region Context

void quickjs_initialize();

s_quick_ctx* quickjs_createContext(void* userData);
void quickjs_incrContext(s_quick_ctx* pCtx);
void quickjs_decrContext(s_quick_ctx* pCtx);

s_quick_error* quickjs_executeScript(s_quick_ctx* pCtx, const char* script, const char* origin);
void quickjs_bindFunction(s_quick_ctx* pCtx, const char* functionName, int minArgCount, JSCFunction fct);

JSValue quickjs_anyValueToJsValue(s_quick_ctx *pCtx, const s_quick_anyValue *anyValue, bool* error);

//endregion

//region Release ref

void quickjs_releaseError(s_quick_error* error);
void quickjs_releaseFunction(s_quick_ctx* pCtx, JSValue* host);

//endregion

//region Calling functions

s_quick_result quickjs_callFunctionWithAnyValues(s_quick_ctx* pCtx, JSValue* fctToCall, int keepAlive);
s_quick_error* quickjs_callFunctionWithUndefined(s_quick_ctx* pCtx, JSValue* fctToCall, int keepAlive);
s_quick_error* quickjs_callFunctionWithAutoReleaseResource2(s_quick_ctx* pCtx, JSValue* fctToCall, int keepAlive, uintptr_t res);

//endregion

s_quick_ctx* quickjs_callParamsToAnyValue(JSContext *ctx, int argc, JSValueConst *argv);

JSValue quickjs_newAutoReleaseResource(s_quick_ctx* pCtx, void* value);

JSValue quickjs_processExternalFunctionCallResult(s_quick_ctx* pCtx, s_quick_anyValue anyValue);

void* quickjs_copyBuffer(void* buffer, int size);

#endif // QUICKJS_ENGINE_CPP