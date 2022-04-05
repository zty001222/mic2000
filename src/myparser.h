#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <stack>
#include <list>
#include <map>
#include <iterator>
#include <algorithm>
#include "koopa.h"

char asm_out[10000];

stack<int> calstack;
map<koopa_raw_value_t,int> saved_bin;
map<koopa_raw_value_t,int> saved_bin_stack;


char regs[7][3] = {"t0","t1","t2","t3","t4","t5","t6"};
bool used[7] = {0,0,0,0,0,0,0};
char us[7][3];
map<std::string, int> alloc_pos;
int sp_max;


void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_integer_t &intval);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_binary_t &bin);
void Visit(const koopa_raw_binary_op_t &op);
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_load_t &load);
void show_usable_regs();
void deal_with_IR(const char * str);

void show_usable_regs(){
  cout<<"usable regs:";
  for(int i = 0 ; i < 7 ; i ++){
    if(used[i] == 0)
      cout<<i<<" ";
  }
  cout<<endl;
}

void deal_with_IR(const char * str){
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  // 释放 Koopa IR 程序占用的内存

  Visit(raw);

  // 使用 for 循环遍历函数列表
  //for (size_t i = 0; i < raw.funcs.len; ++i) {
    // 正常情况下, 列表中的元素就是函数, 我们只不过是在确认这个事实
    // 当然, 你也可以基于 raw slice 的 kind, 实现一个通用的处理函数
    //assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
    // 获取当前函数
    //koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];
    // 进一步处理当前函数
    // @TODO
  //}

  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program builder 处理完毕之前释放 builder
  koopa_delete_program(program);
  koopa_delete_raw_program_builder(builder);
}

void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  std::cout<<"in program"<<endl;
  strcat(asm_out, "  .text\n");
  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  show_usable_regs();
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        std::cout<<"in slice func"<<endl;
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        std::cout<<"in slice block"<<endl;
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        std::cout<<"in slice value"<<endl;
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  cout<<"in func"<<endl;
  // 执行一些其他的必要操作
  strcat(asm_out, "  .globl main\nmain:\n");
  strcat(asm_out, "  addi sp, sp, -1024\n");
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  cout<<"in block"<<endl;
  // 执行一些其他的必要操作
  std::cout<<"in visit block, instlen:"<<bb->insts.len<<" paramslen:"<<bb->params.len<<endl;
  // 访问所有指令
  Visit(bb->insts);
  Visit(bb->params);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  const char * name = value->name;
  std::string tmpstring;
  if(name == NULL){cout<<"no name"<<endl;}
  else {std::cout<<"value name : "<<name<<endl;tmpstring = name;}
  if(saved_bin_stack.find(value) != saved_bin_stack.end()){
    cout<<"in value in stored"<<endl;
    int reg = 0;
      for(int i = 0 ; i < 7 ; i++){
          if(used[i] == 0){
              used[i] = 1;
              reg = i;
              break;
          }
      }
      strcat(asm_out, "  lw t");
      strcat(asm_out, to_string(reg).c_str());
      strcat(asm_out, ", ");
      strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
      strcat(asm_out, "(sp)\n");
      calstack.push(reg);
  }
  else if(name != NULL && (alloc_pos.find(tmpstring) != alloc_pos.end())){
    cout<<"in value in stored varible"<<endl;
    int reg = 0;
      for(int i = 0 ; i < 7 ; i++){
          if(used[i] == 0){
              used[i] = 1;
              reg = i;
              break;
          }
      }
      strcat(asm_out, "  lw t");
      strcat(asm_out, to_string(reg).c_str());
      strcat(asm_out, ", ");
      strcat(asm_out, to_string(alloc_pos[name]).c_str());
      strcat(asm_out, "(sp)\n");
      calstack.push(reg);
  }
  else{
    show_usable_regs();
    cout<<"in value nostore"<<endl;
    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    switch (kind.tag) {
      case KOOPA_RVT_RETURN:
        std::cout<<"in visit ret"<<endl;
        // 访问 return 指令
        Visit(kind.data.ret);
        break;
      case KOOPA_RVT_INTEGER:
        std::cout<<"in visit int"<<endl;
        // 访问 integer 指令
        Visit(kind.data.integer);
        break;
      case KOOPA_RVT_BINARY:
        std::cout<<"in visit bin"<<endl;
        if(saved_bin_stack.find(value) == saved_bin_stack.end()){
          Visit(kind.data.binary);
          //strcat(asm_out, "  addi sp, sp, 4\n");
          saved_bin_stack[value] = sp_max;
          strcat(asm_out, "  sw t");
          strcat(asm_out, to_string(calstack.top()).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
          strcat(asm_out, "(sp)\n");
          used[calstack.top()] = 0;
          calstack.pop();
          sp_max += 4;
        }
        else{
            int reg = 0;
            for(int i = 0 ; i < 7 ; i++){
                if(used[i] == 0){
                    used[i] = 1;
                    reg = i;
                    break;
                }
            }
            strcat(asm_out, "  lw t");
            strcat(asm_out, to_string(reg).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
            strcat(asm_out, "(sp)\n");
            calstack.push(reg);
        }
        break;
      case KOOPA_RVT_STORE:
          std::cout<<"in visit store"<<endl;
          Visit(kind.data.store);
          break;
      case KOOPA_RVT_ALLOC:
          std::cout<<"in visit alloc"<<endl;
          //strcat(asm_out, "  addi sp, sp, 4\n");
          if(name != NULL){
              std::string sname =  name;
              if(alloc_pos.find(sname) == alloc_pos.end()){
                  alloc_pos[sname] = sp_max;
              }
              // 访问 integer 指令
              sp_max += 4;
          }
          break;
      case KOOPA_RVT_GLOBAL_ALLOC: 
        std::cout<<"in visit g-alloc"<<endl;
        break;
      case KOOPA_RVT_LOAD:{
          Visit(kind.data.load);
          if(calstack.empty()){
            cout<<"empty error"<<endl;
          }
          int reg = calstack.top();
          used[reg] = 0;
          calstack.pop();
          saved_bin_stack[value] = sp_max;
          std::cout<<"in visit load"<<endl;
          strcat(asm_out, "  sw t");
          strcat(asm_out, to_string(reg).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(sp_max).c_str());
          strcat(asm_out, "(sp)\n");
          sp_max+=4;
          break;
      }
      default:
        // 其他类型暂时遇不到
        assert(false);
    }
  }
}
void Visit(const koopa_raw_integer_t &intval){
  std::cout<<"in integer"<<endl;
  cout<<intval.value<<endl;
  for(int i = 0 ; i < 7 ; i ++){
    if(used[i] == 0){
      used[i] = 1;
      strcat(asm_out,"  li t");
      strcat(asm_out,to_string(i).c_str());
      strcat(asm_out,", ");
      strcat(asm_out, to_string(intval.value).c_str());
      strcat(asm_out, "\n");
      calstack.push(i);
      break;
    }
  }
}

void Visit(const koopa_raw_return_t &ret){
  cout<<"in ret"<<endl;
  Visit(ret.value);
  int p1 = calstack.top();
  calstack.pop();
  strcat(asm_out,"  add a0, a0");
  strcat(asm_out,", t");
  strcat(asm_out,to_string(p1).c_str());
  strcat(asm_out,"\n");
  used[p1] = 0;
  strcat(asm_out, "  addi sp, sp, 1024\n");
  strcat(asm_out,"  ret\n");
}

void Visit(const koopa_raw_binary_t &bin){
  cout<<"in bin"<<endl;
    Visit(bin.lhs);
    Visit(bin.rhs);
    Visit(bin.op);
}

void Visit(const koopa_raw_binary_op_t &op){
  cout<<"in bin op"<<endl;
  switch(op){
    case KOOPA_RBO_NOT_EQ:{
      cout<<"in neq"<<endl;
      strcat(asm_out,"  sub");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");

      strcat(asm_out,"  snez");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    /// Equal to.
    case KOOPA_RBO_EQ:{
      cout<<"in eq"<<endl;
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out,"  sub");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");


      strcat(asm_out,"  seqz");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Greater than.
    case KOOPA_RBO_GT:{
      cout<<"in gt"<<endl;
      strcat(asm_out,"  sgt");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Less than.
    case KOOPA_RBO_LT:{
      cout<<"in lt"<<endl;
      strcat(asm_out,"  slt");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Greater than or 2qual to.
    case KOOPA_RBO_GE:{
      cout<<"in ge"<<endl;
      int tmp1 = 0;
      for(int i = 0 ; i < 7 ; i ++){
        if(used[i] == 0){
          used[i] = 1;
          tmp1 = i;
          break;
        }
      }
      strcat(asm_out,"  sgt");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(tmp1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");

      strcat(asm_out,"  sub");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");

      strcat(asm_out,"  seqz");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");

      strcat(asm_out,"  or");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(tmp1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      used[tmp1] = 0;
      break;
    }
    break;
    /// Less than or equal to.
    case KOOPA_RBO_LE:{
      cout<<"in le"<<endl;
      int tmp1 = 0;
      for(int i = 0 ; i < 7 ; i ++){
        if(used[i] == 0){
          used[i] = 1;
          tmp1 = i;
          break;
        }
      }
      strcat(asm_out,"  slt");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(tmp1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");

      strcat(asm_out,"  sub");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");

      strcat(asm_out,"  seqz");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");

      strcat(asm_out,"  or");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(tmp1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      used[tmp1] = 0;
      break;
    }
    break;
    /// Addition.
    case KOOPA_RBO_ADD:{
      cout<<"in add"<<endl;
      strcat(asm_out,"  add");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    /// Subtraction.
    case KOOPA_RBO_SUB:{
      cout<<"in sub"<<endl;
      strcat(asm_out,"  sub");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Multiplication.
    case KOOPA_RBO_MUL:{
      cout<<"in mul"<<endl;
      strcat(asm_out,"  mul");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Division.
    case KOOPA_RBO_DIV:{
      cout<<"in div"<<endl;
      strcat(asm_out,"  div");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Modulo.
    case KOOPA_RBO_MOD:{
      cout<<"in mod"<<endl;
      int tmp1 = 0;
      for(int i = 0 ; i < 7 ; i ++){
        if(used[i] == 0){
          used[i] = 1;
          tmp1 = i;
          break;
        }
      }
      strcat(asm_out,"  div");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(tmp1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");
      
      strcat(asm_out,"  mul");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(tmp1).c_str());
      strcat(asm_out,"\n");

      strcat(asm_out,"  sub");
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      used[tmp1] = 0;
      break;
    }
    break;
    /// Bitwise AND.
    case KOOPA_RBO_AND:{
      cout<<"in and"<<endl;
      strcat(asm_out,"  and");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Bitwise OR.
    case KOOPA_RBO_OR:{
      cout<<"in or"<<endl;
      strcat(asm_out,"  or");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Bitwise XOR.
    case KOOPA_RBO_XOR:{
      cout<<"in xor"<<endl;
      strcat(asm_out,"  xor");
      int p1 = calstack.top();
      calstack.pop();
      int p2 = calstack.top();
      strcat(asm_out," t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p1).c_str());
      strcat(asm_out,", t");
      strcat(asm_out,to_string(p2).c_str());
      strcat(asm_out,"\n");
      used[p1] = 0;
      break;
    }
    break;
    /// Shift left logical.
    case KOOPA_RBO_SHL:
    break;
    /// Shift right logical.
    case KOOPA_RBO_SHR:
    break;
    /// Shift right arithmetic.
    case KOOPA_RBO_SAR:
    break;
  }
}


void Visit(const koopa_raw_store_t &store){
    string sname = store.dest->name;
    
    Visit(store.value);
    int loadreg = calstack.top();
    calstack.pop();

    strcat(asm_out, "  sw t");
    strcat(asm_out, to_string(loadreg).c_str());
    strcat(asm_out, ", ");
    strcat(asm_out, to_string(alloc_pos[sname]).c_str());
    strcat(asm_out, "(sp)\n");
    
    used[loadreg] = 0;
}

void Visit(const koopa_raw_load_t &load){
    cout<< load.src<<endl;
    int reg = 0;
        for(int i = 0 ; i < 7 ; i++){ 
            //find aviliable reg
            if(used[i] == 0){
                used[i] = 1;
                reg = i;break;
            }
        }
    string sname = load.src->name;
    cout<<"load name:"<<sname<<endl;

    strcat(asm_out, "  lw t");
    strcat(asm_out, to_string(reg).c_str());
    strcat(asm_out, ", ");
    strcat(asm_out, to_string(alloc_pos[sname]).c_str());
    strcat(asm_out, "(sp)\n");
    calstack.push(reg);
    
}