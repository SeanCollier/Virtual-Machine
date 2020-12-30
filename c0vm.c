/**************************************************************************/
/*              COPYRIGHT Carnegie Mellon University 2020                 */
/* Do not post this file or any derivative on a public site or repository */
/**************************************************************************/


#include <assert.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "lib/xalloc.h"
#include "lib/stack.h"
#include "lib/contracts.h"
#include "lib/c0v_stack.h"
#include "lib/c0vm.h"
#include "lib/c0vm_c0ffi.h"
#include "lib/c0vm_abort.h"

/* call stack frames */
typedef struct frame_info frame;
struct frame_info {
  c0v_stack_t S; /* Operand stack of C0 values */
  ubyte *P;      /* Function body */
  size_t pc;     /* Program counter */
  c0_value *V;   /* The local variables */
};

int execute(struct bc0_file *bc0) {
  REQUIRES(bc0 != NULL);

  /* Variables */
  c0v_stack_t S; /* Operand stack of C0 values */
  ubyte *P;      /* Array of bytes that make up the current function */
  size_t pc ;     /* Current location within the current byte array P */
  c0_value *V;   /* Local variables (you won't need this till Task 2) */
  (void) V;

  /* The call stack, a generic stack that should contain pointers to frames */
  /* You won't need this until you implement functions. */
  gstack_t callStack;
  (void) callStack;
  callStack = stack_new();

  S = c0v_stack_new();
  P = bc0->function_pool[0].code;
  V = xcalloc(sizeof(c0_value),bc0->function_pool[0].num_vars );
  pc = 0;

  while (true) {

//#ifdef DEBUG
//    /* You can add extra debugging information here */
//    fprintf(stderr, "Opcode %x -- Stack size: %zu -- PC: %zu\n",
//            P[pc], c0v_stack_size(S), pc);
//#endif

    switch (P[pc]) {

    /* Additional stack operation: */

    case POP: {
      pc++;
      c0v_pop(S);
      break;
    }

    case DUP: {
      pc++;
      c0_value v = c0v_pop(S);
      c0v_push(S,v);
      c0v_push(S,v);
      break;
    }

    case SWAP:{
      pc++;
      c0_value v2 = c0v_pop(S);
      c0_value v1 = c0v_pop(S);
      c0v_push(S,v2);
      c0v_push(S,v1);
      break;

    }


    /* Returning from a function.
     * This currently has a memory leak! You will need to make a slight
     * change for the initial tasks to avoid leaking memory.  You will
     * need to be revise it further when you write INVOKESTATIC. */

    case RETURN: {

      c0_value res = c0v_pop(S);
      c0v_stack_free(S);
      free(V);
      if(stack_empty(callStack)){
        stack_free(callStack,NULL);
        int retval = val2int(res);
        return retval;
      }
      else{
        frame *recent = (frame*) pop(callStack);
        c0v_stack_t oldstack = recent->S;
        c0_value *oldarray = recent->V;
        ubyte *oldcode = recent->P;
        S = oldstack;
        V = oldarray;
        pc = recent->pc;
        P = oldcode;
        c0v_push(S,res);
        free(recent);
        break;
      }

      
    }
      



      
      

      
   


    /* Arithmetic and Logical operations */

    case IADD:{
      pc++;
      int32_t x = val2int(c0v_pop(S));
      int32_t y = val2int(c0v_pop(S));
      int32_t res = x + y;
      c0v_push(S,int2val(res));
      break;
    }

    case ISUB:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      int32_t res = x - y;
      c0v_push(S,int2val(res));
      break;
    }

    case IMUL:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      int32_t res = x * y;
      c0v_push(S,int2val(res));
      break;
    }

    case IDIV:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      if(y == 0){
        char* err = "division by zero nah";
        c0_arith_error(err);
      }
      
      int32_t x = val2int(c0v_pop(S));
      if(x == INT_MIN && y == -1){
        char* err2 = "can't do this in c0";
        c0_arith_error(err2);
      }
      int32_t res = x/y;
      c0v_push(S,int2val(res));
      break;
    }

    case IREM:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      if(y == 0){
        char* err = "division by zero nah";
        c0_arith_error(err);
      }
      int32_t x = val2int(c0v_pop(S));
      if(x == INT_MIN && y == -1){
        char* err2 = "this may or may not be an error idk";
        c0_arith_error(err2);
      }
      int32_t res = x%y;
      c0v_push(S,int2val(res));
      break;
    }

    case IAND:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      int32_t res = x & y;
      c0v_push(S,int2val(res));
      break;
    }

    case IOR:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      int32_t res = x | y;
      c0v_push(S,int2val(res));
      break;
    }

    case IXOR:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      int32_t res = x ^ y;
      c0v_push(S,int2val(res));
      break;
    }

    case ISHL:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if(y < 0 || y > 31)
      {
        char* err = "bit shift illegal";
        c0_arith_error(err);
      }
      int32_t res = x << y;
      c0v_push(S,int2val(res));
      break;
    }

    case ISHR:{
      pc++;
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if(y < 0 || y > 31)
      {
        char* err = "bit shift illegal";
        c0_arith_error(err);
      }
      int32_t res = x >> y;
      c0v_push(S,int2val(res));
      break;
    }


    /* Pushing constants */

    case BIPUSH:{
      pc++;
      int32_t thing = (int32_t)(int8_t)(P[pc]);
      c0v_push(S,int2val(thing));
      pc++;
      break;
    }

    case ILDC:{
      pc++;
      uint16_t c1 = (uint16_t)(P[pc]);
      pc++;
      uint16_t c2 = (uint16_t)(P[pc]);
      c0v_push(S,int2val(bc0->int_pool[(c1 << 8)|c2]));
      pc++;
      break;
    }

    case ALDC:{
      pc++;
      uint16_t c1 = (uint16_t)(P[pc]);
      pc++;
      uint16_t c2 = (uint16_t)(P[pc]);
      char *a = &bc0->string_pool[(c1 << 8) | c2];
      c0v_push(S,ptr2val((void*) a));
      pc++;
      break;
    }

    case ACONST_NULL:{
      pc++;
      c0v_push(S,ptr2val((void*) 0));
      break;
    }


    /* Operations on local variables */

    case VLOAD:{
      pc++;
      c0v_push(S,V[P[pc]]);
      pc++;
      break;
    }

    case VSTORE:{
      pc++;
      V[P[pc]] = c0v_pop(S);
      pc++;
      break;
    }


    /* Assertions and errors */

    case ATHROW:{
      pc++;
      c0_user_error((char*) (val2ptr(c0v_pop(S))));
      break;
    }

    case ASSERT:{
      pc++;
      void* a = val2ptr(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if(x == 0) c0_assertion_failure((char*) a);
      break;
    }


    /* Control flow operations */

    case NOP:{
      pc++;
      break;
    }

    case IF_CMPEQ:{
      pc++;
      uint16_t o1 = (uint16_t) P[pc];
      pc++;
      uint16_t o2 = (uint16_t) P[pc];
      int16_t offset = (int16_t) ((o1 << 8)|o2);
      if(val_equal(c0v_pop(S),c0v_pop(S)) && offset != 0) pc += (offset - 2);
      else{
        pc++;
      } 
      break;
    }

    case IF_CMPNE:{
      pc++;
      uint16_t o1 = (uint16_t) P[pc];
      pc++;
      uint16_t o2 = (uint16_t) P[pc];
      int16_t offset = (int16_t) ((o1 << 8)|o2);
      if(!val_equal(c0v_pop(S),c0v_pop(S)) && offset != 0) pc += (offset - 2);
      else{
        pc++;
      } 
      break;
    }

    case IF_ICMPLT:{
      pc++;
      uint16_t o1 = (uint16_t) P[pc];
      pc++;
      uint16_t o2 = (uint16_t) P[pc];
      int16_t offset = (int16_t) ((o1 << 8)|o2);
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if((x < y) && offset != 0) pc += (offset - 2);
      else{
        pc++;
      }
      break;
    }

    case IF_ICMPGE:{
      pc++;
      uint16_t o1 = (uint16_t) P[pc];
      pc++;
      uint16_t o2 = (uint16_t) P[pc];
      int16_t offset = (int16_t) ((o1 << 8)|o2);
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if((x >= y) && offset != 0) pc += (offset - 2);
      else{
        pc++;
      }
      break;
    }

    case IF_ICMPGT:{
      pc++;
      uint16_t o1 = (uint16_t) P[pc];
      pc++;
      uint16_t o2 = (uint16_t) P[pc];
      int16_t offset = (int16_t) ((o1 << 8)|o2);
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if((x > y) && offset != 0) pc += (offset - 2);
      else{
        pc++;
      }
      break;
    }

    case IF_ICMPLE:{
      pc++;
      uint16_t o1 = (uint16_t) P[pc];
      pc++;
      uint16_t o2 = (uint16_t) P[pc];
      int16_t offset = (int16_t) ((o1 << 8)|o2);
      int32_t y = val2int(c0v_pop(S));
      int32_t x = val2int(c0v_pop(S));
      if((x <= y) && offset != 0) pc += (offset - 2);
      else{
        pc++;
      }
      break;
    }

    case GOTO:{
      pc++;
      uint16_t o1 = (uint16_t) P[pc];
      pc++;
      uint16_t o2 = (uint16_t) P[pc];
      int16_t offset = (int16_t) ((o1 << 8)|o2);
      if(offset != 0) pc += (offset - 2);
      else pc++;
      break;
    }


    /* Function call operations: */

    case INVOKESTATIC:{
      pc++;
      uint16_t c1 = (uint16_t) P[pc];
      pc++;
      uint16_t c2 = (uint16_t) P[pc];
      uint16_t index = (c1<<8)|c2;
      uint16_t args = bc0->function_pool[index].num_args;
      uint16_t vars = bc0->function_pool[index].num_vars;
      c0_value *V2 = xcalloc(sizeof(c0_value),vars);
      int counter = args - 1;
      while(counter >= 0){
        V2[counter] = c0v_pop(S);
        counter -= 1;
      }
      pc++;
      frame *curr = xcalloc(sizeof(frame),1);
      curr->pc = pc;
      curr->V = V;
      curr->P = P;
      curr->S = S;
      push(callStack,(void*) curr);
      P = bc0->function_pool[index].code;
      V = V2;
      pc = 0;
      S = c0v_stack_new();
      break;
    }

    case INVOKENATIVE:{
      pc++;
      uint16_t c1 = (uint16_t) P[pc];
      pc++;
      uint16_t c2 = (uint16_t) P[pc];
      uint16_t index = (c1<<8)|c2;
      uint16_t args = bc0->native_pool[index].num_args;
      c0_value *things = xcalloc(sizeof(c0_value),args);
      int counter = args - 1;
      while(counter >= 0){
        things[counter] = c0v_pop(S);
        counter -= 1;
      }

      uint16_t i = bc0->native_pool[index].function_table_index;
      native_fn* f = native_function_table[i];
      c0_value res = (*f)(things);
      c0v_push(S,res);
      free(things);
      pc++;
      break;
    }


    /* Memory allocation operations: */

    case NEW:{
      pc++;
      uint8_t size =  (uint8_t) P[pc];
      void* loc = xcalloc(sizeof(char),size);
      c0v_push(S,ptr2val(loc));
      pc++;
      break;
    }

    case NEWARRAY:{
      pc++;
      int s = (int) (int8_t) P[pc];
      int n = (int) val2int(c0v_pop(S));
      c0_array *arr = xmalloc(sizeof(c0_array));
      void *data = xcalloc(s,n);
      arr->count = n;
      arr->elt_size = s;
      arr->elems = data;
      c0v_push(S,ptr2val((void*) arr));
      pc++;
      break;
    }

    case ARRAYLENGTH:{
      pc++;
      c0_array *a = (c0_array*) val2ptr(c0v_pop(S));
      int res = a->count;
      c0v_push(S,int2val(res));
      break;


    }


    /* Memory access operations: */

    case AADDF:{
      pc++;
      char *a = (char*) val2ptr(c0v_pop(S));
      if(a == NULL){
        char *s = "deref null pointer";
        c0_memory_error(s);
      }

      
      int8_t f = (int8_t) P[pc];
      void *res = (a + f);
      c0v_push(S,ptr2val(res));
      pc++;
      break;
    }

    case AADDS:{
      int i = val2int(c0v_pop(S));
      c0_array *a = (c0_array*) val2ptr(c0v_pop(S));
      
      if(i < 0 || i >= a->count)
      {
        char *err = "bad offset";
        c0_memory_error(err);
      }

      if(a == NULL)
      {
        char *s = "deref null pointer";
        c0_memory_error(s);
      }
      int elt_size = a->elt_size;
      char *temp = (char*) a->elems;
      int index = i * elt_size;
      char *res = (temp + index);
      c0v_push(S,ptr2val((void*) res));



      pc++;
      break;
    }

    case IMLOAD:{
      
      void* temp = val2ptr(c0v_pop(S));
      if(temp == NULL){
        char *s = "deref null pointer";
        c0_memory_error(s);
        
      }
      else{
        int *tempint = (int*) temp;
        c0v_push(S,int2val(*tempint));

      }
      pc++;
      break;
    }

    case IMSTORE:{
      int32_t x = val2int(c0v_pop(S));
      void *a = val2ptr(c0v_pop(S));
      int *actual = (int*) a;
      if(a == NULL){
        char *err = "deref null";
        c0_memory_error(err);
      }
      else{
        *actual = x;
        
      }
      pc++;
      break;
    }

    case AMLOAD:{
      void **a = (void**) val2ptr(c0v_pop(S));
      if(a == NULL){
        char *err = "deref null";
        c0_memory_error(err);
      }

      void *res = *a;
      c0v_push(S,ptr2val(res));

      pc++;
      break;
    }

    case AMSTORE:{
      void *b = val2ptr(c0v_pop(S));
      void **a = (void**) val2ptr(c0v_pop(S));
      if(a == NULL){
        char *err = "deref null";
        c0_memory_error(err);
      }
      *a = b;
      pc++;
      break;
    }

    case CMLOAD:{
      void *temp = val2ptr(c0v_pop(S));
      if(temp == NULL){
        char *err = "deref null";
        c0_memory_error(err);
      }
      char *a = (char*) temp;
      int x = (int32_t) *a;
      c0v_push(S,int2val(x));
      pc++;
      break;
    }

    case CMSTORE:{
      int32_t x = val2int(c0v_pop(S));
      char *a = (char*) val2ptr(c0v_pop(S));
      if(a == NULL){
        char *err = "deref null";
        c0_memory_error(err);
      }

      *a = x & 0x7F;

      pc++;
      break;

    }

    default:
      fprintf(stderr, "invalid opcode: 0x%02x\n", P[pc]);
      abort();
    }
  }

  /* cannot get here from infinite loop */
  assert(false);
}
