extern "C"
{

#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"

#define INT_DIST_MAX_ELEMS 100

PG_FUNCTION_INFO_V1(int_dist_insert_many);
PG_FUNCTION_INFO_V1(int_dist_insert_one);
PG_FUNCTION_INFO_V1(int_dist_sum);

Datum int_dist_insert_many(PG_FUNCTION_ARGS);
Datum int_dist_insert_one(PG_FUNCTION_ARGS);
Datum int_dist_sum(PG_FUNCTION_ARGS);

}

ArrayType * alloc_int32_array(int nelems)
{
  ArrayType * result;
  int nbytes = sizeof(int32) * nelems + ARR_OVERHEAD(1);
  result = (ArrayType *) palloc(nbytes);
  result->size = nbytes;
  result->ndim = 1;
  result->flags = 0;
  ARR_DIMS(result)[0] = nelems;
  ARR_LBOUND(result)[0] = 1;
  return result;
}   

Datum int_dist_insert_many(PG_FUNCTION_ARGS)
{
  int32 state_size(0), input_size(0), *state_vals(NULL), *input_vals(NULL);
  
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
    PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));
  else
  {
    ArrayType * a PG_GETARG_ARRAYTYPE_P(1);
    if (ARR_NDIM(a) == 1)
    {
      input_size = ARR_DIMS(a)[0];
      input_vals = (int32 *) ARR_DATA_PTR(a);
    }
  }

  int size = state_size;
  for (int i = 0; i < input_size; ++i)
    if (input_vals[i] >= size)
      size = input_vals[i] + 1;

  if (size > INT_DIST_MAX_ELEMS)
    elog(NOTICE, "%i element array created. int_dist functions really "
      "shouldn't be used for distributions bigger than [0,%i)",
      size, INT_DIST_MAX_ELEMS);
  
  ArrayType * result = alloc_int32_array(size);
  int32 * result_vals = (int32 *) ARR_DATA_PTR(result);
  
  for (int i = 0; i < size; ++i)
    result_vals[i] = i < state_size ? state_vals[i] : 0;
    
  for (int i = 0; i < input_size; ++i)
    ++result_vals[input_vals[i]];

  PG_RETURN_ARRAYTYPE_P(result);
}

Datum int_dist_insert_one(PG_FUNCTION_ARGS)
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
    PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));
  else
  {
    input = PG_GETARG_INT32(1);
  }

  int size = state_size <= input ? input + 1 : state_size;

  if (size > INT_DIST_MAX_ELEMS)
    elog(NOTICE, "%i element array created. int_dist functions really "
      "shouldn't be used for distributions bigger than [0,%i)",
      size, INT_DIST_MAX_ELEMS);
  
  ArrayType * result = alloc_int32_array(size);
  int32 * result_vals = (int32 *) ARR_DATA_PTR(result);

  for (int i = 0; i < size; ++i)
    result_vals[i] = i < state_size ? state_vals[i] : 0;
    
  ++result_vals[input];

  PG_RETURN_ARRAYTYPE_P(result);
}

Datum int_dist_sum(PG_FUNCTION_ARGS)
{
  // if either argument is null return right away  
  if (PG_ARGISNULL(0))
  {
    if (PG_ARGISNULL(1))  
      PG_RETURN_NULL();
    else
      PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(1));
  }
  else
  {
    if (PG_ARGISNULL(1))  
      PG_RETURN_ARRAYTYPE_P(PG_GETARG_ARRAYTYPE_P(0));
  }
  
  // load first and second arguments
  int32 a_size(0), b_size(0), *a_vals(NULL), *b_vals(NULL);
  
  ArrayType * a = PG_GETARG_ARRAYTYPE_P(0);
  if (ARR_NDIM(a) == 1)
  {
    a_size = ARR_DIMS(a)[0];
    a_vals = (int32 *) ARR_DATA_PTR(a);
  }
  
  ArrayType * b = PG_GETARG_ARRAYTYPE_P(1);
  if (ARR_NDIM(b) == 1)
  {
    b_size = ARR_DIMS(b)[0];
    b_vals = (int32 *) ARR_DATA_PTR(b);
  }

  // find bigger and smaller arguments
  int32 big_size(0), small_size(0), *big_vals(NULL), *small_vals(NULL);
  
  if (a_size < b_size)
  {
    big_size = b_size;
    big_vals = b_vals;
    small_size = a_size;
    small_vals = a_vals;
  }
  else
  {
    big_size = a_size;
    big_vals = a_vals;
    small_size = b_size;
    small_vals = b_vals;
  }

  // find result
  ArrayType * result = alloc_int32_array(big_size);
  int32 * result_vals = (int32 *) ARR_DATA_PTR(result);
  
  memcpy(result_vals, big_vals, big_size * sizeof(int32));
  for (int i = 0; i < small_size; ++i)
    result_vals[i] += small_vals[i];

  PG_RETURN_ARRAYTYPE_P(result);
}
