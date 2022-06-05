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

char asm_out[100000000];

stack<int> calstack;
stack<int> immstack;
map<koopa_raw_value_t,int> saved_bin;
map<koopa_raw_value_t,int> saved_bin_stack;
map<string, int>tag_location;
int dpt = 0;
int tmpbj = 0;
string qcur_func;


char regs[7][3] = {"t0","t1","t2","t3","t4","t5","t6"};
bool used[7] = {0,0,0,0,0,0,0};
char params_regs[8][3] = {"a0","a1","a2","a3","a4","a5","a6","a7"};
bool params_used[8] = {0,0,0,0,0,0,0};
char us[7][3];
map<std::string, int> alloc_pos;
vector<int> tmp_array_size;
map<std::string, vector<int> > alloc_size;
int sp_max;
int calls;
int params;
int gallocs;
int galloc;
int zeroinit;
int sys_funcs = 8;
vector<string> funcalloc;


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
void Visit(const koopa_raw_jump_t &jmp);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_call_t &call);
void Visit(const koopa_raw_func_arg_ref_t &argref);
void Visit(const koopa_raw_global_alloc_t &galloc);
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
void check_info(){
  show_usable_regs();
  map<koopa_raw_value_t,int>::iterator it1;
  map<std::string, int>::iterator it2;
  map<std::string, vector<int> >::iterator it3;
  cout<<"bin stack"<<endl;
  for(it1 = saved_bin_stack.begin(); it1 != saved_bin_stack.end() ; it1++){
    cout<<it1->first<<" "<<it1->second<<endl;
  }
  cout<<"alloc pos"<<endl;
  for(it2 = alloc_pos.begin(); it2 != alloc_pos.end() ; it2++){
    cout<<it2->first<<" "<<it2->second<<endl;
  }
  cout<<"func"<<endl;
  for(int i = 0 ; i < funcalloc.size() ; i ++){
    cout<<funcalloc[i]<<endl;
  }
  cout<<"array"<<endl;
  for(it3 = alloc_size.begin(); it3 != alloc_size.end() ; it3++){
    cout<<it3->first<<" "<<it3->second.size()<<endl;
  }
  map<string,int>::iterator it11;
  for(it11 = tag_location.begin();it11 != tag_location.end(); it11++){
    cout<<"check tag\ntag_location: " << it11->first << ", " << it11->second<<endl;
  }
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
        if(galloc){
          std::cout<<"in slice value galloc"<<endl;
          if(!immstack.empty()){
            int imm = immstack.top();
            immstack.pop();
            strcat(asm_out, "  .word ");
            strcat(asm_out, to_string(imm).c_str());
            strcat(asm_out, "\n");
          }
        }
        if(calls){
          std::cout<<"in slice value calls"<<endl;
          if(calstack.empty()){
            cout<<"how come it's empty??????????"<<endl;
          }
          std::cout<<"stackok"<<endl;
          int reg = calstack.top();
          if(params < 8){
            strcat(asm_out, "  add a");
            strcat(asm_out, to_string(params).c_str());
            strcat(asm_out, ", zero");
            strcat(asm_out, ", t");
            strcat(asm_out, to_string(reg).c_str());
            strcat(asm_out, "\n");
            dpt += 4;
            params++;
          }
          else{
            if(200000-4*(params-7) > 1024){
              int spreg1 = 0;
              for(int i = 0 ; i < 7 ; i ++){
                if(!used[i]){spreg1 = i;break;}
              }
              strcat(asm_out, "  li t");
              strcat(asm_out, to_string(spreg1).c_str());
              strcat(asm_out, ", ");
              strcat(asm_out, to_string(200000-4*(params-7)).c_str());
              strcat(asm_out, "\n");

              strcat(asm_out, "  add t");
              strcat(asm_out, to_string(spreg1).c_str());
              strcat(asm_out, ", sp, t");
              strcat(asm_out, to_string(spreg1).c_str());
              strcat(asm_out, "\n");

              strcat(asm_out, "  sw t");
              strcat(asm_out, to_string(calstack.top()).c_str());
              strcat(asm_out, ", (t");
              strcat(asm_out, to_string(spreg1).c_str());
              strcat(asm_out, ")\n");
              dpt += 12;
            }
            else{
              strcat(asm_out, "  sw t");
              strcat(asm_out, to_string(calstack.top()).c_str());
              strcat(asm_out, ", ");
              strcat(asm_out, to_string(200000-4*(params-7)).c_str());
              strcat(asm_out, "(sp)\n");
              dpt += 4;
              
            }
            params++;
          }
          used[reg] = 0;
          calstack.pop();
        }
          std::cout<<"in slice value calls ends"<<endl;
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
  cout<<"func begin, check info"<<endl;
  check_info();
  // 执行一些其他的必要操作
  if(sys_funcs){
    sys_funcs--;return;
  }
  sp_max = 0;
  strcat(asm_out, "\n  .text\n");
  strcat(asm_out, "  .globl ");
  strcat(asm_out, func->name+1);
  strcat(asm_out, "\n");
  strcat(asm_out, func->name+1);
  strcat(asm_out,":\n");
  strcat(asm_out, "  li t0, 200000\n");
  strcat(asm_out, "  sub sp, sp, t0\n");
  strcat(asm_out, "  sw ra, 0(sp)\n");
  string tmptag = func->name + 1;
  qcur_func = tmptag;
  dpt = 0;
  tag_location[tmptag] = dpt;
  dpt = 12;
  sp_max+=4;
  Visit(func->params);
  // 访问所有基本块
  Visit(func->bbs);
  //clean var table
  for(int i = 0 ; i < func->params.len ; i++){
    auto ptr = func->params.buffer[i];
    cout<<reinterpret_cast<koopa_raw_value_t>(ptr)->name<<endl;
    cout<<reinterpret_cast<koopa_raw_value_t>(ptr)<<endl;
    alloc_pos.erase(reinterpret_cast<koopa_raw_value_t>(ptr)->name);
    saved_bin_stack = *new map<koopa_raw_value_t,int>();
  }
  while(!funcalloc.empty()){
    alloc_pos.erase(funcalloc.back());
    alloc_size.erase(funcalloc.back());
    funcalloc.pop_back();
  }
  
  cout<<"func end, check info"<<endl;
  check_info();
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  cout<<"in block"<<endl;
  // 执行一些其他的必要操作
  string tmptag = bb->name + 1;
  if(strcmp(bb->name,"%entry") == 0){
  }
  else{
    strcat(asm_out,bb->name + 1);
    strcat(asm_out,":\n");
    tag_location[tmptag] = dpt;
  }
  map<string,int>::iterator it1;
  for(it1 = tag_location.begin();it1 != tag_location.end(); it1++){
    cout<<"check tag\ntag_location: " << it1->first << ", " << it1->second<<endl;
  }
  std::cout<<"in visit block, instlen:"<<bb->insts.len<<" paramslen:"<<bb->params.len<<endl;
  // 访问所有指令
  Visit(bb->insts);
  Visit(bb->params);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  cout<<"in value"<<endl;
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
      if(saved_bin_stack[value]>1024){
        int spreg1 = 0;
        for(int i = 0 ; i < 7 ; i ++){
          if(!used[i]){spreg1 = i;break;}
        }
        strcat(asm_out, "  li t");
        strcat(asm_out, to_string(spreg1).c_str());
        strcat(asm_out, ", ");
        strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
        strcat(asm_out, "\n");

        strcat(asm_out, "  add t");
        strcat(asm_out, to_string(spreg1).c_str());
        strcat(asm_out, ", sp, t");
        strcat(asm_out, to_string(spreg1).c_str());
        strcat(asm_out, "\n");
        
        strcat(asm_out, "  lw t");
        strcat(asm_out, to_string(reg).c_str());
        strcat(asm_out, ", (t");
        strcat(asm_out, to_string(spreg1).c_str());
        strcat(asm_out, ")\n");
        dpt += 12;
      }
      else{
        strcat(asm_out, "  lw t");
        strcat(asm_out, to_string(reg).c_str());
        strcat(asm_out, ", ");
        strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
        strcat(asm_out, "(sp)\n");
        dpt += 4;
      }
      calstack.push(reg);
    cout<<"in value in stored has ended"<<endl;
  }
  else if(name != NULL && (alloc_pos.find(tmpstring) != alloc_pos.end())){
    cout<<"in value in stored varible"<<endl;
    int reg1 = 0;
      for(int i = 0 ; i < 7 ; i++){
          if(used[i] == 0){
              used[i] = 1;
              reg1 = i;
              break;
          }
      }
    int reg2 = 0;
      for(int i = 0 ; i < 7 ; i++){
          if(used[i] == 0){
              used[i] = 1;
              reg2 = i;
              break;
          }
      }
    int reg3 = 0;
      for(int i = 0 ; i < 7 ; i++){
          if(used[i] == 0){
              used[i] = 1;
              reg3 = i;
              break;
          }
      }
      if(alloc_pos[name] >= 0){
        if(alloc_pos[name] > 1024){
          int spreg1 = 0;
          for(int i = 0 ; i < 7 ; i ++){
            if(!used[i]){spreg1 = i;break;}
          }
          strcat(asm_out, "  li t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(alloc_pos[name]).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ", sp, t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  lw t");
          strcat(asm_out, to_string(reg1).c_str());
          strcat(asm_out, ", (t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ")\n");
          dpt += 12;
        }
        else{
          strcat(asm_out, "  lw t");
          strcat(asm_out, to_string(reg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(alloc_pos[name]).c_str());
          strcat(asm_out, "(sp)\n");
          dpt += 4;
        }
      }
      else{
        strcat(asm_out, "  la t");
        strcat(asm_out, to_string(reg2).c_str());
        strcat(asm_out, ", var\n");
        dpt += 4;

        if(-4-alloc_pos[name] >= 1024){
          strcat(asm_out, "  li t");
          strcat(asm_out, to_string(reg3).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(-4-alloc_pos[name]).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(reg2).c_str());
          strcat(asm_out, ", t");
          strcat(asm_out, to_string(reg2).c_str());
          strcat(asm_out, ", t");
          strcat(asm_out, to_string(reg3).c_str());
          strcat(asm_out, "\n");
          
          strcat(asm_out, "  lw t");
          strcat(asm_out, to_string(reg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, "(t");
          strcat(asm_out, to_string(reg2).c_str());
          strcat(asm_out, ")\n");
          dpt += 12;
        }
        else{
          strcat(asm_out, "  lw t");
          strcat(asm_out, to_string(reg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(-4-alloc_pos[name]).c_str());
          strcat(asm_out, "(t");
          strcat(asm_out, to_string(reg2).c_str());
          strcat(asm_out, ")\n");
          dpt += 4;
        }

      }
      used[reg2] = 0;
      used[reg3] = 0;
      calstack.push(reg1);
  }
  else{
    
    cout<<"in value nostore"<<endl;
    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    switch (kind.tag) {
      case KOOPA_RVT_RETURN:{
        show_usable_regs();
        std::cout<<"in visit ret"<<endl;
        // 访问 return 指令
        Visit(kind.data.ret);
        break;
      }
      case KOOPA_RVT_INTEGER:{
        show_usable_regs();
        std::cout<<"in visit int"<<endl;
        // 访问 integer 指令
        Visit(kind.data.integer);
        break;
      }
      case KOOPA_RVT_BINARY:{
        show_usable_regs();
        std::cout<<"in visit bin"<<endl;
        if(saved_bin_stack.find(value) == saved_bin_stack.end()){
          Visit(kind.data.binary);
          //strcat(asm_out, "  addi sp, sp, 4\n");
          saved_bin_stack[value] = sp_max;
          if(saved_bin_stack[value] > 1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");
        
            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(calstack.top()).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(calstack.top()).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 4;
          }

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
            if(saved_bin_stack[value]>1024){
              int spreg1 = 0;
              for(int i = 0 ; i < 7 ; i ++){
                if(!used[i]){spreg1 = i;break;}
              }
              strcat(asm_out, "  li t");
              strcat(asm_out, to_string(spreg1).c_str());
              strcat(asm_out, ", ");
              strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
              strcat(asm_out, "\n");

              strcat(asm_out, "  add t");
              strcat(asm_out, to_string(spreg1).c_str());
              strcat(asm_out, ", sp, t");
              strcat(asm_out, to_string(spreg1).c_str());
              strcat(asm_out, "\n");

              strcat(asm_out, "  lw t");
              strcat(asm_out, to_string(reg).c_str());
              strcat(asm_out, ", (t");
              strcat(asm_out, to_string(spreg1).c_str());
              strcat(asm_out, ")\n");
              dpt += 12;
            }
            else{
              strcat(asm_out, "  lw t");
              strcat(asm_out, to_string(reg).c_str());
              strcat(asm_out, ", ");
              strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
              strcat(asm_out, "(sp)\n");
              dpt += 4;
            }
            calstack.push(reg);
        }
        break;
      }
      case KOOPA_RVT_STORE:{
        show_usable_regs();
          std::cout<<"in visit store"<<endl;
          Visit(kind.data.store);
          break;
      }
      case KOOPA_RVT_ALLOC:{
        show_usable_regs();
          funcalloc.push_back(name);
          std::cout<<"in visit alloc"<<endl;
          //strcat(asm_out, "  addi sp, sp, 4\n");
          if(value->ty->tag== KOOPA_RTT_ARRAY){
            std::cout<<"in visit alloc array"<<endl;
            cout << "this array has dim len " <<value->ty->data.array.len << endl;
          }
          if(value->ty->tag== KOOPA_RTT_INT32){
            std::cout<<"in visit alloc int32"<<endl;
          }
          if(value->ty->tag== KOOPA_RTT_POINTER){
            std::cout<<"in visit alloc pointer"<<endl;
            std::cout<<"in visit alloc pointer : name is : "<< name << endl;
            std::cout<<"pointer base : "<< (value->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY )<< endl;
            int ttl_size = 4;
            auto base = value->ty->data.pointer.base;
            if(base->tag == KOOPA_RTT_ARRAY){
              alloc_size[name] = *new vector<int>();
              std::cout<<"point - array : "<< name << endl;
              while(base->tag == KOOPA_RTT_ARRAY){
                alloc_size[name].push_back(base->data.array.len);
                ttl_size *= base->data.array.len;
                cout<<"str len: "<<base->data.array.len<<endl;
                base = base->data.array.base;
              }
              alloc_pos[name] = sp_max;
              sp_max += ttl_size;
            }
            if(base->tag == KOOPA_RTT_POINTER){
              alloc_size[name] = *new vector<int>();
              std::cout<<"pointer pointer base : "<< (base->data.pointer.base->tag == KOOPA_RTT_POINTER )<< endl;
              std::cout<<"pointer array base : "<< (base->data.pointer.base->tag == KOOPA_RTT_ARRAY )<< endl;
              std::cout<<"pointer i32 base : "<< (base->data.pointer.base->tag == KOOPA_RTT_INT32 )<< endl;
              base = base->data.pointer.base;
              alloc_size[name].push_back(-1);
              while(base->tag == KOOPA_RTT_ARRAY){
                std::cout<<"pointer and pointer and array : " << endl;
                alloc_size[name].push_back(base->data.array.len);
                cout<<"str len: "<<base->data.array.len<<endl;
                base = base->data.array.base;
              }
              alloc_pos[name] = sp_max;
              sp_max += ttl_size;
            }
            // i32
            else if(base->tag == KOOPA_RTT_INT32){              
              std::cout<<"i32 base : "<< endl;
              std::string sname =  name;
              if(alloc_pos.find(sname) == alloc_pos.end()){
                  alloc_pos[sname] = sp_max;
              }
              // 访问 integer 指令
              sp_max += 4;
            }
          }
          /*
          if(name != NULL){
              std::string sname =  name;
              if(alloc_pos.find(sname) == alloc_pos.end()){
                  alloc_pos[sname] = sp_max;
              }
              funcalloc.push_back(sname);
              // 访问 integer 指令
              sp_max += 4;
          }
          */
          break;
      }
      case KOOPA_RVT_GLOBAL_ALLOC: {
        show_usable_regs();
        galloc = 1;
        zeroinit = 0;
        std::cout<<"in visit g-alloc"<<endl;
        if(gallocs == 0){
          strcat(asm_out, "  .data\n  .globl var\nvar:\n");
        }
        if(value->ty->tag== KOOPA_RTT_POINTER){
          std::cout<<"in visit galloc pointer"<<endl;
          std::cout<<"in visit galloc pointer : name is : "<< name << endl;
          std::cout<<"pointer base : "<< (value->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY )<< endl;
          int ttl_size = 4;
          auto base = value->ty->data.pointer.base;
          if(base->tag == KOOPA_RTT_ARRAY){
            alloc_size[name] = *new vector<int>();
            while(base->tag == KOOPA_RTT_ARRAY){
              alloc_size[name].push_back(base->data.array.len);
              ttl_size *= base->data.array.len;
              cout<<"str len: "<<base->data.array.len<<endl;
              base = base->data.array.base;
            }
              
            alloc_pos[name] = -(4*(1+gallocs));
            gallocs += ttl_size/4;
            Visit(kind.data.global_alloc);
            galloc = 0;
            if(zeroinit){
              strcat(asm_out, "  .zero ");
              strcat(asm_out, to_string(ttl_size).c_str());
              strcat(asm_out, "\n");
            }
          }
          else if(base->tag == KOOPA_RTT_POINTER){
            alloc_size[name] = *new vector<int>();
            std::cout<<"pointer pointer base : "<< (value->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY )<< endl;
            auto base = value->ty->data.pointer.base;
            alloc_size[name].push_back(-1);
            while(base->tag == KOOPA_RTT_ARRAY){
              alloc_size[name].push_back(base->data.array.len);
              ttl_size *= base->data.array.len;
              cout<<"str len: "<<base->data.array.len<<endl;
              base = base->data.array.base;
            }
            alloc_pos[name] = -(4*(1+gallocs));
            gallocs += ttl_size/4;
            Visit(kind.data.global_alloc);
            galloc = 0;
            if(zeroinit){
              strcat(asm_out, "  .zero ");
              strcat(asm_out, to_string(ttl_size).c_str());
              strcat(asm_out, "\n");
            }
          }
          // i32
          else if(base->tag == KOOPA_RTT_INT32){              
            std::cout<<"g i32 base : "<< endl;
            std::string sname =  name;
            if(alloc_pos.find(sname) == alloc_pos.end()){
                alloc_pos[sname] = -(4*(1+gallocs));;
            }
            Visit(kind.data.global_alloc);
            galloc = 0;
            // 访问 integer 指令
            gallocs += 1;
            if(zeroinit)
              strcat(asm_out, "  .zero 4\n");
            else{
              strcat(asm_out, "  .word ");
              strcat(asm_out, to_string(immstack.top()).c_str());
              immstack.pop();
              strcat(asm_out, "\n");
            }
          }
        }
        break;
      }
      case KOOPA_RVT_LOAD:{
        show_usable_regs();
          std::cout<<"in visit load"<<endl;
          Visit(kind.data.load);
          if(calstack.empty()){
            cout<<"empty error"<<endl;
          }
          int reg = calstack.top();
          calstack.pop();
          saved_bin_stack[value] = sp_max;
          if(sp_max > 1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(reg).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(reg).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 4;
          }
          used[reg] = 0;
          sp_max+=4;
          break;
      }
      case KOOPA_RVT_BRANCH:{
          cout<<"in branch"<<endl;
          Visit(kind.data.branch);
          break;
      }
      case KOOPA_RVT_JUMP:{
          cout<<"in jump"<<endl;  
          Visit(kind.data.jump);
          break;
      }
      case KOOPA_RVT_CALL:{
          Visit(kind.data.call);
          saved_bin_stack[value] = sp_max;
          if(saved_bin_stack[value] >1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(calstack.top()).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(calstack.top()).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(saved_bin_stack[value]).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 4;
          }

          used[calstack.top()] = 0;
          calstack.pop();
          sp_max += 4;
        break;
      }
      case KOOPA_RVT_FUNC_ARG_REF:{
        show_usable_regs();
        cout<<"this name is "<<value->name<<endl;
        Visit(kind.data.func_arg_ref);
        if(kind.data.func_arg_ref.index < 8){
          if(sp_max > 1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  sw a");
            strcat(asm_out, to_string(kind.data.func_arg_ref.index).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            strcat(asm_out, "  sw a");
            strcat(asm_out, to_string(kind.data.func_arg_ref.index).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 4;
          }
          alloc_pos[value->name] = sp_max;
          sp_max += 4;
        }
        else{
          int tmp = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp = i;used[i] = 1;break;}
          }
          int tmpshift = 400000-4*(kind.data.func_arg_ref.index-7);
          if(tmpshift > 1024 || sp_max > 1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(tmpshift).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  lw t");
            strcat(asm_out, to_string(tmp).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;

            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            strcat(asm_out, "  lw t");
            strcat(asm_out, to_string(tmp).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(tmpshift).c_str());
            strcat(asm_out, "(sp)\n");

            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 8;
          }
          used[tmp] = 0;
          alloc_pos[value->name] = sp_max;
          sp_max += 4;
        }
        break;
      }
      case KOOPA_RVT_ZERO_INIT:{
          cout<<"in zero init"<<endl;  
          zeroinit = 1;
          break;
      }
      case KOOPA_RVT_GET_PTR:{
        show_usable_regs();
        cout<<"in get ptr"<<endl;  
        vector<int> saved_vec = *new vector<int>();
        string sname  = "";
        if(value->kind.data.get_elem_ptr.src->name)
          sname = value->kind.data.get_elem_ptr.src->name;
        if(sname != ""){
          cout<<"has name, this one is the first element of the array : "<<sname<<endl;
          tmp_array_size = *new vector<int>();
          int shift = 4;
          for(int i = 1 ; i < alloc_size[sname].size(); i++){
            shift *= alloc_size[sname][i];
            tmp_array_size.push_back(alloc_size[sname][i]);
          }
          int tmp1 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp1 = i;used[i] = 1;break;}
          }
          int tmp2 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp2 = i;used[i] = 1;break;}
          }
          int tmp3 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp3 = i;used[i] = 1;break;}
          }
          if(alloc_pos[sname] >= 0){
            
            if(alloc_pos[sname] >= 1024){
              strcat(asm_out, "  li t");
              strcat(asm_out, to_string(tmp3).c_str());
              strcat(asm_out, ", ");                             
              strcat(asm_out, to_string(alloc_pos[sname]).c_str());
              strcat(asm_out, "\n");      

              strcat(asm_out, "  add t");
              strcat(asm_out, to_string(tmp1).c_str());
              strcat(asm_out, ", sp, t");  
              strcat(asm_out, to_string(tmp3).c_str());
              strcat(asm_out, "\n");   
              dpt += 8; 
            }
            else{
              strcat(asm_out, "  addi t");
              strcat(asm_out, to_string(tmp1).c_str());
              strcat(asm_out, ", sp, ");               
              strcat(asm_out, to_string(alloc_pos[sname]).c_str());
              strcat(asm_out, "\n");  
              dpt += 4;    

            }                        

            used[tmp3] = 0;

            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(tmp2).c_str());
            strcat(asm_out, ", "); 
            strcat(asm_out, to_string(shift).c_str());
            strcat(asm_out, "\n"); 
            dpt += 4;
          }
          else{

            strcat(asm_out, "  la t");
            strcat(asm_out, to_string(tmp2).c_str());
            strcat(asm_out, ", var\n"); 
            dpt += 4;

            if(-4-alloc_pos[sname] >= 1024){
              strcat(asm_out, "  li t");
              strcat(asm_out, to_string(tmp3).c_str());                
              strcat(asm_out, ", ");                             
              strcat(asm_out, to_string(-4-alloc_pos[sname]).c_str());  
              strcat(asm_out, "\n"); 

              strcat(asm_out, "  add t");
              strcat(asm_out, to_string(tmp1).c_str());
              strcat(asm_out, ", t");   
              strcat(asm_out, to_string(tmp2).c_str());                   
              strcat(asm_out, ", t");   
              strcat(asm_out, to_string(tmp3).c_str());    
              strcat(asm_out, "\n");  
              dpt += 8; 
            }
            else{
              strcat(asm_out, "  addi t");
              strcat(asm_out, to_string(tmp1).c_str());
              strcat(asm_out, ", t");   
              strcat(asm_out, to_string(tmp2).c_str());     
              strcat(asm_out, ", ");                             
              strcat(asm_out, to_string(-4-alloc_pos[sname]).c_str());  
              strcat(asm_out, "\n"); 
              dpt += 4;
            }

            used[tmp3] = 0;   

            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(tmp2).c_str());
            strcat(asm_out, ", "); 
            strcat(asm_out, to_string(shift).c_str());
            strcat(asm_out, "\n"); 
            dpt += 4;

          }
          saved_vec = tmp_array_size;
          Visit(value->kind.data.get_elem_ptr.index);
          tmp_array_size = saved_vec;
          // get ref
          int ttype = calstack.top();
          calstack.pop();

          strcat(asm_out, "  mul t");
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(ttype).c_str());
          strcat(asm_out, "\n"); 

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, "\n"); 
          dpt += 8;
          if(sp_max > 1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            used[tmp2] = 0;
            used[ttype] = 0;
            used[tmp1] = 0;
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp1).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            used[tmp2] = 0;
            used[ttype] = 0;
            used[tmp1] = 0;
            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 4;
          }
          check_info();
          saved_bin_stack[value] = sp_max;
          sp_max += 4;
          cout<<"finish the first element of the array : "<<sname<<endl;
        }
        else{
          check_info();
          cout<<"this is not the beginning of the array"<<endl;  
          Visit(value->kind.data.get_elem_ptr.src);
          int ttype = calstack.top();
          calstack.pop();
          cout<<"finish checking ref, tmp array has size : "<< tmp_array_size.size() << endl;  
          int shift = 4;
          for(int i = 1 ; i < tmp_array_size.size(); i++){
            shift *= tmp_array_size[i];
          }
          tmp_array_size.erase(tmp_array_size.begin());
          int tmp1 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp1 = i;used[i] = 1;break;}
          }
          int tmp2 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp2 = i;used[i] = 1;break;}
          }
          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", zero, t");                             
          strcat(asm_out, to_string(ttype).c_str());
          strcat(asm_out, "\n");                 
          used[ttype] = 0;           

          strcat(asm_out, "  li t");
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", "); 
          strcat(asm_out, to_string(shift).c_str());
          strcat(asm_out, "\n"); 
          dpt += 8;
          saved_vec = tmp_array_size;
          cout<<"going to checking index"<<endl;  
          Visit(value->kind.data.get_elem_ptr.index);
          cout<<"finish checking index"<<endl;  
          tmp_array_size = saved_vec;
          // get ref
          ttype = calstack.top();
          calstack.pop();

          strcat(asm_out, "  mul t");
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(ttype).c_str());
          strcat(asm_out, "\n"); 

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, "\n"); 
          dpt += 8;
          if(sp_max > 1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            used[tmp2] = 0;
            used[ttype] = 0;
            used[tmp1] = 0;
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp1).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            used[tmp2] = 0;
            used[ttype] = 0;
            used[tmp1] = 0;
            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 4;
          }
          saved_bin_stack[value] = sp_max;
          sp_max += 4;
          cout<<"finish not the first element of the array : "<<sname<<endl;
          // meet the end, push 
        }
        break;
      }

      case KOOPA_RVT_GET_ELEM_PTR:{
        cout<<"in get elem ptr"<<endl;  
        vector<int> saved_vec = *new vector<int>();
        string sname  = "";
        if(value->kind.data.get_elem_ptr.src->name)
          sname = value->kind.data.get_elem_ptr.src->name;
        if(sname != ""){
          cout<<"has name, this one is the first element of the array : "<<sname<<endl;
          tmp_array_size = *new vector<int>();
          int shift = 4;
          for(int i = 1 ; i < alloc_size[sname].size(); i++){
            shift *= alloc_size[sname][i];
            tmp_array_size.push_back(alloc_size[sname][i]);
          }
          int tmp1 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp1 = i;used[i] = 1;break;}
          }
          int tmp2 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp2 = i;used[i] = 1;break;}
          }
          int tmp3 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp3 = i;used[i] = 1;break;}
          }
          if(alloc_pos[sname] >= 0){
            if(alloc_pos[sname] >= 1024){
              strcat(asm_out, "  li t");
              strcat(asm_out, to_string(tmp3).c_str());
              strcat(asm_out, ", ");                             
              strcat(asm_out, to_string(alloc_pos[sname]).c_str());
              strcat(asm_out, "\n");     

              strcat(asm_out, "  add t");
              strcat(asm_out, to_string(tmp1).c_str());
              strcat(asm_out, ", sp, t");                             
              strcat(asm_out, to_string(tmp3).c_str());
              strcat(asm_out, "\n");   
              dpt += 8;   
           
            }
            else{
              strcat(asm_out, "  addi t");
              strcat(asm_out, to_string(tmp1).c_str());
              strcat(asm_out, ", sp,");                             
              strcat(asm_out, to_string(alloc_pos[sname]).c_str());
              strcat(asm_out, "\n");   
              dpt += 4;    
            }         
            used[tmp3] = 0;  

            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(tmp2).c_str());
            strcat(asm_out, ", "); 
            strcat(asm_out, to_string(shift).c_str());
            strcat(asm_out, "\n"); 
            dpt += 4;
          }
          else{
            strcat(asm_out, "  la t");
            strcat(asm_out, to_string(tmp2).c_str());
            strcat(asm_out, ", var\n"); 
            dpt += 4;
            if(-4-alloc_pos[sname] >= 1024){

              strcat(asm_out, "  li t");
              strcat(asm_out, to_string(tmp3).c_str());              
              strcat(asm_out, ", ");                             
              strcat(asm_out, to_string(-4-alloc_pos[sname]).c_str());  
              strcat(asm_out, "\n");     

              strcat(asm_out, "  add t");
              strcat(asm_out, to_string(tmp1).c_str());
              strcat(asm_out, ", t");   
              strcat(asm_out, to_string(tmp2).c_str());                    
              strcat(asm_out, ", t");                             
              strcat(asm_out, to_string(tmp3).c_str());  
              strcat(asm_out, "\n");    
              dpt += 8;
            }
            else{
              strcat(asm_out, "  addi t");
              strcat(asm_out, to_string(tmp1).c_str());
              strcat(asm_out, ", t");   
              strcat(asm_out, to_string(tmp2).c_str());   
              strcat(asm_out, ", ");                             
              strcat(asm_out, to_string(-4-alloc_pos[sname]).c_str());  
              strcat(asm_out, "\n");   
              dpt += 4;   
            }  

            used[tmp3] = 0;
            
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(tmp2).c_str());
            strcat(asm_out, ", "); 
            strcat(asm_out, to_string(shift).c_str());
            strcat(asm_out, "\n"); 
            dpt += 4;
          }
          saved_vec = tmp_array_size;
          Visit(value->kind.data.get_elem_ptr.index);
          tmp_array_size = saved_vec;
          // get ref
          int ttype = calstack.top();
          calstack.pop();

          strcat(asm_out, "  mul t");
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(ttype).c_str());
          strcat(asm_out, "\n"); 

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, "\n"); 
          dpt += 8;
          if(sp_max > 1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp1).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 4;
          }
          used[tmp2] = 0;
          used[ttype] = 0;
          used[tmp1] = 0;
          saved_bin_stack[value] = sp_max;
          sp_max += 4;
          cout<<"finish the first element of the array : "<<sname<<endl;
        }
        else{
          cout<<"this is not the beginning of the array"<<endl;  
          Visit(value->kind.data.get_elem_ptr.src);
          cout<<"finish parsing src %"<<endl;  
          int ttype = calstack.top();
          calstack.pop();
          int shift = 4;
          cout<<"till now the array has size : "<<tmp_array_size.size()<<endl;  
          for(int i = 1 ; i < tmp_array_size.size(); i++){
            shift *= tmp_array_size[i];
          }
          tmp_array_size.erase(tmp_array_size.begin());
          int tmp1 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp1 = i;used[i] = 1;break;}
          }
          int tmp2 = 0;
          for(int i = 0; i < 7 ; i++){
            if(used[i] == 0){tmp2 = i;used[i] = 1;break;}
          }
          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", zero, t");                             
          strcat(asm_out, to_string(ttype).c_str());
          strcat(asm_out, "\n");                 
          used[ttype] = 0;           

          strcat(asm_out, "  li t");
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", "); 
          strcat(asm_out, to_string(shift).c_str());
          strcat(asm_out, "\n"); 
          dpt += 8;
          saved_vec = tmp_array_size;
          Visit(value->kind.data.get_elem_ptr.index);
          cout<<"finish parsing index %"<<endl;  
          tmp_array_size = saved_vec;
          // get ref
          ttype = calstack.top();
          calstack.pop();

          strcat(asm_out, "  mul t");
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(ttype).c_str());
          strcat(asm_out, "\n"); 

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp1).c_str());
          strcat(asm_out, ", t"); 
          strcat(asm_out, to_string(tmp2).c_str());
          strcat(asm_out, "\n"); 
          dpt += 8;
          if(sp_max > 1024){
            int spreg1 = 0;
            for(int i = 0 ; i < 7 ; i ++){
              if(!used[i]){spreg1 = i;break;}
            }
            strcat(asm_out, "  li t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  add t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ", sp, t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, "\n");

            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp1).c_str());
            strcat(asm_out, ", (t");
            strcat(asm_out, to_string(spreg1).c_str());
            strcat(asm_out, ")\n");
            dpt += 12;
          }
          else{
            strcat(asm_out, "  sw t");
            strcat(asm_out, to_string(tmp1).c_str());
            strcat(asm_out, ", ");
            strcat(asm_out, to_string(sp_max).c_str());
            strcat(asm_out, "(sp)\n");
            dpt += 4;
          }

          used[tmp2] = 0;
          used[ttype] = 0;
          used[tmp1] = 0;
          saved_bin_stack[value] = sp_max;
          sp_max += 4;
          cout<<"finish not the first element of the array : "<<sname<<endl;
          // meet the end, push 
        }
        break;
      }
      
      case KOOPA_RVT_UNDEF:{
        show_usable_regs();
        cout<<"in undef"<<endl;  
        break;
      }
      case KOOPA_RVT_AGGREGATE:{
        show_usable_regs();
        cout<<"in agg"<<endl;  
        Visit(kind.data.aggregate.elems);
        break;
      }
      case KOOPA_RVT_BLOCK_ARG_REF:{
        show_usable_regs();
        cout<<"in block arg"<<endl;  
        break;
      }
      default:{
        // 其他类型暂时遇不到
        assert(false);
      }
    }
  }
}


void Visit(const koopa_raw_global_alloc_t &galloc){
  cout<<"in galloc"<<endl;
  Visit(galloc.init);
}

void Visit(const koopa_raw_func_arg_ref_t &argref){
  cout<<"in argref"<<endl;
  int nums = argref.index;
  cout<<"number of params is "<<nums<<endl;
}
 
void Visit(const koopa_raw_call_t &call){
  cout<<"in call"<<endl;
  calls = 1;
  params = 0;
  Visit(call.args);
  calls = 0;
  params = 0;
  strcat(asm_out, "  call ");
  strcat(asm_out, call.callee->name + 1);
  strcat(asm_out, "\n");
  dpt += 4;
  for(int i = 0 ; i < 7 ;i ++){
    if(used[i] == 0){
      strcat(asm_out, "  add t");
      strcat(asm_out, to_string(i).c_str());
      strcat(asm_out, ", zero, a0\n");
      dpt += 4;
      used[i] = 1;
      calstack.push(i);
      break;
    }
  }
}

void Visit(const koopa_raw_branch_t &branch){
  cout<<"in branch"<<endl;
  cout<<branch.true_bb->name<<endl;
  Visit(branch.cond);
  int reg = calstack.top();
  calstack.pop();
  int tmp1 = 0;
  for(int i = 0 ; i < 7 ; i ++){
    if(used[i] == 0){
      used[i] = 1;
      tmp1 = i;
      break;
    }
  }
  int tmp2 = 0;
  for(int i = 0 ; i < 7 ; i ++){
    if(used[i] == 0){
      used[i] = 1;
      tmp2 = i;
      break;
    }
  }
  string tbbn = branch.true_bb->name + 1;
  string fbbn = branch.false_bb->name + 1;
  if(1){
    strcat(asm_out, "  la t");
    strcat(asm_out, to_string(tmp1).c_str());
    strcat(asm_out, ", ");
    strcat(asm_out, tbbn.c_str());
    strcat(asm_out, "\n");

    strcat(asm_out, "  beqz t");
    strcat(asm_out, to_string(reg).c_str());
    strcat(asm_out, ", branch_tmp_");
    strcat(asm_out, to_string(tmpbj).c_str());
    strcat(asm_out, "\n");

    strcat(asm_out, "  jr t");
    strcat(asm_out, to_string(tmp1).c_str());
    strcat(asm_out, "\n");

    strcat(asm_out, "branch_tmp_");
    strcat(asm_out, to_string(tmpbj).c_str());
    strcat(asm_out, ":\n");
    tmpbj += 1;
    dpt += 16;
  }
  else{
    strcat(asm_out, "  bnez t");
    strcat(asm_out, to_string(reg).c_str());
    strcat(asm_out, ", ");
    strcat(asm_out, tbbn.c_str());
    strcat(asm_out, "\n");
  }
  used[tmp1] = 0;
  used[tmp2] = 0;

  if(1){
    strcat(asm_out, "  la t");
    strcat(asm_out, to_string(tmp1).c_str());
    strcat(asm_out, ", ");
    strcat(asm_out, fbbn.c_str());
    strcat(asm_out, "\n");

    strcat(asm_out, "  jr t");
    strcat(asm_out, to_string(tmp1).c_str());
    strcat(asm_out, "\n");
    dpt += 8;
  }
  else{
    strcat(asm_out, "  j ");
    strcat(asm_out, fbbn.c_str());
    strcat(asm_out, "\n");
    dpt += 4;
  }
  used[tmp1] = 0;
  used[tmp2] = 0;


  used[reg] = 0;
}

void Visit(const koopa_raw_jump_t &jmp){
  cout<<"in jmp"<<endl;
  cout<<jmp.target->name<<endl;
  char name[20];
  int tmp1 = 0;
  for(int i = 0 ; i < 7 ; i ++){
    if(used[i] == 0){
      used[i] = 1;
      tmp1 = i;
      break;
    }
  }
  int tmp2 = 0;
  for(int i = 0 ; i < 7 ; i ++){
    if(used[i] == 0){
      used[i] = 1;
      tmp2 = i;
      break;
    }
  }
  string jn = jmp.target->name + 1;
  if(1){
    strcat(asm_out, "  la t");
    strcat(asm_out, to_string(tmp1).c_str());
    strcat(asm_out, ", ");
    strcat(asm_out, jn.c_str());
    strcat(asm_out, "\n");

    strcat(asm_out, "  jr t");
    strcat(asm_out, to_string(tmp1).c_str());
    strcat(asm_out, "\n");
    dpt += 8;
  }
  else{
    strcpy(name, jn.c_str());
    strcat(asm_out, "  j ");
    strcat(asm_out, name);
    strcat(asm_out, "\n");
    dpt += 4;
  }
  used[tmp1] = 0;
  used[tmp2] = 0;
}

void Visit(const koopa_raw_integer_t &intval){
  std::cout<<"in integer"<<endl;
  cout<<intval.value<<endl;
  for(int i = 0 ; i < 7 ; i ++){
    if(used[i] == 0){
      used[i] = 1;
      if(!galloc){
        cout<<"this is not galloc"<<endl;
        strcat(asm_out,"  li t");
        strcat(asm_out,to_string(i).c_str());
        strcat(asm_out,", ");
        strcat(asm_out, to_string(intval.value).c_str());
        strcat(asm_out, "\n");
        dpt += 4;
        calstack.push(i);
      }
      else{
        used[i] = 0;
        cout<<"this is galloc"<<endl;
        immstack.push(intval.value);
      }
      break;
    }
  }
}

void Visit(const koopa_raw_return_t &ret){
  cout<<"in ret"<<endl;
  if(ret.value){
    Visit(ret.value);
    int p1 = calstack.top();
    calstack.pop();
    strcat(asm_out,"  add a0, zero");
    strcat(asm_out,", t");
    strcat(asm_out,to_string(p1).c_str());
    strcat(asm_out,"\n");
    used[p1] = 0;
    strcat(asm_out, "  lw ra, 0(sp)\n");
    strcat(asm_out, "  li t0, 200000\n");
    strcat(asm_out, "  add sp, sp, t0\n");
    strcat(asm_out,"  ret\n");
    dpt += 8;
  }
  else{
    strcat(asm_out, "  lw ra, 0(sp)\n");
    strcat(asm_out, "  li t0, 200000\n");
    strcat(asm_out, "  add sp, sp, t0\n");
    strcat(asm_out,"  ret\n");
    dpt += 4;
  }
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
      dpt += 8;
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
      dpt += 8;
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
      dpt += 4;
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
      dpt += 4;
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
      dpt += 16;
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
      dpt += 16;
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
      dpt += 4;
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
      dpt += 4;
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
      dpt += 4;
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
      dpt += 4;
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
      dpt += 12;
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
      dpt += 4;
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
      dpt += 4;
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
      dpt += 4;
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
    cout<<"in store  "<<endl;
    string sname = "";
    if(store.dest->name){
      cout<<"in store name "<<endl;
      sname = store.dest->name;
      cout<<"store name is: "<<sname<<endl;
    
      Visit(store.value);
      int loadreg = calstack.top();
      calstack.pop();
      
      int reg = 0;
      for(int i = 0 ; i < 7 ; i ++){
        if(used[i] == 0){
          used[i] = 1;
          reg = i;
          break;
        }
      }
      int areg = 0;
      for(int i = 0 ; i < 7 ; i ++){
        if(used[i] == 0){
          used[i] = 1;
          areg = i;
          break;
        }
      }

      if(alloc_pos[sname] >= 0){
        if(alloc_pos[sname] > 1024){
          int spreg1 = 0;
          for(int i = 0 ; i < 7 ; i ++){
            if(!used[i]){spreg1 = i;break;}
          }
          strcat(asm_out, "  li t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(alloc_pos[sname]).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ", sp, t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  sw t");
          strcat(asm_out, to_string(loadreg).c_str());
          strcat(asm_out, ", (t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ")\n");
          dpt += 12;
        }
        else{
          strcat(asm_out, "  sw t");
          strcat(asm_out, to_string(loadreg).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(alloc_pos[sname]).c_str());
          strcat(asm_out, "(sp)\n");
          dpt += 4;
        }

      }
      else{
        strcat(asm_out, "  la t");
        strcat(asm_out, to_string(reg).c_str());
        strcat(asm_out, ", var\n");
        dpt += 4;
        if(-4-alloc_pos[sname] < 1024){
          strcat(asm_out, "  sw t");
          strcat(asm_out, to_string(loadreg).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(-4-alloc_pos[sname]).c_str());
          strcat(asm_out, "(t");
          strcat(asm_out, to_string(reg).c_str());
          strcat(asm_out, ")\n");
          dpt += 4;
        }
        else{
          strcat(asm_out, "  li t");
          strcat(asm_out, to_string(areg).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(-4-alloc_pos[sname]).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(reg).c_str());
          strcat(asm_out, ", t");
          strcat(asm_out, to_string(reg).c_str());
          strcat(asm_out, ", t");
          strcat(asm_out, to_string(areg).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  sw t");
          strcat(asm_out, to_string(loadreg).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, "(t");
          strcat(asm_out, to_string(reg).c_str());
          strcat(asm_out, ")\n");
          dpt += 12;
        }
      }
      
      used[reg] = 0;
      used[areg] = 0;
      used[loadreg] = 0;
    }
    else{
      cout<<"in store no name "<<endl;
      Visit(store.dest);
      int destreg = calstack.top();
      calstack.pop();

      Visit(store.value);
      int loadreg = calstack.top();
      calstack.pop();
      
      int reg = 0;
      for(int i = 0 ; i < 7 ; i ++){
        if(used[i] == 0){
          used[i] = 1;
          reg = i;
          break;
        }
      }

      strcat(asm_out, "  sw t");
      strcat(asm_out, to_string(loadreg).c_str());
      strcat(asm_out, ", (t");
      strcat(asm_out, to_string(destreg).c_str());
      strcat(asm_out, ")\n");
      dpt += 4;
      
      used[reg] = 0;
      used[loadreg] = 0;
      used[destreg] = 0;
    }
}

void Visit(const koopa_raw_load_t &load){
    cout<<"in load"<<endl;
    cout<< load.src<<endl;
    int reg1 = 0;
        for(int i = 0 ; i < 7 ; i++){ 
            //find aviliable reg
            if(used[i] == 0){
                used[i] = 1;
                reg1 = i;break;
            }
        }
    int reg2 = 0;
        for(int i = 0 ; i < 7 ; i++){ 
            //find aviliable reg
            if(used[i] == 0){
                used[i] = 1;
                reg2 = i;break;
            }
        }
    int reg3 = 0;
        for(int i = 0 ; i < 7 ; i++){ 
            //find aviliable reg
            if(used[i] == 0){
                used[i] = 1;
                reg3 = i;break;
            }
        }
    if(load.src->name){
      string sname = load.src->name;
      cout<<"load name:"<<sname<<endl;

      if(alloc_pos[sname] >= 0){
        if(alloc_pos[sname] > 1024){
          cout<<"loading saved pointer"<<endl;
          if(alloc_size.find(sname) != alloc_size.end() && alloc_size[sname][0] == -1){
            cout<<"loading saved pointer, in it, array has size : "<<alloc_size[sname].size()<<endl;
            if(alloc_size[sname][0] == -1){
              tmp_array_size = *new vector<int>();
              for(int i = 0 ; i < alloc_size[sname].size(); i++){
                tmp_array_size.push_back(alloc_size[sname][i]);
              }
              cout<<"why"<<endl;
            }
          }
          int spreg1 = 0;
          for(int i = 0 ; i < 7 ; i ++){
            if(!used[i]){spreg1 = i;break;}
          }
          strcat(asm_out, "  li t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(alloc_pos[sname]).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ", sp, t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  lw t");
          strcat(asm_out, to_string(reg1).c_str());
          strcat(asm_out, ", (t");
          strcat(asm_out, to_string(spreg1).c_str());
          strcat(asm_out, ")\n");
          dpt += 12;
        }
        else{
          if(alloc_size.find(sname) != alloc_size.end() && !(alloc_size[sname].size() == 0)){
            cout<<"loading saved pointer, in it, array has size : "<<alloc_size[sname].size()<<endl;
            if(alloc_size[sname][0] == -1){
              tmp_array_size = *new vector<int>();
              for(int i = 0 ; i < alloc_size[sname].size(); i++){
                tmp_array_size.push_back(alloc_size[sname][i]);
              }
              cout<<"why"<<endl;
            }
          }
          strcat(asm_out, "  lw t");
          strcat(asm_out, to_string(reg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(alloc_pos[sname]).c_str());
          strcat(asm_out, "(sp)\n");
          dpt += 4;
        }
      }
      else{
        strcat(asm_out, "  la t");
        strcat(asm_out, to_string(reg2).c_str());
        strcat(asm_out, ", var\n");
         dpt += 4;

        if(-4-alloc_pos[sname] < 1024){
          strcat(asm_out, "  lw t");
          strcat(asm_out, to_string(reg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(-4-alloc_pos[sname]).c_str());
          strcat(asm_out, "(t");
          strcat(asm_out, to_string(reg2).c_str());
          strcat(asm_out, ")\n");
          dpt += 4;  
        }
        else{
          strcat(asm_out, "  li t");
          strcat(asm_out, to_string(reg3).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, to_string(-4-alloc_pos[sname]).c_str());
          strcat(asm_out, "\n");

          strcat(asm_out, "  add t");
          strcat(asm_out, to_string(reg2).c_str());
          strcat(asm_out, ", t");
          strcat(asm_out, to_string(reg2).c_str());
          strcat(asm_out, ", t");
          strcat(asm_out, to_string(reg3).c_str());
          strcat(asm_out, "\n");
          
          strcat(asm_out, "  lw t");
          strcat(asm_out, to_string(reg1).c_str());
          strcat(asm_out, ", ");
          strcat(asm_out, "(t");
          strcat(asm_out, to_string(reg2).c_str());
          strcat(asm_out, ")\n");
          dpt += 12;
        }
      }
      used[reg2] = 0;
      calstack.push(reg1);
    }
    else{
      Visit(load.src);
      int ttype = calstack.top();
      calstack.pop();

      strcat(asm_out, "  lw t");
      strcat(asm_out, to_string(reg1).c_str());
      strcat(asm_out, ", (t");
      strcat(asm_out, to_string(ttype).c_str());
      strcat(asm_out, ")\n");
      dpt += 4;

      used[ttype] = 0;
      used[reg2] = 0;
      calstack.push(reg1);
    }
    used[reg3] = 0;
    
}