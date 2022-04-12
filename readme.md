
      std::cout << koopa_str;
build/compiler -koopa hello.c -o hello.koopa
build/compiler -riscv hello.c -o hello.asm
sudo docker run -it --rm -v ~/Desktop:/root/compiler maxxing/compiler-dev bash
autotest -koopa -s lv3 /root/compiler
autotest -riscv -s lv3 /root/compiler

typedef struct {
  /// Buffer of slice items.
  const void **buffer;
  /// Length of slice.
  uint32_t len;
  /// Kind of slice items.
  koopa_raw_slice_item_kind_t kind;
} koopa_raw_slice_t;

ypedef enum {
  /// 32-bit integer.
  KOOPA_RTT_INT32,
  /// Unit (void).
  KOOPA_RTT_UNIT,
  /// Array (with base type and length).
  KOOPA_RTT_ARRAY,
  /// Pointer (with base type).
  KOOPA_RTT_POINTER,
  /// Function (with parameter types and return type).
  KOOPA_RTT_FUNCTION,
} koopa_raw_type_tag_t;


typedef struct {
  /// Global values (global allocations only).
  koopa_raw_slice_t values;
  /// Function definitions.
  koopa_raw_slice_t funcs;
} koopa_raw_program_t;

typedef struct {
  /// Type of function.
  koopa_raw_type_t ty;
  /// Name of function.
  const char *name;
  /// Parameters.
  koopa_raw_slice_t params;
  /// Basic blocks, empty if is a function declaration.
  koopa_raw_slice_t bbs;
} koopa_raw_function_data_t;


typedef struct {
  /// Name of basic block, null if no name.
  const char *name;
  /// Parameters.
  koopa_raw_slice_t params;
  /// Values that this basic block is used by.
  koopa_raw_slice_t used_by;
  /// Instructions in this basic block.
  koopa_raw_slice_t insts;
} koopa_ret_data_t;

typedef struct {
  /// Value of integer.
  int32_t value;
} koopa_raw_integer_t;

typedef struct {
  /// Elements.
  koopa_raw_slice_t elems;
} koopa_raw_aggregate_t;

///
/// Raw function argument reference.
///
typedef struct {
  /// Index.
  size_t index;
} koopa_raw_func_arg_ref_t;

///
/// Raw basic block argument reference.
///
typedef struct {
  /// Index.
  size_t index;
} koopa_raw_block_arg_ref_t;


Lv3:

Stmt        ::= "return" Exp ";";

Exp         ::= UnaryExp;
PrimaryExp  ::= "(" Exp ")" | Number;
Number      ::= INT_CONST;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";
MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp     ::= EqExp | LAndExp "&&" EqExp;
LOrExp      ::= LAndExp | LOrExp "||" LAndExp;


        myexp[0] = "%";
        strcat(myexp, to_string(*exp_depth).c_str());
        *exp_depth ++;



      int ttype;
      char exp1[10];
      char exp2[10];
      ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        strcpy(exp1, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        exp1[0] = '%';
        strcat(exp1, to_string(ttype).c_str());
      }
      if(ttype == -1){
        strcpy(exp2, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        exp2[0] = '%';
        strcat(exp2, to_string(ttype).c_str());
      }
      exp_depth += 1;
      char myexp[10];
      myexp[0] = '%';
      strcat(myexp, to_string(exp_depth).c_str());
      