#ifndef VFP_TYPES_H
#define VFP_TYPES_H
 
#include "vfp_lib.h"

#ifdef __cpluspluss
extern "C"
{
#endif

// ============================================================================
// vfp_echo_types.h
// Funciones de prueba "echo": reciben un parametro de un tipo LCK y lo
// devuelven identico. Sirven para validar que el marshaling Fox<->C
// funciona correctamente en ambos sentidos, tipo por tipo.
//
// Cubre los tipos con getter/setter ya disponibles en vfp_types.c:
//   C - Character      N - Numeric (double)   I - Integer
//   Y - Currency        D - Date                T - DateTime
//   L - Logical         M - Memo
//
// NO cubre (no hay helper Safe* equivalente en el proyecto todavia):
//   G - General/OLE     Q - Varbinary/blob binario propio
//   0 - Array            R - Parametro por referencia (@var)
// Si los necesitas, avisame y armamos los getters/setters que falten.
// ============================================================================

    char *SafeGetStringUtf8(Value *val, int *outLen);
    char *GetParamStringUtf8(ParamBlk *parm, int index, int *outLen);
    //wchar_t *GetParamStringToUnicode(ParamBlk *parm, int index, int *outLen);
    // ============================================================================
    // SAFE GET PARAM
    // ============================================================================
    Value SafeGetParam(ParamBlk *parm, int index);

    int SafeStoreByRefInt(Locator *loc, int value);

    void SafeRetValBinary(const BYTE *pData, DWORD cbData);

    void SafeRetValString(const char *data);
    char *GetParamString(ParamBlk *parm, int index, int *outLen);
    // ============================================================================
    // HELPERS
    // ============================================================================

    char *GetParamString(ParamBlk *parm, int index, int *outLen);
    long GetParamLong(ParamBlk *parm, int index, long defaultVal);
    // ============================================================================
    // HANDLE
    // ============================================================================

    void *SafeLockHandle(MHANDLE handle);
    void SafeUnlockHandle(MHANDLE handle);

    // ============================================================================
    // VALIDADORES DE TIPO
    // ============================================================================

    int IsNumeric(Value *val);
    int IsInteger(Value *val);
    int IsDouble(Value *val);
    int IsString(Value *val);
    int IsLogical(Value *val);
    int IsDate(Value *val);
    int IsDateTime(Value *val);
    int IsCurrency(Value *val);
    int IsMemo(Value *val);

    // ============================================================================
    // EXTRACTORES SEGUROS (GETTERS)
    // ============================================================================

    long SafeGetLong(Value *val);
    double SafeGetDouble(Value *val);
    int SafeGetLogical(Value *val);
    char *SafeGetString(Value *val);
    void SafeGetDate(Value *val, int *year, int *month, int *day);
    void SafeGetDateTime(Value *val, SYSTEMTIME *st);
    CCY SafeGetCurrency(Value *val);
    char *SafeGetChar(Value *val);

    // ============================================================================
    // RETORNOS SEGUROS A FOXPRO (_RETVAL)
    // ============================================================================

    void SafeRetValString(const char *data);
    void SafeRetValInt(long value);
    void SafeRetValDouble(double value);
    void SafeRetValLogical(int flag);
    void SafeRetValDate(int year, int month, int day);
    void SafeRetValDateTime(SYSTEMTIME *st);
    void SafeRetValCurrency(CCY ccyVal);
    
    // ============================================================================
    // ALMACENAMIENTO POR REFERENCIA (_STORE)
    // ============================================================================

    int SafeStoreReferenceInt(Locator *loc, int value);
    int SafeStoreReferenceDouble(Locator *loc, double value);
    // int SafeStoreReferenceString(Locator *loc, const char *value);
    int SafeStoreReferenceString(Locator *loc, const char *value, int valueLen);
    int SafeStoreReferenceLogical(Locator *loc, int flag);
    int SafeStoreReferenceDate(Locator *loc, int year, int month, int day);
    int SafeStoreReferenceDateTime(Locator *loc, SYSTEMTIME *st);
    int SafeStoreReferenceCurrency(Locator *loc, CCY ccyVal);

#ifdef __cplusplus
}
#endif

#endif // VFP_TYPES_H