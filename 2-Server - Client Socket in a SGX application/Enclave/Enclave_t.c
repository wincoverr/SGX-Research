#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


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

static sgx_status_t SGX_CDECL sgx_ecall_test(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	ecall_test();
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_connexion(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_connexion_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_connexion_t* ms = SGX_CAST(ms_ecall_connexion_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_pseudo = ms->ms_pseudo;
	char* _tmp_mdp = ms->ms_mdp;



	ecall_connexion(_tmp_pseudo, _tmp_mdp);


	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_meteo(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_meteo_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_meteo_t* ms = SGX_CAST(ms_ecall_meteo_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_city = ms->ms_city;
	size_t _len_city = 80 * sizeof(char);
	char* _in_city = NULL;

	CHECK_UNIQUE_POINTER(_tmp_city, _len_city);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_city != NULL && _len_city != 0) {
		if ( _len_city % sizeof(*_tmp_city) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_city = (char*)malloc(_len_city);
		if (_in_city == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_city, _len_city, _tmp_city, _len_city)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}

	ecall_meteo(_in_city);

err:
	if (_in_city) free(_in_city);
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[3];
} g_ecall_table = {
	3,
	{
		{(void*)(uintptr_t)sgx_ecall_test, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_connexion, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_meteo, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[1][3];
} g_dyn_entry_table = {
	1,
	{
		{0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_print(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_t));
	ocalloc_size -= sizeof(ms_ocall_print_t);

	if (str != NULL) {
		ms->ms_str = (const char*)__tmp;
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}
	
	status = sgx_ocall(0, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

