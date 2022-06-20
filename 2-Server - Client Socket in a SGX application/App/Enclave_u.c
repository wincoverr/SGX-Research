#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_ecall_connexion_t {
	char* ms_pseudo;
	char* ms_mdp;
} ms_ecall_connexion_t;

typedef struct ms_ecall_meteo_t {
	char* ms_city;
} ms_ecall_meteo_t;

typedef struct ms_ocall_print_t {
	const char* ms_str;
} ms_ocall_print_t;

static sgx_status_t SGX_CDECL Enclave_ocall_print(void* pms)
{
	ms_ocall_print_t* ms = SGX_CAST(ms_ocall_print_t*, pms);
	ocall_print(ms->ms_str);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_Enclave = {
	1,
	{
		(void*)Enclave_ocall_print,
	}
};
sgx_status_t ecall_test(sgx_enclave_id_t eid)
{
	sgx_status_t status;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, NULL);
	return status;
}

sgx_status_t ecall_connexion(sgx_enclave_id_t eid, char pseudo[80], char mdp[80])
{
	sgx_status_t status;
	ms_ecall_connexion_t ms;
	ms.ms_pseudo = (char*)pseudo;
	ms.ms_mdp = (char*)mdp;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t ecall_meteo(sgx_enclave_id_t eid, char city[80])
{
	sgx_status_t status;
	ms_ecall_meteo_t ms;
	ms.ms_city = (char*)city;
	status = sgx_ecall(eid, 2, &ocall_table_Enclave, &ms);
	return status;
}

