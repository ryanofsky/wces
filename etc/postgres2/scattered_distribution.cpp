extern "C"
{

#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "catalog/pg_type.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(int_scat_distf);
PG_FUNCTION_INFO_V1(int_scat_modef);

Datum int_scat_distf(PG_FUNCTION_ARGS);
Datum int_scat_modef(PG_FUNCTION_ARGS);

}

// From https://github.com/greenplum-db/postgis/blob/master/libpgcommon/pgsql_compat.h
/* Define ARR_OVERHEAD_NONULLS for PostgreSQL < 8.2 */
#ifndef ARR_OVERHEAD_NONULLS
#define ARR_OVERHEAD_NONULLS(x) ARR_OVERHEAD((x))
#endif
/* PostgreSQL < 8.3 uses VARATT_SIZEP rather than SET_VARSIZE for varlena types */
#ifndef SET_VARSIZE
#define SET_VARSIZE(var, size)   VARATT_SIZEP(var) = size
#endif

static ArrayType * alloc_int32_array(int nelems)
{
  // based on new_intArrayType contrib/intarray/_int.c
  ArrayType * result;
  int nbytes = sizeof(int32) * nelems + ARR_OVERHEAD_NONULLS(1);
  result = (ArrayType *) palloc(nbytes);
  MemSet(result, 0, nbytes);
  SET_VARSIZE(result, nbytes);
  ARR_NDIM(result) = 1;
  ARR_ELEMTYPE(result) = INT4OID;
  ARR_DIMS(result)[0] = nelems;
  ARR_LBOUND(result)[0] = 1;
  return result;
}

Datum int_scat_distf(PG_FUNCTION_ARGS)
{
  int32 state_size(0), *state_vals(NULL), input(0);

  if (!PG_ARGISNULL(0))
  {
    ArrayType * a PG_GETARG_ARRAYTYPE_P(0);
    if (ARR_NDIM(a) == 1)
    {
      state_size = ARR_DIMS(a)[0];
      state_vals = (int32 *) ARR_DATA_PTR(a);
    }
  }

  if (PG_ARGISNULL(1))
  {
    if (state_size > 0)
      PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));
    else
      PG_RETURN_NULL();
  }
  else
    input = PG_GETARG_INT32(1);

  if (state_size % 2 != 0)
  {
    elog(NOTICE, "%i element array passed to int_scat_distf(). Expecting "
      "an array with an even number of elements.", state_size);
    --state_size;
  }

  int inc_index = state_size;
  int result_size = state_size + 2;

  for (int i = 0; i < state_size; i += 2)
  {
    if (state_vals[i] == input)
    {
      inc_index = i;
      result_size = state_size;
      break;
    };
  };

  ArrayType * result = alloc_int32_array(result_size);
  int32 * result_vals = (int32 *) ARR_DATA_PTR(result);

  if(state_size > 0)
    memcpy(result_vals, state_vals, state_size * sizeof(int32));
  
  if (inc_index < state_size)
    ++result_vals[inc_index+1];
  else
  {
    result_vals[inc_index] = input;
    result_vals[inc_index+1] = 1;
  };

  PG_RETURN_ARRAYTYPE_P(result);
}

Datum int_scat_modef(PG_FUNCTION_ARGS)
{
  int32 state_size(0), *state_vals(NULL);

  if (!PG_ARGISNULL(0))
  {
    ArrayType * a PG_GETARG_ARRAYTYPE_P(0);
    if (ARR_NDIM(a) == 1)
    {
      state_size = ARR_DIMS(a)[0];
      state_vals = (int32 *) ARR_DATA_PTR(a);
    }
  }
  
  if (state_size < 2) PG_RETURN_NULL();
  
  int max_val = state_vals[0];
  int max = state_vals[1];
  
  for (int i = 3; i < state_size; i += 2)
  {
    if (state_vals[i] > max)
    {
      max_val = state_vals[i-1];
      max = state_vals[i];
    };
  };

  PG_RETURN_INT32(max_val);
}