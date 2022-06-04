#ifndef MYAST_H
#define MYAST_H

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <vector>
#include <stack>
#include <list>
#include <map>
#include <iterator>
#include <algorithm>
using namespace std;

static stack<int> cur_num;
static stack<int> imm_stack;
static stack<int> if_stack;
static map<std::string, std::string> const_symtbl;
static map<std::string, std::string> var_symtbl;
static vector<string> func_var;
static vector<string> func_var_array;
static vector<string> func_var_array_type;
static vector<vector<string> > param_list;
static vector<map<std::string, std::string> > var_symtbl_b;
static vector<map<std::string, std::string> > const_symtbl_b;
static vector<map<std::string, std::string> > saved_var_symtbl_b;
static vector<map<std::string, std::string> > saved_const_symtbl_b;
static vector<map<std::string, vector<int> > > array_index;
static map<std::string, vector<int> > tmp_array_index;
static map<std::string, std::string> func_tbl;
static map<std::string, int> quick_ret;
static std::string cur_func;
static std::string cur_array;
static int rparam;
static int bool_res_cnt;
static int cur_while_cnt;
static int array_dims;
static int align_depth;
static int cur_total_num;
static int need_to_fill;
static int exp_depth;
static int block_depth;
static int func_depth;
static int max_block_depth;
static bool optrue = 0;
static vector<int> terminated;
static int whilecnt = 0;
static vector<int> cur_while;
static int ifcnt = 0;
static int elsecnt = 0;
static string tmptype;

class BaseAST
{
public:
  static std::string koopa_string; 
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
};

class CompUnitAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> compunit;

  void Dump() const override
  {
    cout<<"in compunit"<<endl;
    koopa_string +=  "decl @getint(): i32\ndecl @getch(): i32\ndecl @getarray(*i32): i32\ndecl @putint(i32)\ndecl @putch(i32)\ndecl @putarray(i32, *i32)\ndecl @starttime()\ndecl @stoptime()\n";
    func_tbl["getint"] = "int";
    func_tbl["getch"] = "int";
    func_tbl["getarray"] = "int";
    func_tbl["putint"] = "void";
    func_tbl["putch"] = "void";
    func_tbl["putarray"] = "void";
    func_tbl["starttime"] = "void";
    func_tbl["stoptime"] = "void";
    quick_ret["getint"] = -114514;
    quick_ret["getch"] = -114514;
    quick_ret["getarray"] = -114514;
    quick_ret["putint"] = -114514;
    quick_ret["putch"] = -114514;
    quick_ret["putarray"] = -114514;
    quick_ret["starttime"] = -114514;
    quick_ret["stoptime"] = -114514;
    // global vars table
    cur_func = "global";
    var_symtbl_b.push_back(*(new map<std::string, std::string>()));
    const_symtbl_b.push_back(*(new map<std::string, std::string>()));
    saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
    saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
    array_index.push_back(*(new map<std::string, vector<int> >()));
    compunit -> Dump();
    
  }
};

class CompUnitBAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> compunit;
  std::unique_ptr<BaseAST> func_def;
  std::unique_ptr<BaseAST> decl;
  int type;

  void Dump() const override
  {
    cout<<"in compunitb"<<endl;
    if(type == 1){
      compunit -> Dump();
      func_def->Dump();
    }
    if(type == 2){
      compunit -> Dump();
      decl->Dump();
    }
    if(type == 3){
      func_def->Dump();
    }
    if(type == 4){
      decl->Dump();
    }

  }
};

class FuncDefAST : public BaseAST
{
public:
  int type;
  std::string ident;
  std::unique_ptr<BaseAST> func_type;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> fparams;

  void Dump() const override
  {

    terminated.push_back(0);
    cout<<"in func def : "<<array_index.size()<<endl;
    saved_var_symtbl_b.clear();
    saved_const_symtbl_b.clear();
    saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
    saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
    cout<<"in funcdef"<<endl;
    cur_func = ident;
    func_var = *new vector<string>();
    if(type == 1){
      koopa_string += "fun @" + ident + "()";
      func_tbl[ident] = "int"; 
      koopa_string += ": i32 {\n%entry:\n";
      cout<<2<<endl;
      block->Dump();
      if(!terminated.back()){
        koopa_string += "  ret 0\n";
      }
      koopa_string += "}\n";
    }
    if(type == 2){
      koopa_string += "fun @" + ident + "(";
      fparams -> Dump();
      koopa_string += ")";
      func_tbl[ident] = "int";
      koopa_string += ": i32 {\n%entry:\n";
      cout<<2<<endl;
      block->Dump();
      if(!terminated.back()){
        koopa_string += "  ret 0\n";
      }
      koopa_string += "}\n";
    }
    if(type == 3){
      koopa_string += "fun @" + ident + "()";
      func_tbl[ident] = "void"; 
      koopa_string += "{\n%entry:\n";
      cout<<2<<endl;
      block->Dump();
      if(!terminated.back()){
        quick_ret[cur_func] = -114514;
        koopa_string += "  ret\n";
      }
      koopa_string += "}\n";
    }
    if(type == 4){
      koopa_string += "fun @" + ident + "(";
      fparams -> Dump();
      koopa_string += ")";
      func_tbl[ident] = "void";
      koopa_string += "{\n%entry:\n";
      cout<<2<<endl;
      block->Dump();
      if(!terminated.back()){
        quick_ret[cur_func] = -114514;
        koopa_string += "  ret\n";
      }
      koopa_string += "}\n";
    }
    terminated.pop_back();
  }
};

class FuncFParamsAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> fparams;
  std::unique_ptr<BaseAST> constexpa;
  int type;

  void Dump() const override
  {
    cout<<"in fparam"<<endl;
    if(type == 1){
      koopa_string += "@" + ident;
      koopa_string += ": i32";
      koopa_string += ",";
      func_var.push_back(ident);
      fparams->Dump();
    }
    if(type == 2){
      koopa_string += "@" + ident;
      koopa_string += ": i32";
      func_var.push_back(ident);
    }
    if(type == 3 || type == 4){
      string lval = ident;
      int save_array_dims = array_dims;
      array_dims = 0;
      constexpa -> Dump();
      string ref = "";
      tmp_array_index[lval] = *new vector<int>();
      while(!imm_stack.empty() && array_dims--){
        std::cout << "in func param 3 safe1 : " << imm_stack.size()<<endl;
        tmp_array_index[lval].push_back(imm_stack.top());
        if(tmp_array_index[lval].size() == 1){
          ref = "[i32, " + to_string(imm_stack.top()) + "]";
        }
        else{
          ref = "[" + ref + ", " + to_string(imm_stack.top()) + "]";
        }
        imm_stack.pop();
        cur_num.pop();
      }
      // this means the first dimision of this array is not defined
      tmp_array_index[lval].push_back(-1);
      ref = "*" + ref;
      koopa_string += "@" + ident + ": " + ref;
      func_var_array.push_back(ident);
      func_var_array_type.push_back(ref);
      if(type == 3){
        koopa_string += ",";
        fparams->Dump();
      }
      array_dims = save_array_dims;
    }
    if(type == 5 || type == 6){
      string lval = ident;
      string ref = "";
      tmp_array_index[lval] = *new vector<int>();
      // this means the first dimision of this array is not defined
      tmp_array_index[lval].push_back(-1);
      ref = "*i32";
      koopa_string += "@" + ident + ": " + ref ;
      func_var_array.push_back(ident);
      func_var_array_type.push_back(ref);
      if(type == 5){
        koopa_string += ",";
        fparams->Dump();
      }
    }
  }
};

class FuncTypeAST : public BaseAST
{
public:
  std::string functype;
  void Dump() const override
  {
    //TODO:depth isn't correct
    std::cout << "in functype1" << endl;
    tmptype = functype;
    if(functype == "int")
      koopa_string += ": i32 {\n%entry:\n";
    else
      koopa_string += "{\n%entry:\n";
    std::cout << functype << endl;
  }
};

class BlockAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> blockitem;
  int type;
  void Dump() const override
  {
    cout<<"in block, check terminated:"<<endl;
    for(int i = 0 ; i < terminated.size(); i ++){
      cout<<"  i: "<<terminated[i]<<endl;
    }
    if(type == 1){
      var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      const_symtbl_b.push_back(*(new map<std::string, std::string>()));
      saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
      array_index.push_back(*(new map<std::string, vector<int> >()));
      block_depth += 1;
      if(block_depth > max_block_depth){
        saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
        saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
        max_block_depth = block_depth;
      }
      while(!func_var.empty()){
        cout<<"parse"<<endl;
        string tmp = func_var.front();
        func_var.erase(func_var.begin());
        koopa_string += "  @" + tmp + "_" + to_string(block_depth) + " = alloc i32\n  store @" + tmp + ", @" + tmp + "_" + to_string(block_depth) + "\n";
        var_symtbl_b[block_depth][tmp] = "%"+tmp;
      }
      while(!func_var_array.empty()){
        cout<<"parse array"<<endl;
        string tmp = func_var_array.front();
        array_index[block_depth][tmp] = *new vector<int>();
        func_var_array.erase(func_var_array.begin());
        string tmp2 = func_var_array_type.front();
        func_var_array_type.erase(func_var_array_type.begin());
        koopa_string += "  @" + tmp + "_" + to_string(block_depth) + " = alloc " + tmp2 + "\n  store @" + tmp + ", @" + tmp + "_" + to_string(block_depth) + "\n";
        for(int i = 0 ; i < tmp_array_index[tmp].size(); i++){
          array_index[block_depth][tmp].push_back(tmp_array_index[tmp][i]);
        }
      }
      std::cout << "in block" << endl;
      blockitem->Dump();
      var_symtbl_b.pop_back();
      const_symtbl_b.pop_back();
      array_index.pop_back();
      block_depth -= 1;
    }
    else{
      var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      const_symtbl_b.push_back(*(new map<std::string, std::string>()));
      saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
      array_index.push_back(*(new map<std::string, vector<int> >()));
      block_depth += 1;
      if(block_depth > max_block_depth){
        saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
        saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
        max_block_depth = block_depth;
      }
      while(!func_var.empty()){
        cout<<"parse"<<endl;
        string tmp = func_var.front();
        func_var.erase(func_var.begin());
        koopa_string += "  @" + tmp + "_" + to_string(block_depth) + " = alloc i32\n  store @" + tmp + ", @" + tmp + "_" + to_string(block_depth) + "\n";
        var_symtbl_b[block_depth][tmp] = "%"+tmp;
      }
      while(!func_var_array.empty()){
        cout<<"parse array"<<endl;
        string tmp = func_var_array.front();
        array_index[block_depth][tmp] = *new vector<int>();
        func_var_array.erase(func_var_array.begin());
        string tmp2 = func_var_array_type.front();
        func_var_array_type.erase(func_var_array_type.begin());
        koopa_string += "  @" + tmp + "_" + to_string(block_depth) + " = alloc " + tmp2 + "\n  store @" + tmp + ", @" + tmp + "_" + to_string(block_depth) + "\n";
        for(int i = 0 ; i < tmp_array_index[tmp].size(); i++){
          array_index[block_depth][tmp].push_back(tmp_array_index[tmp][i]);
        }
      }
      std::cout << "in block" << endl;
      var_symtbl_b.pop_back();
      const_symtbl_b.pop_back();
      array_index.pop_back();
      block_depth -= 1;
      std::cout << "in block" << endl;
    }
    cout<<"out of block, check terminated:"<<endl;
    for(int i = 0 ; i < terminated.size(); i ++){
      cout<<"  i: "<<terminated[i]<<endl;
    }
  }
};
class StmtAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> expa;
  std::string lval;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> ifstmt;
  std::unique_ptr<BaseAST> elsestmt;
  int type;
  void Dump() const override
  {
    std::cout << "in stmt" << endl;
    if (terminated.back())
    {
      return;
    }
    if ((type == 1 || type == 3) && !terminated.back())
    {
      cout<<"heading into stmt ret"<<endl;
      terminated.back() = 1;
      if(type == 1)
        exp->Dump();
      koopa_string += "  ret ";
      if(type == 3){
        koopa_string += "\n";
        return ;
      }
      if (cur_num.empty())
        koopa_string += "0\n";
      else
      {
        int ttype = cur_num.top();
        cur_num.pop();
        if (ttype == -1)
        {
          std::cout << "no expr";
          quick_ret[cur_func] = imm_stack.top();
          koopa_string +=  to_string(imm_stack.top()) + "\n";
          imm_stack.pop();
        }
        else
        {
          quick_ret[cur_func] = -114514;
          std::cout << "expr";
          koopa_string +=  "%" + to_string(ttype) + "\n";
        }
      }
      cout<<"finish stmt ret"<<endl;
    }
    // fuzhiyuju
    else if (type == 2)
    {
      exp->Dump();
      int ttype = cur_num.top();
      cur_num.pop();
      string exp1;
      if (ttype == -1)
      {
        std::cout << "give no expr";

        exp1 += to_string(imm_stack.top());
        imm_stack.pop();
      }
      else
      {
        std::cout << "give expr";
        exp1 += "%" + to_string(ttype);
      }
      int depth;
      for (depth = block_depth; depth >= 0; depth--)
      {
        if (var_symtbl_b[depth].find(lval) != var_symtbl_b[depth].end())
        {
          break;
        }
      }
      var_symtbl_b[depth][lval] = exp1;
      koopa_string += "  store " + exp1 + " ,@" + lval + "_" + to_string(depth) + "\n";
    }
    else if (type == 4)
    {
      exp->Dump();
      int ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        imm_stack.pop();
      }
    }
    else if (type == 5)
    {
      // happy 
    }
    else if(type == 6)
    {
      block->Dump();
    }
    else if(type == 7){
      exp -> Dump();
      int ttype = cur_num.top();
      cur_num.pop();
      int thiscnt = ifcnt;
      ifcnt += 1;
      if(ttype == -1){
        koopa_string += "  br " + to_string(imm_stack.top()) + ", %then" + to_string(thiscnt) + ", %end" + to_string(thiscnt) + "\n";
        imm_stack.pop();
      }
      else{
        koopa_string += "  br %" + to_string(ttype) + ", %then" + to_string(thiscnt) + ", %end" + to_string(thiscnt) + "\n";
      }
      koopa_string += "%then" + to_string(thiscnt) + ":\n";
      terminated.push_back(0);
      ifstmt -> Dump();
      if(!terminated.back())
          koopa_string += "  jump %end" + to_string(thiscnt) + "\n";
      terminated.pop_back();
      koopa_string += "%end" + to_string(thiscnt) + ":\n";
    }
    else if(type == 8){
      if_stack.push(ifcnt);
      if_stack.push(elsecnt);
      if_stack.push(2);
      exp -> Dump();
      int ttype = cur_num.top();
      cur_num.pop();
      int thiscnt = ifcnt;
      ifcnt += 1;
      int thiselsecnt = elsecnt;
      elsecnt += 1;
      if(ttype == -1){
        koopa_string += "  br " + to_string(imm_stack.top()) + ", %then" + to_string(thiscnt) + ", %else" + to_string(thiselsecnt) + "\n";  
        imm_stack.pop();
      }
      else{
        koopa_string += "  br %" + to_string(ttype) + ", %then" + to_string(thiscnt) + ", %else" + to_string(thiselsecnt) + "\n";
      }
      koopa_string += "%then" + to_string(thiscnt) + ":\n";
      terminated.push_back(0);
      ifstmt -> Dump();
      if(!terminated.back())
          koopa_string += "  jump %end" + to_string(thiscnt) + "\n";
      terminated.pop_back();
      koopa_string += "%else" + to_string(thiselsecnt) + ":\n";
      terminated.push_back(0);
      elsestmt -> Dump();
      if(!terminated.back())
          koopa_string += "  jump %end" + to_string(thiscnt) + "\n";
      terminated.pop_back();
      koopa_string += "%end" + to_string(thiscnt) + ":\n";
    }
    else if(type == 9){
      int thiscnt = whilecnt;
      cur_while.push_back(thiscnt);
      whilecnt += 1;
      koopa_string += "  jump %while_entry" + to_string(thiscnt) + "\n";
      koopa_string += "%while_entry" + to_string(thiscnt) + ":\n";
      if_stack.push(whilecnt);
      if_stack.push(3);
      exp -> Dump();
      //refresh cur while loop
      int ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        koopa_string += "  br " + to_string(imm_stack.top()) + ", %while_body" + to_string(thiscnt) + ", %while_end" + to_string(thiscnt) + "\n";  
        imm_stack.pop();
      }
      else{
        koopa_string += "  br %" + to_string(ttype) + ", %while_body" + to_string(thiscnt) + ", %while_end" + to_string(thiscnt) + "\n";
      }
      koopa_string += "%while_body" + to_string(thiscnt) + ":\n";
      terminated.push_back(0);
      ifstmt -> Dump();
      if(!terminated.back())
        koopa_string += "  jump %while_entry" + to_string(thiscnt) +"\n";
      cout<<"t"<<terminated.back()<<endl;
      terminated.pop_back();
      koopa_string += "%while_end"  +to_string(thiscnt) + ":\n";
      cur_while.pop_back();
    }
    else if(type == 10){
      koopa_string += "  jump %while_end" + to_string(cur_while.back()) + "\n";
      terminated.back() = 1;
    }
    else if(type == 11){
      koopa_string += "  jump %while_entry" + to_string(cur_while.back()) + "\n";
      terminated.back() = 1;
    }
    else if(type == 12){
      int cur_depth = 0;
      std::cout << "in stmt 4 safe1 : " << block_depth << endl;
      for(int i = block_depth ; i >= 0 ; i --){
        if(!array_index[i][lval].empty()){
          cout<<"not empty :"<<i<<endl;
          cur_depth = i;
          break;
        }
      }
      int ref_cnt = array_index[cur_depth][lval].size();
      int save_array_dims = array_dims;
      array_dims = 0;
      expa->Dump();
      std::cout << "in stmt 4 safe1.0 : " << ref_cnt << endl;
      std::cout << "in stmt 4 safe1.1 : " << cur_depth << endl;
      string ref = "";
      stack<int> get_ref;
      while(!cur_num.empty() && ref_cnt --){
        std::cout << "in stmt  3 safe1 : 1 :" << imm_stack.size()<<endl;
        std::cout << "in stmt  3 safe1 : 2 :" << cur_num.size()<<endl;
        if(cur_num.top() == -1){
          get_ref.push(imm_stack.top());
          imm_stack.pop();
          cur_num.pop();
        }
        else{
          get_ref.push(-cur_num.top()-1);
          cur_num.pop();
        }
      }
      if(array_index[cur_depth][lval][array_index[cur_depth][lval].size()-1] == -1){
        cout<<"in stmt , going to usr get ptr,because array_index = " << array_index[cur_depth][lval][array_index[cur_depth][lval].size()-1] << endl;
        koopa_string += "  %" + to_string(exp_depth) + " = load @" + lval + "_" + to_string(cur_depth) + "\n";
        exp_depth += 1;
        if(get_ref.top() >=0){
          koopa_string += "  %" + to_string(exp_depth) + " = getptr %" + to_string(exp_depth -1) + ", " + to_string(get_ref.top()) + "\n";
        }
        else{
          koopa_string += "  %" + to_string(exp_depth) + " = getptr %" + to_string(exp_depth -1) + ", %" + to_string(-get_ref.top()-1) + "\n";
        }
      }
      else{
        if(get_ref.top() >=0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + lval + "_" + to_string(cur_depth) + ", " + to_string(get_ref.top()) + "\n";
        }
        else{
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + lval + "_" + to_string(cur_depth) + ", %" + to_string(-get_ref.top()-1) + "\n";
        }
      }
      get_ref.pop();
      exp_depth ++;
      while(!get_ref.empty()){
        if(get_ref.top() >=0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth - 1) + ", " + to_string(get_ref.top()) + "\n";
        }
        else{
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth - 1) + ", %" + to_string(-get_ref.top()-1) + "\n";
        }
        get_ref.pop();
        exp_depth ++;
      }
      int saved_exp_depth = exp_depth - 1;
      array_dims = save_array_dims;
      exp->Dump();
      int ttype= cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        koopa_string += "  store " + to_string(imm_stack.top()) + ", %" + to_string(saved_exp_depth ) + "\n";    
        imm_stack.pop();
      }
      else{
        koopa_string += "  store %" + to_string(ttype) + ", %" + to_string(saved_exp_depth ) + "\n";  
      }
    }
  }
};

class ExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> lorexp;
  void Dump() const override
  {
    std::cout << "in exp" << endl;
    lorexp->Dump();
  }
};

class LOrExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> landexp;
  std::unique_ptr<BaseAST> lorexp;
  void Dump() const override
  {
    std::cout << "in lorexp" << endl;
    if (type == 1)
    {
      landexp->Dump();
    }
    if (type != 1)
    {
      lorexp -> Dump();
      int ttype = cur_num.top();
      cur_num.pop();
      int thiscnt = ifcnt;
      ifcnt += 1;
      int thisboolcnt = bool_res_cnt;
      bool_res_cnt += 1;
      koopa_string += "  @bool_res_0_" + to_string(thisboolcnt) + " = alloc i32\n";
      koopa_string += "  store 1, @bool_res_0_" + to_string(thisboolcnt) +"\n";
      if(ttype == -1){
        koopa_string += "  %" + to_string(exp_depth) + " = eq 0, " + to_string(imm_stack.top()) + "\n";
        imm_stack.pop();
      }
      else{
        koopa_string += "  %" + to_string(exp_depth) + " = eq 0, %" + to_string(ttype) + "\n";
      }
      koopa_string += "  br %" + to_string(exp_depth) + ", %then" + to_string(thiscnt) + ", %end" + to_string(thiscnt) + "\n";
      exp_depth += 1;
      koopa_string += "%then" + to_string(thiscnt) + ":\n";
      landexp -> Dump();
      int ttype2 = cur_num.top();
      cur_num.pop();
      if(ttype2 == -1){
        koopa_string += "  %" + to_string(exp_depth) + " = ne 0, " + to_string(imm_stack.top()) + "\n";
        imm_stack.pop();
      }
      else{
        koopa_string += "  %" + to_string(exp_depth) + " = ne 0, %" + to_string(ttype2) + "\n";
      }
      koopa_string += "  store %" + to_string(exp_depth) + ", @bool_res_0_" + to_string(thisboolcnt) +"\n";
      exp_depth += 1;
      koopa_string += "  jump %end" + to_string(thiscnt) + "\n";
      koopa_string += "%end" + to_string(thiscnt) + ":\n";
      koopa_string += "  %" + to_string(exp_depth) + " = load @bool_res_0_" + to_string(thisboolcnt) +"\n";
      exp_depth += 1;
      cur_num.push(exp_depth - 1);
    }
  }
};

class LAndExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> eqexp;
  std::unique_ptr<BaseAST> landexp;
  void Dump() const override
  {
    std::cout << "in landexp" << endl;
    if (type == 1)
    {
      eqexp->Dump();
    }
    if (type != 1)
    {
      landexp -> Dump();
      int ttype = cur_num.top();
      cur_num.pop();
      int thiscnt = ifcnt;
      ifcnt += 1;
      int thisboolcnt = bool_res_cnt;
      bool_res_cnt += 1;
      koopa_string += "  @bool_res_0_" + to_string(thisboolcnt) + " = alloc i32\n";
      koopa_string += "  store 0, @bool_res_0_" + to_string(thisboolcnt) +"\n";
      if(ttype == -1){
        koopa_string += "  %" + to_string(exp_depth) + " = ne 0, " + to_string(imm_stack.top()) + "\n";
        imm_stack.pop();
      }
      else{
        koopa_string += "  %" + to_string(exp_depth) + " = ne 0, %" + to_string(ttype) + "\n";
      }
      koopa_string += "  br %" + to_string(exp_depth) + ", %then" + to_string(thiscnt) + ", %end" + to_string(thiscnt) + "\n";
      exp_depth += 1;
      koopa_string += "%then" + to_string(thiscnt) + ":\n";
      eqexp -> Dump();
      int ttype2 = cur_num.top();
      cur_num.pop();
      if(ttype2 == -1){
        koopa_string += "  %" + to_string(exp_depth) + " = ne 0, " + to_string(imm_stack.top()) + "\n";
        imm_stack.pop();
      }
      else{
        koopa_string += "  %" + to_string(exp_depth) + " = ne 0, %" + to_string(ttype2) + "\n";
      }
      koopa_string += "  store %" + to_string(exp_depth) + ", @bool_res_0_" + to_string(thisboolcnt) +"\n";
      exp_depth += 1;
      koopa_string += "  jump %end" + to_string(thiscnt) + "\n";
      koopa_string += "%end" + to_string(thiscnt) + ":\n";
      koopa_string += "  %" + to_string(exp_depth) + " = load @bool_res_0_" + to_string(thisboolcnt) +"\n";
      exp_depth += 1;
      cur_num.push(exp_depth - 1);
    }
  }
};

class EqExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> eqexp;
  std::unique_ptr<BaseAST> relexp;
  void Dump() const override
  {
    std::cout << "in eqexp" << endl;
    if (type == 1)
    {
      relexp->Dump();
    }
    if (type != 1)
    {
      relexp->Dump();
      eqexp->Dump();
      int ttype;
      string exp1;
      string exp2;
      ttype = cur_num.top();
      cur_num.pop();
      if (ttype == -1)
      {
        exp1 = to_string(imm_stack.top());
        imm_stack.pop();
      }
      else
      {
        exp1 = "%" + to_string(ttype);
      }
      ttype = cur_num.top();
      cur_num.pop();
      if (ttype == -1)
      {
        exp2 = to_string(imm_stack.top());
        imm_stack.pop();
      }
      else
      {
        exp2 = "%" + to_string(ttype);
      }
      string myexp;
      myexp = "%" + to_string(exp_depth);
      exp_depth += 1;
      if (type == 2)
      {
        koopa_string += "  " + myexp + " = eq " + exp1 + ", " + exp2 + "\n";
      }
      if (type == 3)
      {
        koopa_string += "  " + myexp + " = ne " + exp1 + ", " + exp2 + "\n";
      }
      cur_num.push(exp_depth - 1);
    }
  }
};

class RelExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> addexp;
  std::unique_ptr<BaseAST> relexp;
  void Dump() const override
  {
    std::cout << "in rel exp" << endl;
    if (type == 1)
    {
      addexp->Dump();
    }
    if (type != 1)
    {
      addexp->Dump();
      relexp->Dump();
      int ttype;
      string exp1;
      string exp2;
      ttype = cur_num.top();
      cur_num.pop();
      if (ttype == -1)
      {
        exp1 = to_string(imm_stack.top());
        imm_stack.pop();
      }
      else
      {
        exp1 = "%" + to_string(ttype);
      }
      ttype = cur_num.top();
      cur_num.pop();
      if (ttype == -1)
      {
        exp2 = to_string(imm_stack.top());
        imm_stack.pop();
      }
      else
      {
        exp2 = "%" + to_string(ttype);
      }
      string myexp;
      myexp = "%" + to_string(exp_depth);
      exp_depth += 1;
      if (type == 2)
      {
        koopa_string += "  " + myexp + " = lt " + exp1 + ", " + exp2 + "\n";
      }
      if (type == 3)
      {
        koopa_string += "  " + myexp + " = gt " + exp1 + ", " + exp2 + "\n";
      }
      if (type == 4)
      {
        koopa_string += "  " + myexp + " = le " + exp1 + ", " + exp2 + "\n";
      }
      if (type == 5)
      {
        koopa_string += "  " + myexp + " = ge " + exp1 + ", " + exp2 + "\n";
      }
      cur_num.push(exp_depth - 1);
    }
  }
};

class AddExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> addexp;
  std::unique_ptr<BaseAST> mulexp;
  void Dump() const override
  {
    std::cout << "in add" << endl;
    if (type == 1)
    {
      mulexp->Dump();
    }
    if (type != 1)
    {
      std::cout << "add going to parse" << endl;
      addexp->Dump();
      std::cout << "add parsed add" << endl;
      mulexp->Dump();
      std::cout << "add parsed mul" << endl;
      int ttype1;
      int ttype2;
      string exp1;
      string exp2;
      ttype2 = cur_num.top();
      cur_num.pop();
      ttype1 = cur_num.top();
      cur_num.pop();
      // all numbers, calculate
      if (ttype1 == -1 && ttype2 == -1){
        int result;
        int var2 = imm_stack.top();
        imm_stack.pop();
        int var1 = imm_stack.top();
        imm_stack.pop();
        if(type == 2){
          result = var1 + var2;
        }
        if(type == 3){
          result = var1 - var2;
        }
        cur_num.push(-1);
        imm_stack.push(result);
      }
      else{
        if (ttype1 == -1)
        {
          exp1 = to_string(imm_stack.top());
          imm_stack.pop();
        }
        else
        {
          exp1 = "%" + to_string(ttype1);
        }
        if (ttype2 == -1)
        {
          exp2 = to_string(imm_stack.top());
          imm_stack.pop();
        }
        else
        {
          exp2 = "%" + to_string(ttype2);
        }
        string myexp;
        myexp = "%" + to_string(exp_depth);
        exp_depth += 1;
        if (type == 2)
        {
          koopa_string += "  " + myexp + " = add " + exp1 + ", " + exp2 + "\n";
        }
        if (type == 3)
        {
          koopa_string += "  " + myexp + " = sub " + exp1 + ", " + exp2 + "\n";
        }
        cur_num.push(exp_depth - 1);
        std::cout << "finish add without problem" << endl;
      }
    }
  }
};

class MulExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> mulexp;
  std::unique_ptr<BaseAST> unaryexp;
  void Dump() const override
  {
    std::cout << "in mul" << endl;
    if (type == 1)
    {
      unaryexp->Dump();
    }
    if (type != 1)
    {
      unaryexp->Dump();
      mulexp->Dump();
      int ttype1;
      int ttype2;
      string exp1;
      string exp2;
      ttype1 = cur_num.top();
      cur_num.pop();
      ttype2 = cur_num.top();
      cur_num.pop();
      if (ttype1 == -1 && ttype2 == -1){
        int result;
        int var1 = imm_stack.top();
        imm_stack.pop();
        int var2 = imm_stack.top();
        imm_stack.pop();
        if(type == 2){
          result = var1 * var2;
        }
        if(type == 3){
          result = var1 / var2;
        }
        if(type == 4){
          result = var1 % var2;
        }
        cur_num.push(-1);
        imm_stack.push(result);
      }
      else{
        if (ttype1 == -1)
        {
          exp1 = to_string(imm_stack.top());
          imm_stack.pop();
        }
        else
        {
          exp1 = "%" + to_string(ttype1);
        }
        if (ttype2 == -1)
        {
          exp2 = to_string(imm_stack.top());
          imm_stack.pop();
        }
        else
        {
          exp2 = "%" + to_string(ttype2);
        }
        string myexp;
        myexp = "%" + to_string(exp_depth);
        exp_depth += 1;
        if (type == 2)
        {
          koopa_string += "  " + myexp + " = mul " + exp1 + ", " + exp2 + "\n";
        }
        if (type == 3)
        {
          koopa_string += "  " + myexp + " = div " + exp1 + ", " + exp2 + "\n";
        }
        if (type == 4)
        {
          koopa_string += "  " + myexp + " = mod " + exp1 + ", " + exp2 + "\n";
        }
        cur_num.push(exp_depth - 1);
      }
    }
  }
};

class UnaryExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> primaryexp;
  std::unique_ptr<BaseAST> unaryexp;
  std::unique_ptr<BaseAST> rparams;
  std::string ident;
  void Dump() const override
  {
    std::cout << "in unary" << endl;
    if (type == 1)
    {
      std::cout << "in unary1" << endl;
      primaryexp->Dump();
    }
    if (type != 1 && type != 5 && type != 6)
    {
      std::cout << "in unary234" << endl;
      unaryexp->Dump();
      if (type != 2)
      {
        int ttype = cur_num.top();
        cur_num.pop();
        string exp1;
        if (ttype == -1)
        {
          int var = imm_stack.top();
          imm_stack.pop();
          cur_num.push(-1);
          if(type == 3){
            imm_stack.push(-var);
          }
          if(type == 4){
            imm_stack.push(!var);
          }
        }
        else
        {
          exp1 = "%" + to_string(ttype);
        
          string myexp;
          myexp = "%" + to_string(exp_depth);
          exp_depth += 1;
          if (type == 3)
          {
            koopa_string += "  " + myexp + " = sub 0, " + exp1 + "\n";
          }
          if (type == 4)
          {
            koopa_string += "  " + myexp + " = eq 0, " + exp1 + "\n";
          }
          cur_num.push(exp_depth - 1);
        }
      }
    }
    if(type == 5){
      std::cout << "in unary5" << endl;
      if(quick_ret[ident] != -114514){
        cur_num.push(-1);
        imm_stack.push(quick_ret[ident]);
        koopa_string += "  call @" + ident + "()\n";
      } 
      else{
        if(func_tbl[ident] == "int"){
          koopa_string += "  %" + to_string(exp_depth) + " = call @" + ident + "()\n";
          cur_num.push(exp_depth);
          exp_depth += 1;
        }
        else{
          // not defined
          koopa_string += "  call @" + ident + "()\n";
          cur_num.push(-1);
          imm_stack.push(1);
        }
      }
    }
    if(type == 6){
      std::cout << "in unary6" << endl;
      param_list.push_back(*new vector<string>());
      rparams -> Dump();
      if(quick_ret[ident] != -114514){
        cur_num.push(-1);
        imm_stack.push(quick_ret[ident]);
      } 
      if(func_tbl[ident] == "int"){
        koopa_string += "  %" + to_string(exp_depth) + " = call @" + ident + "(";
        cur_num.push(exp_depth);
        exp_depth += 1;
      }
      else{
        koopa_string += "  call @" + ident + "(";
        cur_num.push(-1);
        imm_stack.push(1);
      }
      while(!param_list.back().empty()){
        cout<<"parsing param list"<<endl;
        if(param_list.back().size() == 1){
          koopa_string += param_list.back().front();
        }
        else{
          koopa_string += param_list.back().front() + ", ";
        }
        param_list.back().erase(param_list.back().begin());
        cout<<"parsing param list finish"<<endl;
      }
      koopa_string += ")\n";
      param_list.pop_back();
    
    }
  }
};

class FuncRParamsAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> rparams;
  int type;

  void Dump() const override
  {
    std::cout << "in rparam1" << endl;
    rparam = 1;
    exp->Dump();
    rparam = 0;
    std::cout << "in rparam2" << endl;
    int ttype = cur_num.top();
    std::cout << "in rparam3" << endl;
    cur_num.pop();
    if(ttype == -1){
      param_list.back().push_back(to_string(imm_stack.top()));
      imm_stack.pop();
    }
    else{
      param_list.back().push_back("%" + to_string(ttype));
    }
    if(type == 1){
      rparam = 1;
      rparams -> Dump();
      rparam = 0;
    }
  }
};

class PrimaryExpAST : public BaseAST
{
public:
  int type;
  std::string lval;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> number;
  std::unique_ptr<BaseAST> constexp;
  std::unique_ptr<BaseAST> constexpa;
  void Dump() const override
  {
    std::cout << "in primaryexp" << endl;
    if (type == 3)
    {
      std::cout << "in primaryexp and going to parse lval" << endl;
      map<string, string>::iterator it1;
      map<string, string>::iterator it2;
      int ttype = 0;
      int depth1;
      int depth2;
      int depth3;
      std::cout << "parsing 1" << endl;
      for (depth1 = block_depth; depth1 >= 0; depth1--)
      {
        if (const_symtbl_b[depth1].find(lval) != const_symtbl_b[depth1].end())
        {
          it1 = const_symtbl_b[depth1].find(lval);
          ttype = 1;
          break;
        }
      }
      std::cout << "parsing 2" << endl;
      for (depth2 = block_depth; depth2 > depth1; depth2--)
      {
        if (var_symtbl_b[depth2].find(lval) != var_symtbl_b[depth2].end())
        {
          it2 = var_symtbl_b[depth2].find(lval);
          ttype = 2;
          break;
        }
      }
      std::cout << "parsing 3" << endl;
      for (depth3 = block_depth; depth3 > depth2; depth3--)
      {
        if (array_index[depth3].find(lval) != array_index[depth3].end() && !(array_index[depth3][lval].size() == 0))
        {
          cout<<"this ref has depth : "<< array_index[depth3][lval].size()<<endl;
          ttype = 3;
          break;
        }
      }
      // variable need to be load
      if (ttype == 3)
      {
        std::cout << "loading 3" << endl;
        std::cout << "no target in const symbol & var symbol : " << depth3 << endl;
        string myexp;
        myexp = "%" + to_string(exp_depth);
        if(array_index[depth3][lval][array_index[depth3][lval].size()-1] == -1){
          koopa_string += "  " + myexp + " = load @" + lval + "_" + to_string(depth3) + "\n";
          string nmyexp = "%" + to_string(exp_depth+1);
          cout<<"in primary exp load , going to usr get ptr,because array_index = " << array_index[depth3][lval][array_index[depth3][lval].size()-1] << endl;

          koopa_string += "  " + nmyexp + " = getptr " + myexp + ", 0\n";
          exp_depth++;
        }
        else{
          koopa_string += "  " + myexp + " = getelemptr @" + lval + "_" + to_string(depth3) + ", 0\n";
        }
        cur_num.push(exp_depth);
        exp_depth++;
      }
      if (ttype == 2)
      {
        std::cout << "loading 2" << endl;
        std::cout << "no target in const symbol" << endl;
        string myexp;
        myexp = "%" + to_string(exp_depth);
        koopa_string += "  " + myexp + " = load @" + lval + "_" + to_string(depth2) + "\n";
        cur_num.push(exp_depth);
        exp_depth++;
      }
      if(ttype == 1)
      {
        std::cout << "loading 1" << endl; 
        std::string target = it1->second;
        if (target[0] == '%')
        {
          std::cout << "after this, target become:" << target << endl;
          target.erase(target.begin());
          int tmp = stoi(target);
          cur_num.push(tmp);
        }
        else
        {
          std::cout << "target become:" << target << endl;
          cur_num.push(-1);
          imm_stack.push(stoi(target));
        }
      }
      std::cout << "finish loading 1" << endl;
    }
    else if (type == 2)
      number->Dump();
    else if(type == 1)
      exp->Dump();
    else if(type == 4){
      std::cout << " in primary exp type 4" << endl;
      int cur_depth = 0;
      for(int i = block_depth ; i >= 0 ; i --){
        if(!array_index[i][lval].empty()){
          cout<<"not empty :"<<i<<endl;
          cur_depth = i;
          break;
        }
      }
      int ref_cnt = array_index[cur_depth][lval].size();
      int save_array_dims = array_dims;
      array_dims = 0;
      int ttl_dims = 0;
      constexpa->Dump();
      std::cout << " in primary exp type 4 finish parsing constexp" << endl;
      ttl_dims = array_dims;
      std::cout << "in primary 4 safe1 : " << ref_cnt << ", " << array_dims <<  endl;
      string ref = "";
      stack<int> get_ref;
      std::cout << "out primary 3 safe1 : 1 :" << imm_stack.size()<<endl;
      std::cout << "out primary 3 safe1 : 2 :" << cur_num.size()<<endl;
      while(!cur_num.empty() && ref_cnt -- && array_dims--){
        std::cout << "in primary 3 safe1 : 1 :" << imm_stack.size()<<endl;
        std::cout << "in primary 3 safe1 : 2 :" << cur_num.size()<<endl;
        if(cur_num.top() == -1){
          get_ref.push(imm_stack.top());
          imm_stack.pop();
          cur_num.pop();
        }
        else{
          get_ref.push(-cur_num.top()-1);
          cur_num.pop();
        }
      }
      std::cout << "in primary 4 safe1 out of while " << ref_cnt << endl;
      if(array_index[cur_depth][lval][array_index[cur_depth][lval].size()-1] == -1){
        koopa_string += "  %" + to_string(exp_depth) + " = load @" + lval + "_" + to_string(cur_depth) + "\n";
        exp_depth += 1;
        if(get_ref.top() >=0){
          cout<<"in primary exp ref , going to usr get ptr,because array_index = " << array_index[cur_depth][lval][array_index[cur_depth][lval].size()-1] << endl;
          koopa_string += "  %" + to_string(exp_depth) + " = getptr %" + to_string(exp_depth -1) + ", " + to_string(get_ref.top()) + "\n";
        }
        else{
          koopa_string += "  %" + to_string(exp_depth) + " = getptr %" + to_string(exp_depth -1) + ", %" + to_string(-get_ref.top()-1) + "\n";
        }
      }
      else{
        if(get_ref.top() >=0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + lval + "_" + to_string(cur_depth) + ", " + to_string(get_ref.top()) + "\n";
        }
        else{
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + lval + "_" + to_string(cur_depth) + ", %" + to_string(-get_ref.top()-1) + "\n";
        }
      }
      get_ref.pop();
      exp_depth ++;
      while(!get_ref.empty()){
        if(get_ref.top() >=0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth - 1) + ", " + to_string(get_ref.top()) + "\n";
        }
        else{
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth - 1) + ", %" + to_string(-get_ref.top()-1) + "\n";
        }
        get_ref.pop();
        exp_depth ++;
      }
      // is not parsing from function params. this means need to return a value.
      if(!rparam){
        koopa_string += "  %" + to_string(exp_depth) + " = load %" + to_string(exp_depth-1) + "\n"; 
        exp_depth ++;
      }
      else{
        if(ttl_dims == array_index[cur_depth][lval].size()){
          koopa_string += "  %" + to_string(exp_depth) + " = load %" + to_string(exp_depth-1) + "\n"; 
          exp_depth ++;
        }
        else{
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth - 1) + ", " + to_string(0) + "\n";
          exp_depth ++;
        }
      }
      array_dims = save_array_dims;
      cur_num.push(exp_depth-1);
    }
  }
};

class NumberAST : public BaseAST
{
public:
  int num;
  void Dump() const override
  {
    std::cout << "in number" << endl;
    cur_num.push(-1);
    std::cout << "in number1" << endl;
    imm_stack.push(num);
    std::cout << "in number2" << endl;
    std::cout << num;
    std::cout << "in number3" << endl;
  }
};

// newly defined

class BlockItemAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> stmt;
  std::unique_ptr<BaseAST> decl;
  std::unique_ptr<BaseAST> blockitem;
  int type;
  void Dump() const override
  {
    if (terminated.back())
    {
      return;
    }
    std::cout << "in blockitem" << endl;
    if (type == 1)
    {
      std::cout << "    type1" << endl;
      blockitem->Dump();
      if (terminated.back())
      {
        return;
      }
      stmt->Dump();
    }
    else if (type == 2)
    {
      std::cout << "    type2" << endl;
      blockitem->Dump();
      if (terminated.back())
      {
        return;
      }
      decl->Dump();
    }
    else if (type == 3)
    {
      std::cout << "    type3" << endl;
      decl->Dump();
    }
    else if (type == 4)
    {
      std::cout << "    type4" << endl;
      stmt->Dump();
    }
  }
};

class DeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constdecl;
  std::unique_ptr<BaseAST> vardecl;
  int type;
  void Dump() const override
  {
    std::cout << "in decl" << endl;
    if (type == 1)
    {
      constdecl->Dump();
    }
    else
    {
      vardecl->Dump();
    }
  }
};

class ConstDeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constdef;
  void Dump() const override
  {
    std::cout << "in const decl" << endl;
    constdef->Dump();
  }
};

class ConstDefAST : public BaseAST
{
public:
  std::string lval;
  std::unique_ptr<BaseAST> constdef;
  std::unique_ptr<BaseAST> constinitval;
  std::unique_ptr<BaseAST> constexpa;
  int type;
  void Dump() const override
  {
    int array_depth = -1;
    std::cout << "in const def" << endl;
    if (type == 2)
    {
      std::cout << "in const def 2" << endl;
      constdef->Dump();
    }
    if(type == 3 || type == 4){
      if(type == 4){
        constdef->Dump();
      }
      array_depth = 0;
      if(block_depth != 0){
        std::cout << "in def 3" << endl;
        cout<<"in decl of array"<<endl;
        int save_array_dims = array_dims;
        array_dims = 0;
        constexpa->Dump();
        std::cout << "in const def 3 safe1 : " << imm_stack.size()<< endl;
        string ref = "";
        array_index[block_depth][lval] = *new vector<int>();
        while(!imm_stack.empty() && array_dims--){
          std::cout << "in const def 3 safe1 : " << imm_stack.size()<<endl;
          array_index[block_depth][lval].push_back(imm_stack.top());
          if(array_index[block_depth][lval].size() == 1){
            ref = "[i32, " + to_string(imm_stack.top()) + "]";
          }
          else{
            ref = "[" + ref + ", " + to_string(imm_stack.top()) + "]";
          }
          imm_stack.pop();
          cur_num.pop();
        }
        koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc " + ref + "\n";  
        cur_array = lval;
        cur_total_num = 0;
        align_depth = -1;
        std::cout << "in def 3 going to parse initval" << endl;
        array_dims = save_array_dims;
        constinitval -> Dump();
        std::cout << "back" << endl;
      }
      else{
        std::cout << "in def 3" << endl;
        cout<<"in decl of array"<<endl;
        int save_array_dims = array_dims;
        array_dims = 0;
        constexpa->Dump();
        std::cout << "in const def 3 safe1 : " << imm_stack.size()<< endl;
        string ref = "";
        array_index[block_depth][lval] = *new vector<int>();
        while(!imm_stack.empty() && array_dims--){
          std::cout << "in const def 3 safe1 : " << imm_stack.size()<<endl;
          array_index[block_depth][lval].push_back(imm_stack.top());
          if(array_index[block_depth][lval].size() == 1){
            ref = "[i32, " + to_string(imm_stack.top()) + "]";
          }
          else{
            ref = "[" + ref + ", " + to_string(imm_stack.top()) + "]";
          }
          imm_stack.pop();
          cur_num.pop();
        }
        koopa_string += "global @" + lval + "_" + to_string(block_depth) + " = alloc " + ref + ",";  
        cur_array = lval;
        cur_total_num = 0;
        align_depth = -1;
        std::cout << "in def 3 going to parse initval" << endl;
        array_dims = save_array_dims;
        constinitval -> Dump();
        koopa_string += "\n";
        std::cout << "back" << endl;
      }
    }
    if(array_depth == -1){
      constinitval->Dump();
      int ttype = cur_num.top();
      cur_num.pop();
      if (ttype == -1)
      {
        int tmp = imm_stack.top();
        imm_stack.pop();
        const_symtbl_b[block_depth][lval] = to_string(tmp);
      }
      else
      {
        const_symtbl_b[block_depth][lval] = "%" + to_string(ttype);
      }
    }
  }
};

class ConstInitValAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constinitval;
  std::unique_ptr<BaseAST> constexp;
  std::unique_ptr<BaseAST> constexpb;
  int type;
  void Dump() const override
  {
    std::cout << "in const init val" << endl;
    if(type == 1)
      constexp->Dump();
    if(type == 2){
      std::cout << "in type {}" << endl;
      // this means having another {}, so the exp in this {} can only fill in a smaller array
      align_depth += 1;
      int tmp = cur_total_num;
      int ttldepth = array_index[block_depth][cur_array].size() - align_depth;
      int this_block_depth = 0;
      need_to_fill = 1 ;
      std::cout << "  this ttl depth : " << ttldepth << endl;
      // array_index[block_depth][cur_array] saves block width from 0 to depth
      // this_block_depth means in this {} need to fill depth from 0 to this_block_depth
      // this_block_depth = depth means 
      for(int i = 0 ; i < ttldepth ; i ++){
        cout<<array_index[block_depth][cur_array][i]<<endl;
      }
      while(tmp % array_index[block_depth][cur_array][this_block_depth] == 0){
        need_to_fill *= array_index[block_depth][cur_array][this_block_depth];
        tmp = tmp / array_index[block_depth][cur_array][this_block_depth];
        this_block_depth ++;
        if(this_block_depth >= ttldepth){
          cout<<"this block need to fill in full array"<<endl;
          break;
        } 
      }
      need_to_fill += cur_total_num;
      constexpb -> Dump();
    }
    if(type == 3){
      std::cout << "in type empty {}" << endl;
      align_depth ++;
      int tmp = cur_total_num;
      int ttldepth = array_index[block_depth][cur_array].size() - align_depth;
      int this_block_depth = 0;
      // array_index[block_depth][cur_array] saves block width from 0 to depth
      // this_block_depth means in this {} need to fill depth from 0 to ttldepth
      // this_block_depth = depth means 
      std::cout << "  this block depth : " << this_block_depth << endl;
      std::cout << "  this ttl depth : " << ttldepth << endl;
      cout << array_index[block_depth][cur_array][this_block_depth] << endl;
      while(tmp % array_index[block_depth][cur_array][this_block_depth] == 0){
        std::cout << "  array wod : " << endl;
        tmp = tmp / array_index[block_depth][cur_array][this_block_depth];
        this_block_depth ++;
        if(this_block_depth >= ttldepth){
          cout<<"this block need to fill in full array"<<endl;
          break;
        }
      }
      int total_num = 1;
      std::cout << "  this block depth : " << this_block_depth << endl;
      for(int i = 0 ; i < this_block_depth ; i ++){
        total_num *= array_index[block_depth][cur_array][i];
      }
      cout<<"this block need to fill in :"<< total_num << endl;
      need_to_fill = total_num;
      for(int i = 0 ; i < total_num ; i++){
        std::cout << "    step1 : "  << cur_total_num << endl;
        int tmp = cur_total_num;
        ttldepth = array_index[block_depth][cur_array].size();
        stack<int> ref_stack;
        vector<int> ref_vector;
        for(int i = 0 ; i < ttldepth ; i++){
          ref_stack.push(tmp%array_index[block_depth][cur_array][i]);
          ref_vector.push_back(tmp%array_index[block_depth][cur_array][i]);
          tmp /= array_index[block_depth][cur_array][i];
        }
        if(block_depth != 0){
          std::cout << "    step2 : " << ref_stack.top() << endl;
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + cur_array + "_" + to_string(block_depth) + ", " + to_string(ref_stack.top()) + "\n";
          ref_stack.pop();
          exp_depth ++;
          for(int i = 1 ; i < ttldepth ; i ++){
            int refi = ref_stack.top();
            ref_stack.pop();
            koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth-1) + ", " + to_string(refi) + "\n";
            exp_depth++;
          }
          std::cout << "    step3 : "  << endl;
          koopa_string += "  store " + to_string(0) + ", %" + to_string(exp_depth-1) + "\n"; 
          cur_total_num++;
        }
        else{
          std::cout << "    step2 - global: " << ref_stack.top() << endl;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == 0);
            }
            if(flg){
              koopa_string += "{";
            }
          }
          koopa_string += "0";
          int flg2 = 1;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == array_index[block_depth][cur_array][k] - 1);
            }
            flg2 = flg2 && flg;
            if(flg){
              koopa_string += "}";
            }
          }
          if(!flg2){
            koopa_string += ", ";
          }
          cur_total_num++;
          std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        }
      }
    }
  }
};

class ConstExpBAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constexpb;
  std::unique_ptr<BaseAST> constexp;
  std::unique_ptr<BaseAST> constinitval;
  int type;
  void Dump() const override
  {
    std::cout << "in const exp b" << endl;
    if(type == 1){
      int total_need_fill = need_to_fill;
      constinitval->Dump();
      need_to_fill = total_need_fill;
      std::cout << "  in const exp b type 1 total need to fill : " << need_to_fill << endl;
      // this is not another {}
      if(!imm_stack.empty()){
        int ttldepth = array_index[block_depth][cur_array].size();
        stack<int> ref_stack;
        vector<int> ref_vector;
        int tmp = cur_total_num;
        cur_total_num += 1;
        std::cout << "  in const exp b type 1 : " << tmp << endl;
        for(int i = 0 ; i < ttldepth ; i++){
          ref_stack.push(tmp%array_index[block_depth][cur_array][i]);
          ref_vector.push_back(tmp%array_index[block_depth][cur_array][i]);
          tmp /= array_index[block_depth][cur_array][i];
        }
        if(block_depth != 0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + cur_array + "_" + to_string(block_depth) + ", " + to_string(ref_stack.top()) + "\n";
          ref_stack.pop();
          exp_depth ++;
          for(int i = 1 ; i < ttldepth ; i ++){
            int refi = ref_stack.top();
            ref_stack.pop();
            koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth-1) + ", " + to_string(refi) + "\n";
            exp_depth++;
          }
          koopa_string += "  store " + to_string(imm_stack.top()) + ", %" + to_string(exp_depth-1) + "\n"; 
          imm_stack.pop();
          cur_num.pop();
        }
        else{
          std::cout << "    step2 - global: " << ref_stack.top() << endl;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == 0);
            }
            if(flg){
              koopa_string += "{";
            }
          }
          koopa_string += to_string(imm_stack.top());
          imm_stack.pop();
          cur_num.pop();
          int flg2 = 1;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == array_index[block_depth][cur_array][k] - 1);
            }
            flg2 = flg2 && flg;
            if(flg){
              koopa_string += "}";
            }
          }
          if(!flg2){
            koopa_string += ", ";
          }
          std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        }
      }
      total_need_fill = need_to_fill;
      constexpb->Dump();
      std::cout << "  in const exp b type 1 total need to fill : " << need_to_fill << endl;
      need_to_fill = total_need_fill;
    }
    if(type == 2){
      std::cout << "  in const exp b type 2 stage 1" << endl;
      int total_need_fill = need_to_fill;
      constinitval->Dump();
      need_to_fill = total_need_fill;
      std::cout << "  in exp b stage 2" << endl;
      // this is not another {}
      if(!imm_stack.empty()){
        int ttldepth = array_index[block_depth][cur_array].size();
        stack<int> ref_stack;
        vector<int> ref_vector;
        int tmp = cur_total_num;
        cur_total_num += 1;
        std::cout << "  in exp b type 1 : " << tmp << endl;
        for(int i = 0 ; i < ttldepth ; i++){
          ref_stack.push(tmp%array_index[block_depth][cur_array][i]);
          ref_vector.push_back(tmp%array_index[block_depth][cur_array][i]);
          tmp /= array_index[block_depth][cur_array][i];
        }
        if(block_depth != 0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + cur_array + "_" + to_string(block_depth) + ", " + to_string(ref_stack.top()) + "\n";
          ref_stack.pop();
          exp_depth ++;
          for(int i = 1 ; i < ttldepth ; i ++){
            int refi = ref_stack.top();
            ref_stack.pop();
            koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth-1) + ", " + to_string(refi) + "\n";
            exp_depth++;
          }
          koopa_string += "  store " + to_string(imm_stack.top()) + ", %" + to_string(exp_depth-1) + "\n"; 
          imm_stack.pop();
          cur_num.pop();
        }
        else{
          std::cout << "    step2 - global: " << ref_stack.top() << endl;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == 0);
            }
            if(flg){
              koopa_string += "{";
            }
          }
          koopa_string += to_string(imm_stack.top());
          imm_stack.pop();
          cur_num.pop();
          int flg2 = 1;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == array_index[block_depth][cur_array][k] - 1);
            }
            flg2 = flg2 && flg;
            if(flg){
              koopa_string += "}";
            }
          }
          if(!flg2){
            koopa_string += ", ";
          }
          std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        }
      }
      for(int tmp = cur_total_num ; tmp < need_to_fill ;){
        int ttldepth = array_index[block_depth][cur_array].size();
        std::cout << "  in const exp b stage 7" << endl;
        stack<int> ref_stack;
        vector<int> ref_vector;
        for(int i = 0 ; i < ttldepth ; i++){
          ref_stack.push(tmp%array_index[block_depth][cur_array][i]);
          ref_vector.push_back(tmp%array_index[block_depth][cur_array][i]);
          tmp /= array_index[block_depth][cur_array][i];
        }
        if(block_depth != 0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + cur_array + "_" + to_string(block_depth) + ", " + to_string(ref_stack.top()) + "\n";
          ref_stack.pop();
          exp_depth ++;
          for(int i = 1 ; i < ttldepth ; i ++){
            int refi = ref_stack.top();
            ref_stack.pop();
            koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth-1) + ", " + to_string(refi) + "\n";
            exp_depth++; 
          }
          koopa_string += "  store " + to_string(0) + ", %" + to_string(exp_depth-1) + "\n"; 
          cur_total_num++;
          tmp = cur_total_num;
        }
        else{
          std::cout << "    step2 - global: " << ref_stack.top() << endl;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == 0);
            }
            if(flg){
              koopa_string += "{";
            }
          }
          koopa_string += "0";
          int flg2 = 1;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == array_index[block_depth][cur_array][k] - 1);
            }
            flg2 = flg2 && flg;
            if(flg){
              koopa_string += "}";
            }
          }
          if(!flg2){
            koopa_string += ", ";
          }
          cur_total_num++;
          tmp = cur_total_num;
          std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        }
      }
    }
  }
};

class ConstExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constexp;
  std::unique_ptr<BaseAST> constexpb;
  int type;
  void Dump() const override
  {
    std::cout << "in const exp" << endl;
    constexp->Dump();
  }
};

class VarDeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> vardef;
  void Dump() const override
  {
    std::cout << "in var decl" << endl;
    vardef->Dump();
  }
};

class VarDefAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> vardef;
  std::string lval;
  std::unique_ptr<BaseAST> initval;
  std::unique_ptr<BaseAST> constexp;
  std::unique_ptr<BaseAST> constexpa;
  int type;
  void Dump() const override
  {
    std::cout << "in var def" << endl;
    if (type == 1)
    {
      std::cout << "in var def type 1" << endl;
      vardef->Dump();
      if(block_depth <= 1 || (saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end())){
        if(block_depth != 0)
          koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc i32\n";
        else
          koopa_string += "global @" + lval + "_" + to_string(block_depth) + " = alloc i32, zeroinit\n";
        var_symtbl_b[block_depth][lval] = "";
        saved_var_symtbl_b[block_depth][lval] = "";
      }
      else{
        var_symtbl_b[block_depth][lval] = "";
      }
    }
    if (type == 2)
    {
      std::cout << "in var def type 2" << endl;
      vardef->Dump();
      initval->Dump();
      if(block_depth == 1 || (block_depth != 0 && saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end())){
        koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc i32\n";
      }
      int ttype = cur_num.top();
      cur_num.pop();
      std::cout << "fine in type2" << endl;
      if (ttype == -1)
      {
        int tmp = imm_stack.top();
        imm_stack.pop();

        var_symtbl_b[block_depth][lval] = to_string(tmp);
        saved_var_symtbl_b[block_depth][lval] = to_string(tmp);
        if(block_depth != 0)
          koopa_string += "  store " + var_symtbl_b[block_depth][lval] + " ,@" + lval + "_" + to_string(block_depth) + "\n";
        else 
          koopa_string += "global @" + lval + "_" + to_string(block_depth) + " = alloc i32, " + to_string(tmp) + "\n";
      }
      else
      {
        var_symtbl_b[block_depth][lval] = "%" + to_string(ttype);
        saved_var_symtbl_b[block_depth][lval] = "%" + to_string(ttype);
        koopa_string += "  store " + var_symtbl_b[block_depth][lval] + " ,@" + lval + "_" + to_string(block_depth) + "\n";
      }
    }
    if (type == 3)
    {
      std::cout << "in var def type 3" << endl;
      if(block_depth <= 1 || (saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end())){
        if(block_depth != 0)
          koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc i32\n";
        else
          koopa_string += "global @" + lval + "_" + to_string(block_depth) + " = alloc i32, zeroinit\n";
        var_symtbl_b[block_depth][lval] = "";
        saved_var_symtbl_b[block_depth][lval] = "";
      }
      else{
        var_symtbl_b[block_depth][lval] = "";
        saved_var_symtbl_b[block_depth][lval] = "";
      }
    }
    if (type == 4)
    {
      std::cout << "in var def type 4" << endl;
      initval->Dump();
      if(block_depth == 1 || (block_depth != 0 && saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end())){
        std::cout << "in var def type 4 and is local" << endl;
        koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc i32\n";
      }
      int ttype = cur_num.top();
      cur_num.pop();
      std::cout << "fine in type4" << endl;
      if (ttype == -1)
      {
        int tmp = imm_stack.top();
        //std::cout << koopa_str << endl;
        imm_stack.pop();
        std::cout << "fine in type44" << endl;
        var_symtbl_b[block_depth][lval] = to_string(tmp);
        saved_var_symtbl_b[block_depth][lval] = to_string(tmp);
        std::cout << var_symtbl_b[block_depth][lval] << endl;
        std::cout << "fine in type442" << endl;
        if(block_depth != 0)
          koopa_string += "  store " + var_symtbl_b[block_depth][lval] + " ,@" + lval + "_" + to_string(block_depth) + "\n";
        else 
          koopa_string += "global @" + lval + "_" + to_string(block_depth) + " = alloc i32, " + to_string(tmp) + "\n";
      }
      else
      {
        var_symtbl_b[block_depth][lval] = "%" + to_string(ttype);
        saved_var_symtbl_b[block_depth][lval] = "%" + to_string(ttype);
        std::cout << "fine in type4" << endl;
        koopa_string += "  store " + var_symtbl_b[block_depth][lval] + " ,@" + lval + "_" + to_string(block_depth) + "\n";
      }
    }
    if(type == 5 || type == 7){
      if(type == 7){
        vardef->Dump();
      }
      if(block_depth != 0){
        std::cout << "in def 3" << endl;
        cout<<"in decl of array"<<endl;
        array_dims = 0;
        constexpa->Dump();
        std::cout << "in var def 3 safe1 : " << imm_stack.size()<< endl;
        string ref = "";
        array_index[block_depth][lval] = *new vector<int>();
        while(!imm_stack.empty() && array_dims--){
          std::cout << "in var def 3 safe1.5 : " << imm_stack.size()<<endl;
          array_index[block_depth][lval].push_back(imm_stack.top());
          if(array_index[block_depth][lval].size() == 1){
            ref = "[i32, " + to_string(imm_stack.top()) + "]";
          }
          else{
            ref = "[" + ref + ", " + to_string(imm_stack.top()) + "]";
          }
          imm_stack.pop();
          cur_num.pop();
        }
        koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc " + ref + "\n";  
        cur_array = lval;
        cur_total_num = 0;
        align_depth = -1;
        std::cout << "in def 3 going to parse initval" << endl;
        initval -> Dump();
        std::cout << "back" << endl;
      }
      else{
        std::cout << "in def 3" << endl;
        cout<<"in decl of array"<<endl;
        array_dims = 0;
        constexpa->Dump();
        std::cout << "in var def 3 safe1 : " << imm_stack.size()<< endl;
        string ref = "";
        array_index[block_depth][lval] = *new vector<int>();
        while(!imm_stack.empty() && array_dims--){
          std::cout << "in var def 3 safe1 : " << imm_stack.size()<<endl;
          array_index[block_depth][lval].push_back(imm_stack.top());
          if(array_index[block_depth][lval].size() == 1){
            ref = "[i32, " + to_string(imm_stack.top()) + "]";
          }
          else{
            ref = "[" + ref + ", " + to_string(imm_stack.top()) + "]";
          }
          imm_stack.pop();
          cur_num.pop();
        }
        koopa_string += "global @" + lval + "_" + to_string(block_depth) + " = alloc " + ref + ",";  
        cur_array = lval;
        cur_total_num = 0;
        align_depth = -1;
        std::cout << "in def 3 going to parse initval" << endl;
        initval -> Dump();
        koopa_string += "\n";
        std::cout << "back" << endl;
      }
    }
    if(type == 6|| type == 8){
      if(type == 8){
        vardef->Dump();
      }
      if(block_depth != 0){
        std::cout << "in def 4" << endl;
        cout<<"in decl of array"<<endl;
        array_dims = 0;
        constexpa->Dump();
        std::cout << "in var def 3 safe1 : " << imm_stack.size()<< endl;
        string ref = "";
        array_index[block_depth][lval] = *new vector<int>();
        while(!imm_stack.empty() && array_dims--){
          std::cout << "in var def 3 safe1 : " << imm_stack.size()<<endl;
          array_index[block_depth][lval].push_back(imm_stack.top());
          if(array_index[block_depth][lval].size() == 1){
            ref = "[i32, " + to_string(imm_stack.top()) + "]";
          }
          else{
            ref = "[" + ref + ", " + to_string(imm_stack.top()) + "]";
          }
          imm_stack.pop();
          cur_num.pop();
        }
        koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc " + ref + "\n";  
        cur_array = lval;
        cur_total_num = 0;
        align_depth = -1;
      }
      else{
        std::cout << "in global def 4" << endl;
        cout<<"in decl of array"<<endl;
        array_dims = 0;
        constexpa->Dump();
        std::cout << "in var def 3 safe1 : " << imm_stack.size()<< endl;
        string ref = "";
        array_index[block_depth][lval] = *new vector<int>();
        while(!imm_stack.empty() && array_dims--){
          std::cout << "in var def 3 safe1 : " << imm_stack.size()<<endl;
          array_index[block_depth][lval].push_back(imm_stack.top());
          if(array_index[block_depth][lval].size() == 1){
            ref = "[i32, " + to_string(imm_stack.top()) + "]";
          }
          else{
            ref = "[" + ref + ", " + to_string(imm_stack.top()) + "]";
          }
          imm_stack.pop();
          cur_num.pop();
        }
        cur_array = lval;
        cur_total_num = 0;
        align_depth = -1;
        koopa_string += "global @" + lval + "_" + to_string(block_depth) + " = alloc " + ref + ", zeroinit\n";
      }
    }
  }
};

class InitValAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> expb;
  std::unique_ptr<BaseAST> initval;
  int type;
  void Dump() const override
  {
    std::cout << "in init val" << endl;
    if(type == 1)
      exp->Dump();
    if(type == 2){
      std::cout << "in type {}" << endl;
      // this means having another {}, so the exp in this {} can only fill in a smaller array
      align_depth += 1;
      int tmp = cur_total_num;
      int ttldepth = array_index[block_depth][cur_array].size() - align_depth;
      int this_block_depth = 0;
      need_to_fill = 1 ;
      // array_index[block_depth][cur_array] saves block width from 0 to depth
      // this_block_depth means in this {} need to fill depth from 0 to this_block_depth
      // this_block_depth = depth means 
      for(int i = 0 ; i < ttldepth ; i ++){
        cout<<array_index[block_depth][cur_array][i]<<endl;
      }
      while(tmp % array_index[block_depth][cur_array][this_block_depth] == 0){
        need_to_fill *= array_index[block_depth][cur_array][this_block_depth];
        tmp = tmp / array_index[block_depth][cur_array][this_block_depth];
        this_block_depth ++;
        if(this_block_depth >= ttldepth){
          cout<<"this block need to fill in full array"<<endl;
          break;
        } 
      }
      need_to_fill += cur_total_num;
      expb -> Dump();
    }
    if(type == 3){
      std::cout << "in type empty {}" << endl;
      align_depth ++;
      int tmp = cur_total_num;
      int ttldepth = array_index[block_depth][cur_array].size() - align_depth;
      int this_block_depth = 0;
      // array_index[block_depth][cur_array] saves block width from 0 to depth
      // this_block_depth means in this {} need to fill depth from 0 to ttldepth
      // this_block_depth = depth means 
      std::cout << "  this block depth : " << this_block_depth << endl;
      cout << array_index[block_depth][cur_array][this_block_depth] << endl;
      while(tmp % array_index[block_depth][cur_array][this_block_depth] == 0){
        std::cout << "  array wod : " << endl;
        tmp = tmp / array_index[block_depth][cur_array][this_block_depth];
        this_block_depth ++;
        if(this_block_depth >= ttldepth){
          cout<<"this block need to fill in full array"<<endl;
          break;
        }
      }
      int total_num = 1;
      std::cout << "  this block depth : " << this_block_depth << endl;
      for(int i = 0 ; i < this_block_depth ; i ++){
        total_num *= array_index[block_depth][cur_array][i];
      }
      cout<<"this block need to fill in :"<< total_num << endl;
      need_to_fill = total_num;
      for(int i = 0 ; i < total_num ; i++){
        std::cout << "    step1 : "  << cur_total_num << endl;
        int tmp = cur_total_num;
        ttldepth = array_index[block_depth][cur_array].size();
        stack<int> ref_stack;
        vector<int> ref_vector;
        for(int i = 0 ; i < ttldepth ; i++){
          ref_stack.push(tmp%array_index[block_depth][cur_array][i]);
          ref_vector.push_back(tmp%array_index[block_depth][cur_array][i]);
          tmp /= array_index[block_depth][cur_array][i];
        }
        if(block_depth != 0){
          std::cout << "    step2 : " << ref_stack.top() << endl;
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + cur_array + "_" + to_string(block_depth) + ", " + to_string(ref_stack.top()) + "\n";
          ref_stack.pop();
          exp_depth ++;
          for(int i = 1 ; i < ttldepth ; i ++){
            int refi = ref_stack.top();
            ref_stack.pop();
            koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth-1) + ", " + to_string(refi) + "\n";
            exp_depth++;
          }
          std::cout << "    step3 : "  << endl;
          koopa_string += "  store " + to_string(0) + ", %" + to_string(exp_depth-1) + "\n"; 
          cur_total_num++;
        }
        else{
          std::cout << "    step2 - global: " << ref_stack.top() << endl;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == 0);
            }
            if(flg){
              koopa_string += "{";
            }
          }
          koopa_string += "0";
          int flg2 = 1;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == array_index[block_depth][cur_array][k] - 1);
            }
            flg2 = flg2 && flg;
            if(flg){
              koopa_string += "}";
            }
          }
          if(!flg2){
            koopa_string += ", ";
          }
          cur_total_num++;
          std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        }
      }
    }
  }
};

class ConstExpAAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constexpa;
  std::unique_ptr<BaseAST> constexp;
  int type;
  void Dump() const override
  {
    std::cout << "in const exp a" << endl;
    if(type == 1){
      std::cout << "in const exp a type1" << endl;
      array_dims ++;
      constexpa->Dump();
      constexp->Dump();
    }
    else{
      std::cout << "in const exp a type2" << endl;
      array_dims ++;
      constexp->Dump();
    }
    std::cout << "finish const exp a" << endl;
  }
};

class ExpAAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> expa;
  std::unique_ptr<BaseAST> exp;
  int type;
  void Dump() const override
  {
    std::cout << "in exp a" << endl;
    if(type == 1){
      array_dims ++;
      expa->Dump();
      exp->Dump();
    }
    else{
      array_dims ++;
      exp->Dump();
    }
    std::cout << "finish exp a" << endl;
  }
};


class ExpBAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> expb;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> initval;
  int type;
  void Dump() const override
  {
    std::cout << "in exp b" << endl;
    if(type == 1){
      int total_need_fill = need_to_fill;
      initval->Dump();
      need_to_fill = total_need_fill;
      // this is not another {}
      if(!imm_stack.empty()){
        int ttldepth = array_index[block_depth][cur_array].size();
        stack<int> ref_stack;
        vector<int> ref_vector;
        int tmp = cur_total_num;
        cur_total_num += 1;
        std::cout << "  in exp b type 1 : " << tmp << endl;
        for(int i = 0 ; i < ttldepth ; i++){
          ref_stack.push(tmp%array_index[block_depth][cur_array][i]);
          ref_vector.push_back(tmp%array_index[block_depth][cur_array][i]);
          tmp /= array_index[block_depth][cur_array][i];
        }
        if(block_depth != 0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + cur_array + "_" + to_string(block_depth) + ", " + to_string(ref_stack.top()) + "\n";
          ref_stack.pop();
          exp_depth ++;
          for(int i = 1 ; i < ttldepth ; i ++){
            int refi = ref_stack.top();
            ref_stack.pop();
            koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth-1) + ", " + to_string(refi) + "\n";
            exp_depth++;
          }
          koopa_string += "  store " + to_string(imm_stack.top()) + ", %" + to_string(exp_depth-1) + "\n"; 
          imm_stack.pop();
          cur_num.pop();
        }
        else{
          std::cout << "    step2 - global: " << ref_stack.top() << endl;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == 0);
            }
            if(flg){
              koopa_string += "{";
            }
          }
          koopa_string += to_string(imm_stack.top());
          imm_stack.pop();
          cur_num.pop();
          int flg2 = 1;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == array_index[block_depth][cur_array][k] - 1);
            }
            flg2 = flg2 && flg;
            if(flg){
              koopa_string += "}";
            }
          }
          if(!flg2){
            koopa_string += ", ";
          }
          std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        }
      }
      total_need_fill = need_to_fill;
      expb->Dump();
      need_to_fill = total_need_fill;
    }
    if(type == 2){
      std::cout << "  in exp b stage 1" << endl;
      int total_need_fill = need_to_fill;
      initval->Dump();
      need_to_fill = total_need_fill;
      std::cout << "  in exp b stage 2" << endl;
      // this is not another {}
      if(!imm_stack.empty()){
        int ttldepth = array_index[block_depth][cur_array].size();
        stack<int> ref_stack;
        vector<int> ref_vector;
        int tmp = cur_total_num;
        cur_total_num += 1;
        std::cout << "  in exp b type 1 : " << tmp << endl;
        for(int i = 0 ; i < ttldepth ; i++){
          ref_stack.push(tmp%array_index[block_depth][cur_array][i]);
          ref_vector.push_back(tmp%array_index[block_depth][cur_array][i]);
          tmp /= array_index[block_depth][cur_array][i];
        }
        if(block_depth != 0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + cur_array + "_" + to_string(block_depth) + ", " + to_string(ref_stack.top()) + "\n";
          ref_stack.pop();
          exp_depth ++;
          for(int i = 1 ; i < ttldepth ; i ++){
            int refi = ref_stack.top();
            ref_stack.pop();
            koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth-1) + ", " + to_string(refi) + "\n";
            exp_depth++;
          }
          koopa_string += "  store " + to_string(imm_stack.top()) + ", %" + to_string(exp_depth-1) + "\n"; 
          imm_stack.pop();
          cur_num.pop();
        }
        else{
          std::cout << "    step2 - global: " << ref_stack.top() << endl;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == 0);
            }
            if(flg){
              koopa_string += "{";
            }
          }
          koopa_string += to_string(imm_stack.top());
          imm_stack.pop();
          cur_num.pop();
          int flg2 = 1;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == array_index[block_depth][cur_array][k] - 1);
            }
            flg2 = flg2 && flg;
            if(flg){
              koopa_string += "}";
            }
          }
          if(!flg2){
            koopa_string += ", ";
          }
          std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        }
      }
      std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        
      for(int tmp = cur_total_num ; tmp < need_to_fill ;){
        int ttldepth = array_index[block_depth][cur_array].size();
        std::cout << "  in const exp b stage 7" << endl;
        stack<int> ref_stack;
        vector<int> ref_vector;
        for(int i = 0 ; i < ttldepth ; i++){
          ref_stack.push(tmp%array_index[block_depth][cur_array][i]);
          ref_vector.push_back(tmp%array_index[block_depth][cur_array][i]);
          tmp /= array_index[block_depth][cur_array][i];
        }
        if(block_depth != 0){
          koopa_string += "  %" + to_string(exp_depth) + " = getelemptr @" + cur_array + "_" + to_string(block_depth) + ", " + to_string(ref_stack.top()) + "\n";
          ref_stack.pop();
          exp_depth ++;
          for(int i = 1 ; i < ttldepth ; i ++){
            int refi = ref_stack.top();
            ref_stack.pop();
            koopa_string += "  %" + to_string(exp_depth) + " = getelemptr %" + to_string(exp_depth-1) + ", " + to_string(refi) + "\n";
            exp_depth++; 
          }
          koopa_string += "  store " + to_string(0) + ", %" + to_string(exp_depth-1) + "\n"; 
          cur_total_num++;
          tmp = cur_total_num;
        }
        else{
          std::cout << "    step2 - global: " << ref_stack.top() << endl;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == 0);
            }
            if(flg){
              koopa_string += "{";
            }
          }
          koopa_string += "0";
          int flg2 = 1;
          for(int j = 0; j < ttldepth ; j++){
            int flg = 1;
            for(int k = j; k >= 0 ; k--){
              flg = flg && (ref_vector[k] == array_index[block_depth][cur_array][k] - 1);
            }
            flg2 = flg2 && flg;
            if(flg){
              koopa_string += "}";
            }
          }
          if(!flg2){
            koopa_string += ", ";
          }
          cur_total_num++;
          tmp = cur_total_num;
          std::cout << "    step3 : need_to_fill: " <<need_to_fill << "; cur_total_num : " <<cur_total_num << endl;
        }
      }
    }
  }
};

#endif
