#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_PRINT_DEFINED__
#define OCALL_PRINT_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print, (const char* str));
#endif

sgx_status_t ecall_test(sgx_enclave_id_t eid);
sgx_status_t ecall_connexion(sgx_enclave_id_t eid, char pseudo[80], char mdp[80]);
sgx_status_t ecall_meteo(sgx_enclave_id_t eid, char city[80]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
