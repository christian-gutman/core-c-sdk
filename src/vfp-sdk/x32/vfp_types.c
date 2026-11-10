#include "vfp_types.h" 
#include "core_encoding.h"
#include "core_string.h"

wchar_t *FoxToWChar(const char *ansiStr)
{
    if (!ansiStr || ansiStr[0] == '\0')
        return NULL;
    int len = MultiByteToWideChar(CP_THREAD_ACP, 0, ansiStr, -1, NULL, 0);
    if (len <= 0)
        return NULL;
    wchar_t *wstr = (wchar_t *)malloc(len * sizeof(wchar_t));
    if (wstr)
        MultiByteToWideChar(CP_THREAD_ACP, 0, ansiStr, -1, wstr, len);
    return wstr;
}

Value SafeGetParam(ParamBlk *parm, int index)
{
    Value valParam;
    valParam.ev_type = '0';
    valParam.ev_length = 0;
    valParam.ev_handle = 0;
    if (parm && index >= 0 && index < parm->pCount)
    {
        if (parm->p[index].loc.l_type == 'R')
        {
            _Load(&parm->p[index].loc, &valParam);
        }
        else
        {
            valParam = parm->p[index].val;
        }
    }
    return valParam;
}

// ============================================================================
// Helper para retornar datos binarios de longitud exacta a FoxPro (Soporta nulos)
// ============================================================================
void SafeRetValBinary(const BYTE *pData, DWORD cbData)
{
    // 1. Si no hay datos o el tamaño es cero, devolvemos un string vacío básico
    if (!pData || cbData == 0)
    {
        _RetChar("");
        return;
    }

    // 2. Inicializar la estructura de comunicación nativa de FoxPro
    Value retVal;
    retVal.ev_type = 'C';            // Tipo de datos Character en FoxPro
    retVal.ev_length = (long)cbData; // Le indicamos la longitud exacta de la variable
    retVal.ev_width = 0;
    retVal.ev_padding = 0;

    // 3. PASO 1: Solicitar el Handle de memoria al heap interno de FoxPro
    retVal.ev_handle = _AllocHand((unsigned int)cbData);
    if (retVal.ev_handle == 0)
    {
        _Error(182); // "Insufficient memory" en FoxPro si el OS está al límite
        return;
    }

    // 4. PASO 2: Bloquear el Handle y obtener el puntero físico de C
    char *pFoxBuffer = (char *)SafeLockHandle(retVal.ev_handle);
    if (pFoxBuffer)
    {
        // 5. PASO 3: Transferencia binaria de los bytes del archivo
        memcpy(pFoxBuffer, pData, cbData);

        // 6. PASO 4: Liberar el seguro impuesto sobre la memoria de FoxPro
        SafeUnlockHandle(retVal.ev_handle);

        // 7. Entregarle el control de la estructura final al motor de Visual FoxPro
        _RetVal(&retVal);
    }
    else
    {
        // En caso de una falla crítica de locking, devolvemos vacío para no colgar VFP
        _RetChar("");
    }
} 
/*
wchar_t *GetParamStringToUnicode(ParamBlk *parm, int index, int *outLen)
{
    if (outLen) *outLen = 0;
    if (!parm || index >= parm->pCount) return NULL;
    if (parm->p[index].val.ev_type != 'C' && parm->p[index].val.ev_type != 'M') return NULL;
 
    int len = parm->p[index].val.ev_length;
    if (len <= 0) return L"";

    const char *src = (const char *)SafeLockHandle(parm->p[index].val.ev_handle);
    if (!src) return NULL;
 
    StrAllTrim((char *)src);
    char *outsrc = StrSanitize((const char *)src);
    outsrc = StrCleanSpaces((const char *)outsrc);
   
    wchar_t *wSrc = StrAutoToUnicode(outsrc, (int)strlen(outsrc), NULL);
    SafeUnlockHandle(parm->p[index].val.ev_handle);
 
    if(outsrc)
        free(outsrc);
    
    if (wSrc) {        
        return wSrc;
    }
    return NULL;
}*/
 
// ============================================================================
// Helper interno: extrae un parámetro string de forma segura (lock + copia).
// El caller es responsable de hacer free() del resultado.
// ============================================================================
char *GetParamStringUtf8(ParamBlk *parm, int index, int *outLen)
{
    if (outLen) *outLen = 0;
    if (!parm || index >= parm->pCount) return NULL;
    if (parm->p[index].val.ev_type != 'C' && parm->p[index].val.ev_type != 'M') return NULL;
 
    int len = parm->p[index].val.ev_length;
    if (len <= 0) return _strdup("");

    const char *src = (const char *)SafeLockHandle(parm->p[index].val.ev_handle);
    if (!src) return NULL;
 
    //StrAllTrim(src);
    //src = StrCleanSpaces((const char *)src);
    //src = StrSanitize((const char *)src);

    int utf8Len = 0;
    char *result = StrAutoToUtf8(src, len, &utf8Len);
 
    SafeUnlockHandle(parm->p[index].val.ev_handle);
 
    //if(src)
    //    free(src);
    
    if (result) {
        if (outLen) *outLen = utf8Len;
        return result;
    }

    return NULL;
} 

char *GetParamString(ParamBlk *parm, int index, int *outLen)
{
    if (outLen)
        *outLen = 0;
    if (!parm || index >= parm->pCount)
        return NULL;
    if (parm->p[index].val.ev_type != 'C' && parm->p[index].val.ev_type != 'M')
        return NULL;
 
    int len = parm->p[index].val.ev_length;
    if (len <= 0)
    {
        if (outLen)
            *outLen = 0;
        return _strdup("");
    }

    const char *src = (const char *)SafeLockHandle(parm->p[index].val.ev_handle);
    if (!src)
        return NULL;
 
    char *copy = (char *)malloc((size_t)len + 1);
    if (copy)
    {
        memcpy(copy, src, (size_t)len);
        copy[len] = '\0';
        if (outLen)
            *outLen = len;
    }

    SafeUnlockHandle(parm->p[index].val.ev_handle);
    return copy;
}

// Helper interno: extrae un parámetro numérico como long, con default si no
// viene presente.
long GetParamLong(ParamBlk *parm, int index, long defaultVal)
{
    //
    if (!parm || index >= parm->pCount)
    {
        return defaultVal;
    }
    //
    char type = parm->p[index].val.ev_type;
    if (type == 'I')
    {
        return parm->p[index].val.ev_long;
    }
    if (type == 'N')
    {
        return (long)parm->p[index].val.ev_real;
    }
    if (type != 'I' && type != 'N')
    {
        return defaultVal;
    }
    return SafeGetLong(&(parm->p[index].val));
}

// ============================================================================
// VALIDADORES DE TIPO EXPANDIDOS
// ============================================================================

int IsNumeric(Value *val) { return (val && (val->ev_type == 'I' || val->ev_type == 'N' || val->ev_type == 'F')); }
int IsInteger(Value *val) { return (val && val->ev_type == 'I'); }
int IsDouble(Value *val) { return (val && (val->ev_type == 'N' || val->ev_type == 'F')); }
int IsString(Value *val) { return (val && val->ev_type == 'C'); }
int IsLogical(Value *val) { return (val && val->ev_type == 'L'); }
int IsDate(Value *val) { return (val && val->ev_type == 'D'); }
int IsDateTime(Value *val) { return (val && val->ev_type == 'T'); }
int IsCurrency(Value *val) { return (val && val->ev_type == 'Y'); }
int IsMemo(Value *val) { return (val && val->ev_type == 'M'); }

// ============================================================================
// HELPERS GENÉRICOS DE LOCKING DE HANDLES (MHANDLE)
// ----------------------------------------------------------------------------
// Centraliza el patrón _HLock/_HandToPtr/_HUnLock que antes se repetía (y en
// algunos módulos, como vfp_strings.c, se omitía directamente el lock).
// Cualquier código que necesite leer el contenido crudo de un MHANDLE de VFP
// debería pasar por acá en vez de llamar a _HandToPtr directo.
//
// Uso:
//   void *p = SafeLockHandle(hMem);
//   if (p) {
//       // usar p...
//       SafeUnlockHandle(hMem);
//   }
// ============================================================================

void *SafeLockHandle(MHANDLE handle)
{
    if (handle == 0)
        return NULL;
    _HLock(handle);
    void *ptr = _HandToPtr(handle);
    if (!ptr)
    {
        // Si _HandToPtr falla igual liberamos el lock para no dejarlo colgado
        _HUnLock(handle);
        return NULL;
    }
    return ptr;
}

void SafeUnlockHandle(MHANDLE handle)
{
    if (handle == 0)
        return;
    _HUnLock(handle);
}

// ============================================================================
// EXTRACTORES SEGUROS (GETTERS)
// ============================================================================

long SafeGetLong(Value *val)
{
    if (!val)
        return 0;
    if (val->ev_type == 'I')
        return val->ev_long;
    if (val->ev_type == 'N' || val->ev_type == 'F')
        return (long)val->ev_real;
    if (val->ev_type == 'Y')
        return (long)(val->ev_currency.QuadPart / 10000);
    return 0;
}

double SafeGetDouble(Value *val)
{
    if (!val)
        return 0.0;
    if (val->ev_type == 'N' || val->ev_type == 'F')
        return val->ev_real;
    if (val->ev_type == 'I')
        return (double)val->ev_long;
    if (val->ev_type == 'Y')
        return (double)val->ev_currency.QuadPart / 10000.0;
    return 0.0;
}

int SafeGetLogical(Value *val)
{
    if (!val || val->ev_type != 'L')
        return 0;
    return (val->ev_length != 0);
}
 
char *SafeGetStringUtf8(Value *val, int *outLen)
{
    if (!val || (val->ev_type != 'C' && val->ev_type != 'M'))
        return NULL;

    unsigned int strLen = val->ev_length;
    if (strLen == 0) {
        *outLen = 0;
        return _strdup("");
    }
 
    void *src = SafeLockHandle(val->ev_handle);
    if (!src) return NULL;
 
    int utf8Len = 0;
    char *utf8Str = StrAutoToUtf8((const char*)src, (int)strLen, &utf8Len);
 
    SafeUnlockHandle(val->ev_handle);
 
    *outLen = utf8Len;
    return utf8Str;
}

// Reescrita para usar los helpers genéricos SafeLockHandle/SafeUnlockHandle
// en vez de llamar directo a _HLock/_HandToPtr/_HUnLock.
char *SafeGetString(Value *val)
{
    if (!val || (val->ev_type != 'C' && val->ev_type != 'M'))
        return NULL;

    unsigned int strLen = val->ev_length;
    if (strLen == 0)
        return _strdup("");

    char *c_str = (char *)malloc(strLen + 1);
    if (c_str == NULL)
        return NULL;

    void *src = SafeLockHandle(val->ev_handle);
    if (!src)
    {
        free(c_str);
        return NULL;
    }

    memcpy(c_str, src, strLen);
    c_str[strLen] = '\0';
    SafeUnlockHandle(val->ev_handle);

    return c_str;
}
char *SafeGetChar(Value *val)
{
    if (!val || (val->ev_type != 'C' && val->ev_type != 'M'))
        return NULL;

    unsigned int strLen = val->ev_length;
    if (strLen == 0)
        return _strdup("");

    if (strLen > 1)
        return NULL;

    char *c_str = (char *)malloc(strLen + 1);
    if (c_str == NULL)
        return NULL;

    void *src = SafeLockHandle(val->ev_handle);
    if (!src)
    {
        free(c_str);
        return NULL;
    }

    memcpy(c_str, src, strLen);
    c_str[strLen] = '\0';
    SafeUnlockHandle(val->ev_handle);

    return c_str;
}
// Extrae una fecha convirtiéndola a enteros de Año, Mes y Día
void SafeGetDate(Value *val, int *year, int *month, int *day)
{
    if (!val || val->ev_type != 'D' || !year || !month || !day)
        return;
    // FoxPro almacena las fechas como un número juliano en ev_real
    long julian = (long)val->ev_real;

    // Algoritmo de conversión estándar de fechas Julianas a Gregoriano
    long l = julian + 68569;
    long n = (4 * l) / 146097;
    l = l - (146097 * n + 3) / 4;
    long i = (4000 * (l + 1)) / 1461001;
    l = l - (1461 * i) / 4 + 31;
    long j = (80 * l) / 2447;
    *day = (int)(l - (2447 * j) / 80);
    l = j / 11;
    *month = (int)(j + 2 - (12 * l));
    *year = (int)(100 * (n - 49) + i + l);
}

// Extrae la fecha y hora completa en formato estructurado de Windows
void SafeGetDateTime(Value *val, SYSTEMTIME *st)
{
    if (!val || val->ev_type != 'T' || !st)
        return;
    memset(st, 0, sizeof(SYSTEMTIME));

    long julian = (long)val->ev_real;
    long msec = val->ev_long; // VFP almacena milisegundos desde la medianoche aquí

    // Conversión de fecha juliana interna
    long l = julian + 68569;
    long n = (4 * l) / 146097;
    l = l - (146097 * n + 3) / 4;
    long i = (4000 * (l + 1)) / 1461001;
    l = l - (1461 * i) / 4 + 31;
    long j = (80 * l) / 2447;
    st->wDay = (WORD)(l - (2447 * j) / 80);
    l = j / 11;
    st->wMonth = (WORD)(j + 2 - (12 * l));
    st->wYear = (WORD)(100 * (n - 49) + i + l);

    // Conversión de tiempo (msec)
    st->wHour = (WORD)(msec / 3600000);
    msec %= 3600000;
    st->wMinute = (WORD)(msec / 60000);
    msec %= 60000;
    st->wSecond = (WORD)(msec / 1000);
    st->wMilliseconds = (WORD)(msec % 1000);
}
CCY SafeGetCurrency(Value *val)
{
    CCY ccyRes;
    ccyRes.HighPart = 0;
    ccyRes.LowPart = 0;

    if (!val)
        return ccyRes;

    if (val->ev_type == 'Y')
    {
        // Copia directa de la estructura nativa
        ccyRes = val->ev_currency;
    }
    else if (val->ev_type == 'I')
    {
        // Conversión manual usando los miembros reales de _LARGE_INTEGER
        long total = val->ev_long * 10000;
        ccyRes.LowPart = (unsigned long)total;
        ccyRes.HighPart = (total < 0) ? -1 : 0;
    }
    else if (val->ev_type == 'N' || val->ev_type == 'F')
    {
        // Si viene un double, usamos QuadPart que es nativo de la unión
        ccyRes.QuadPart = (LONGLONG)(val->ev_real * 10000.0);
    }

    return ccyRes;
}

// ============================================================================
// RETORNOS SEGUROS A FOXPRO (_RETVAL)
// ============================================================================
void SafeRetValString(const char *data)
{
    const char *pStr = NULL;
    size_t len = 0;
    Value retVal = {0};
    char *fox_ptr = NULL;
    MHANDLE hAlloc = 0;

    // Validar y preparar string de entrada
    pStr = data ? data : "";
    len = strlen(pStr);

    // Validar longitud razonable (protección contra overflow)
    if (len > 0x7FFFFFFF)
    {
        _RetChar("");
        return;
    }

    // Inicializar estructura de retorno
    retVal.ev_type = 'C';
    retVal.ev_length = (long)len;
    retVal.ev_handle = 0;
    retVal.ev_padding = 0;

    // Manejo especial para string vacío
    if (len == 0)
    {
        _RetChar("");
        return;
    }

    // Asignar memoria para el handle
    hAlloc = _AllocHand(len);
    if (hAlloc == 0)
    {
        _RetChar("");
        return;
    }

    retVal.ev_handle = hAlloc;

    // Obtener puntero al handle y validar
    fox_ptr = (char *)SafeLockHandle(retVal.ev_handle);
    if (!fox_ptr)
    {
        _FreeHand(retVal.ev_handle);
        _RetChar("");
        return;
    }

    // Copiar datos con validación
    memcpy(fox_ptr, pStr, len);
    SafeUnlockHandle(retVal.ev_handle);

    // Establecer valor de retorno
    _RetVal(&retVal);
}

void SafeRetValInt(long value)
{
    Value retVal;
    retVal.ev_type = 'I';
    retVal.ev_long = value;
    retVal.ev_width = 11;
    _RetVal(&retVal);
}

void SafeRetValDouble(double value)
{
    Value retVal;
    retVal.ev_type = 'N';
    retVal.ev_real = value;
    retVal.ev_width = 20;
    retVal.ev_length = 4;
    _RetVal(&retVal);
}

void SafeRetValLogical(int flag)
{
    Value retVal;
    retVal.ev_type = 'L';
    retVal.ev_length = flag ? 1 : 0;
    _RetVal(&retVal);
}

// Helper interno para calcular días julianos requeridos por VFP
static double Local_GetJulianDate(int year, int month, int day)
{
    long a = (14 - month) / 12;
    long y = year + 4800 - a;
    long m = month + 12 * a - 3;
    return (double)(day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045);
}

void SafeRetValDate(int year, int month, int day)
{
    Value retVal;
    retVal.ev_type = 'D';
    retVal.ev_real = Local_GetJulianDate(year, month, day);
    _RetVal(&retVal);
}

void SafeRetValDateTime(SYSTEMTIME *st)
{
    if (!st)
    {
        _RetVal(NULL);
        return;
    }
    Value retVal;
    retVal.ev_type = 'T';
    retVal.ev_real = Local_GetJulianDate(st->wYear, st->wMonth, st->wDay);
    retVal.ev_long = (long)(st->wHour * 3600000 + st->wMinute * 60000 + st->wSecond * 1000 + st->wMilliseconds);
    _RetVal(&retVal);
}
void SafeRetValCurrency(CCY ccyVal)
{
    Value retVal;
    retVal.ev_type = 'Y';
    retVal.ev_width = 20;

    // Asignación directa pura (CCY a CCY) compatible con pro_ext.h
    retVal.ev_currency = ccyVal;

    _RetVal(&retVal);
}

// ============================================================================
// ALMACENAMIENTO POR REFERENCIA (_STORE)
// ============================================================================

int SafeStoreByRefInt(Locator *loc, int value)
{
    Value val;
    int r = -1;
    if (loc->l_type == 'R')
    {
        r = _Load(loc, &val);
        if (r == 0)
        {
            if (IsNumeric(&val))
            {
                val.ev_long = (long)value;
                r = (_Store(loc, &val) == 0);
            }
        }
        _FreeHand(val.ev_handle);
    }
    return r;
}

int SafeStoreReferenceInt(Locator *loc, int value)
{
    if (!loc || (loc->l_type != 'R' && loc->l_type != 'V'))
        return 0;
    Value valResult;
    valResult.ev_type = 'I';
    valResult.ev_long = (long)value;
    valResult.ev_width = 11;
    valResult.ev_handle = 0;
    return (_Store(loc, &valResult) == 0);
}

int SafeStoreReferenceDouble(Locator *loc, double value)
{
    if (!loc || (loc->l_type != 'R' && loc->l_type != 'V'))
        return 0;
    Value valResult;
    valResult.ev_type = 'N';
    valResult.ev_real = value;
    valResult.ev_width = 20;
    valResult.ev_length = 4;
    return (_Store(loc, &valResult) == 0);
}

int SafeStoreReferenceString(Locator *loc, const char *value, int valueLen)
{
    if (!loc || (loc->l_type != 'R' && loc->l_type != 'V'))
        return 0;

    const char *pStr = value ? value : "";
    size_t text_len = valueLen >= 0 ? (size_t)valueLen : strlen(pStr);

    // Protección contra longitud inválida
    if (text_len > 0x7FFFFFFF)
        return 0;

    Value valResult = {0};
    valResult.ev_type = 'C';
    valResult.ev_length = (long)text_len;
    valResult.ev_handle = 0;

    // String vacío: no reservar handle
    if (text_len == 0)
    {
        int rc = _Store(loc, &valResult);
        return (rc == 0);
    }

    // Reservar memoria
    valResult.ev_handle = _AllocHand((long)text_len);
    if (valResult.ev_handle == 0)
        return 0;

    // Obtener puntero y validar
    char *fox_ptr = (char *)SafeLockHandle(valResult.ev_handle);
    if (!fox_ptr)
    {
        _FreeHand(valResult.ev_handle);
        return 0;
    }

    // Copiar datos
    memcpy(fox_ptr, pStr, text_len);
    SafeUnlockHandle(valResult.ev_handle);

    // Almacenar y liberar
    int rc = _Store(loc, &valResult);
    _FreeHand(valResult.ev_handle);

    return (rc == 0);
}

int SafeStoreReferenceLogical(Locator *loc, int flag)
{
    if (!loc || (loc->l_type != 'R' && loc->l_type != 'V'))
        return 0;
    Value valResult;
    valResult.ev_type = 'L';
    valResult.ev_length = flag ? 1 : 0;
    valResult.ev_handle = 0;
    return (_Store(loc, &valResult) == 0);
}

int SafeStoreReferenceDate(Locator *loc, int year, int month, int day)
{
    if (!loc || (loc->l_type != 'R' && loc->l_type != 'V'))
        return 0;
    Value valResult;
    valResult.ev_type = 'D';
    valResult.ev_real = Local_GetJulianDate(year, month, day);
    valResult.ev_handle = 0;
    return (_Store(loc, &valResult) == 0);
}

int SafeStoreReferenceDateTime(Locator *loc, SYSTEMTIME *st)
{
    if (!loc || (loc->l_type != 'R' && loc->l_type != 'V') || !st)
        return 0;
    Value valResult;
    valResult.ev_type = 'T';
    valResult.ev_real = Local_GetJulianDate(st->wYear, st->wMonth, st->wDay);
    valResult.ev_long = (long)(st->wHour * 3600000 + st->wMinute * 60000 + st->wSecond * 1000 + st->wMilliseconds);
    valResult.ev_handle = 0;
    return (_Store(loc, &valResult) == 0);
}
int SafeStoreReferenceCurrency(Locator *loc, CCY ccyVal)
{
    if (!loc || (loc->l_type != 'R' && loc->l_type != 'V'))
        return 0;

    Value valResult;
    valResult.ev_type = 'Y';
    valResult.ev_width = 20;
    valResult.ev_handle = 0;

    // Asignación directa sin conversiones externas
    valResult.ev_currency = ccyVal;

    return (_Store(loc, &valResult) == 0);
}
