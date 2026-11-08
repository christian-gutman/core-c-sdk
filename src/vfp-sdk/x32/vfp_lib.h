#ifndef VFP_LIB_H
#define VFP_LIB_H

#include "../../core/core.h" 

#ifdef __cplusplus
extern "C"
{
#endif

#include "pro_ext.h"

#define FILELOG "vfputils.log" 

/**
 * Estructura de Contexto para Callbacks de Visual FoxPro
 * 
 * Se utiliza para transferir de forma síncrona el control y estado 
 * de la creación de cursores dinámicos entre la FLL y el Core en C puro.
 */
typedef struct {
    char *alias;            // Nombre físico del cursor destino en FoxPro (Ej: "curClientes")
    int cursor_created;     // Flag de control interno (1 = Ya creado por _Execute; 0 = Pendiente)
} VfpCursorContext;

typedef struct {
    char      f_name[11];  // Nombre del campo (10 caracteres + el terminador nulo '\0')
    char      f_type;      // Tipo de dato en un solo carácter ('C', 'I', 'N', 'M', 'L', etc.)
    short     f_len;       // Longitud total del campo en la estructura de la tabla
    short     f_dec;       // Cantidad de posiciones decimales (solo válido para 'N' o 'F')
} FoxFieldInfo;
 

#ifdef __cplusplus
}
#endif

#endif // VFP_LIB_H 
