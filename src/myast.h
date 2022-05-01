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
static map<std::string, std::string> const_symtbl;
static map<std::string, std::string> var_symtbl;
static vector<string> func_var;
static vector<vector<string> > param_list;
static vector<map<std::string, std::string> > var_symtbl_b;
static vector<map<std::string, std::string> > const_symtbl_b;
static vector<map<std::string, std::string> > saved_var_symtbl_b;
static vector<map<std::string, std::string> > saved_const_symtbl_b;
static map<std::string, std::string> func_tbl;
static map<std::string, int> quick_ret;
static std::string koopa_string;
static std::string cur_func;
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
  virtual ~BaseAST() = default;
  virtual void Dump(FILE *fout, char *koopa_str) const = 0;
};

class CompUnitAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> compunit;

  void Dump(FILE *fout, char *koopa_str) const override
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
    compunit -> Dump(fout, koopa_str);
    
    strcpy(koopa_str, koopa_string.c_str());
  }
};

class CompUnitBAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> compunit;
  std::unique_ptr<BaseAST> func_def;
  std::unique_ptr<BaseAST> decl;
  int type;

  void Dump(FILE *fout, char *koopa_str) const override
  {
    cout<<"in compunitb"<<endl;
    if(type == 1){
      compunit -> Dump(fout, koopa_str);
      func_def->Dump(fout, koopa_str);
    }
    if(type == 2){
      compunit -> Dump(fout, koopa_str);
      decl->Dump(fout, koopa_str);
    }
    if(type == 3){
      func_def->Dump(fout, koopa_str);
    }
    if(type == 4){
      decl->Dump(fout, koopa_str);
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

  void Dump(FILE *fout, char *koopa_str) const override
  {
    cout<<"in funcdef"<<endl;
    cur_func = ident;
    func_var = *new vector<string>();
    if(type == 1){
      koopa_string += "fun @" + ident + "()";
      func_tbl[ident] = "int"; 
      koopa_string += ": i32 {\n%entry:\n";
      cout<<2<<endl;
      block->Dump(fout, koopa_str);
      if(!terminated.back()){
        koopa_string += "  ret\n";
      }
      koopa_string += "}\n";
    }
    if(type == 2){
      koopa_string += "fun @" + ident + "(";
      fparams -> Dump(fout, koopa_str);
      koopa_string += ")";
      func_tbl[ident] = "int";
      koopa_string += ": i32 {\n%entry:\n";
      cout<<2<<endl;
      block->Dump(fout, koopa_str);
      if(!terminated.back()){
        koopa_string += "  ret\n";
      }
      koopa_string += "}\n";
    }
    if(type == 3){
      koopa_string += "fun @" + ident + "()";
      func_tbl[ident] = "void"; 
      koopa_string += "{\n%entry:\n";
      cout<<2<<endl;
      block->Dump(fout, koopa_str);
      if(!terminated.back()){
        quick_ret[cur_func] = -114514;
        koopa_string += "  ret\n";
      }
      koopa_string += "}\n";
    }
    if(type == 4){
      koopa_string += "fun @" + ident + "(";
      fparams -> Dump(fout, koopa_str);
      koopa_string += ")";
      func_tbl[ident] = "void";
      koopa_string += "{\n%entry:\n";
      cout<<2<<endl;
      block->Dump(fout, koopa_str);
      if(!terminated.back()){
        quick_ret[cur_func] = -114514;
        koopa_string += "  ret\n";
      }
      koopa_string += "}\n";
    }
  }
};

class FuncFParamsAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> fparams;
  int type;

  void Dump(FILE *fout, char *koopa_str) const override
  {
    cout<<"in fparam"<<endl;
    if(type == 1){
      koopa_string += "@" + ident;
      koopa_string += ": i32";
      koopa_string += ",";
      func_var.push_back(ident);
      fparams->Dump(fout, koopa_str);
    }
    if(type == 2){
      koopa_string += "@" + ident;
      koopa_string += ": i32";
      func_var.push_back(ident);
    }
  }
};

class FuncTypeAST : public BaseAST
{
public:
  std::string functype;
  void Dump(FILE *fout, char *koopa_str) const override
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
  void Dump(FILE *fout, char *koopa_str) const override
  {
    if(type == 1){
      terminated.push_back(0);
      var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      const_symtbl_b.push_back(*(new map<std::string, std::string>()));
      saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
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
      std::cout << "in block" << endl;
      blockitem->Dump(fout, koopa_str);
      var_symtbl_b.pop_back();
      const_symtbl_b.pop_back();
      block_depth -= 1;
    }
    else{
      terminated.push_back(0);
      var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      const_symtbl_b.push_back(*(new map<std::string, std::string>()));
      saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
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
      std::cout << "in block" << endl;
    }
  }
};
class StmtAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  std::string lval;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> ifstmt;
  std::unique_ptr<BaseAST> elsestmt;
  int type;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in stmt" << endl;
    if (terminated.back())
    {
      return;
    }
    if ((type == 1 || type == 3) && !terminated.back())
    {
      terminated.back() = 1;
      std::cout << "StmtAST { ";
      exp->Dump(fout, koopa_str);
      koopa_string += "  ret ";
      if (cur_num.empty() || type == 3)
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
        //std::cout << " }";
      }
    }
    // fuzhiyuju
    else if (type == 2)
    {
      exp->Dump(fout, koopa_str);
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
      exp->Dump(fout, koopa_str);
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
      block->Dump(fout, koopa_str);
    }
    else if(type == 7){
      exp -> Dump(fout, koopa_str);
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
      ifstmt -> Dump(fout, koopa_str);
      if(!terminated.back())
          koopa_string += "  jump %end" + to_string(thiscnt) + "\n";
      terminated.pop_back();
      koopa_string += "%end" + to_string(thiscnt) + ":\n";
    }
    else if(type == 8){
      exp -> Dump(fout, koopa_str);
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
      ifstmt -> Dump(fout, koopa_str);
      if(!terminated.back())
          koopa_string += "  jump %end" + to_string(thiscnt) + "\n";
      terminated.pop_back();
      koopa_string += "%else" + to_string(thiselsecnt) + ":\n";
      terminated.push_back(0);
      elsestmt -> Dump(fout, koopa_str);
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
      exp -> Dump(fout, koopa_str);
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
      ifstmt -> Dump(fout, koopa_str);
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
  }
};

class ExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> lorexp;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in exp" << endl;
    lorexp->Dump(fout, koopa_str);
  }
};

class LOrExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> landexp;
  std::unique_ptr<BaseAST> lorexp;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in lorexp" << endl;
    if (type == 1)
    {
      landexp->Dump(fout, koopa_str);
    }
    if (type != 1)
    {
      lorexp->Dump(fout, koopa_str);
      int ttype1;
      int ttype2;
      string exp1;
      string exp2;
      ttype1 = cur_num.top();
      cur_num.pop();
      if(ttype1 == -1){
        int tmptop = imm_stack.top();
        if(tmptop == 1){
          imm_stack.pop();
          cur_num.push(-1);
          imm_stack.push(1);
          return;
        }
      }
      landexp->Dump(fout, koopa_str);
      ttype2 = cur_num.top();
      cur_num.pop();
      if(ttype1 == -1 && ttype2 == -1){
        int var1 = imm_stack.top();
        imm_stack.pop();
        int var2 = imm_stack.top();
        imm_stack.pop();
        cur_num.push(-1);
        imm_stack.push(var1||var2);
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
          exp2 = "%" +  to_string(ttype2);
        }
        string myexp1;
        myexp1 = "%" + to_string(exp_depth);
        exp_depth += 1;
        string myexp2;
        myexp2 = "%" + to_string(exp_depth);
        exp_depth += 1;
        string myexp3;
        myexp3 = "%" + to_string(exp_depth);
        exp_depth += 1;
        koopa_string += "  " + myexp1 + " = ne 0, " + exp1 + "\n";
        koopa_string += "  " + myexp2 + " = ne 0, " + exp2 + "\n";
        koopa_string += "  " + myexp3 + " = or " + myexp1 + ", " + myexp2 + "\n";
        cur_num.push(exp_depth - 1);
      }
    }
  }
};

class LAndExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> eqexp;
  std::unique_ptr<BaseAST> landexp;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in landexp" << endl;
    if (type == 1)
    {
      eqexp->Dump(fout, koopa_str);
    }
    if (type != 1)
    {
      landexp->Dump(fout, koopa_str);
      int ttype1;
      int ttype2;
      string exp1;
      string exp2;
      ttype1 = cur_num.top();
      cur_num.pop();
      if(ttype1 == -1){
        int tmptop = imm_stack.top();
        if(tmptop == 0){
          imm_stack.pop();
          cur_num.push(-1);
          imm_stack.push(0);
          return;
        }
      }
      eqexp->Dump(fout, koopa_str);
      ttype2 = cur_num.top();
      cur_num.pop();
      if(ttype1 == -1 && ttype2 == -1){
        int var1 = imm_stack.top();
        imm_stack.pop();
        int var2 = imm_stack.top();
        imm_stack.pop();
        cur_num.push(-1);
        imm_stack.push(var1&&var2);
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
        
        string myexp1;
        myexp1 = "%" + to_string(exp_depth);
        exp_depth += 1;
        string myexp2;
        myexp2 = "%" + to_string(exp_depth);
        exp_depth += 1;
        string myexp3;
        myexp3 = "%" + to_string(exp_depth);
        exp_depth += 1;
        koopa_string += "  " + myexp1 + " = ne 0, " + exp1 + "\n";
        koopa_string += "  " + myexp2 + " = ne 0, " + exp2 + "\n";
        koopa_string += "  " + myexp3 + " = and " + myexp1 + ", " + myexp2 + "\n";
        cur_num.push(exp_depth - 1);
      }
    }
  }
};

class EqExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> eqexp;
  std::unique_ptr<BaseAST> relexp;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in eqexp" << endl;
    if (type == 1)
    {
      relexp->Dump(fout, koopa_str);
    }
    if (type != 1)
    {
      relexp->Dump(fout, koopa_str);
      eqexp->Dump(fout, koopa_str);
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
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in rel exp" << endl;
    if (type == 1)
    {
      addexp->Dump(fout, koopa_str);
    }
    if (type != 1)
    {
      addexp->Dump(fout, koopa_str);
      relexp->Dump(fout, koopa_str);
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
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in add" << endl;
    if (type == 1)
    {
      mulexp->Dump(fout, koopa_str);
    }
    if (type != 1)
    {
      addexp->Dump(fout, koopa_str);
      mulexp->Dump(fout, koopa_str);
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
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in mul" << endl;
    if (type == 1)
    {
      unaryexp->Dump(fout, koopa_str);
    }
    if (type != 1)
    {
      unaryexp->Dump(fout, koopa_str);
      mulexp->Dump(fout, koopa_str);
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
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in unary" << endl;
    if (type == 1)
    {
      std::cout << "in unary1" << endl;
      primaryexp->Dump(fout, koopa_str);
    }
    if (type != 1 && type != 5 && type != 6)
    {
      std::cout << "in unary234" << endl;
      unaryexp->Dump(fout, koopa_str);
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
      rparams -> Dump(fout, koopa_str);
      if(quick_ret[ident] != -114514){
        cur_num.push(-1);
        imm_stack.push(quick_ret[ident]);
        koopa_string += "  call @" + ident + "()\n";
      } 
      else{
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
        }
        koopa_string += ")\n";
        param_list.pop_back();
      }
    }
  }
};

class FuncRParamsAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> rparams;
  int type;

  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in rparam" << endl;
    exp->Dump(fout, koopa_str);
    int ttype = cur_num.top();
    cur_num.pop();
    if(ttype == -1){
      param_list.back().push_back(to_string(imm_stack.top()));
      imm_stack.pop();
    }
    else{
      param_list.back().push_back("%" + to_string(ttype));
    }
    if(type == 1){
      rparams -> Dump(fout, koopa_str);
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
  void Dump(FILE *fout, char *koopa_str) const override
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
      for (depth1 = block_depth; depth1 >= 0; depth1--)
      {
        if (const_symtbl_b[depth1].find(lval) != const_symtbl_b[depth1].end())
        {
          it1 = const_symtbl_b[depth1].find(lval);
          ttype = 1;
          break;
        }
      }
      for (depth2 = block_depth; depth2 > depth1; depth2--)
      {
        if (var_symtbl_b[depth2].find(lval) != var_symtbl_b[depth2].end())
        {
          it2 = var_symtbl_b[depth2].find(lval);
          ttype = 2;
          break;
        }
      }
      // variable need to be load
      if (ttype == 2)
      {
        std::cout << "no target in const symbol" << endl;
        string myexp;
        myexp = "%" + to_string(exp_depth);
        koopa_string += "  " + myexp + " = load @" + lval + "_" + to_string(depth2) + "\n";
        cur_num.push(exp_depth);
        exp_depth++;
      }
      else
      {
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
    }
    else if (type == 2)
      number->Dump(fout, koopa_str);
    else
      exp->Dump(fout, koopa_str);
  }
};

class NumberAST : public BaseAST
{
public:
  int num;
  void Dump(FILE *fout, char *koopa_str) const override
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
  void Dump(FILE *fout, char *koopa_str) const override
  {
    if (terminated.back())
    {
      return;
    }
    std::cout << "in blockitem" << endl;
    if (type == 1)
    {
      std::cout << "    type1" << endl;
      blockitem->Dump(fout, koopa_str);
      if (terminated.back())
      {
        return;
      }
      stmt->Dump(fout, koopa_str);
    }
    else if (type == 2)
    {
      std::cout << "    type2" << endl;
      blockitem->Dump(fout, koopa_str);
      if (terminated.back())
      {
        return;
      }
      decl->Dump(fout, koopa_str);
    }
    else if (type == 3)
    {
      std::cout << "    type3" << endl;
      decl->Dump(fout, koopa_str);
    }
    else if (type == 4)
    {
      std::cout << "    type4" << endl;
      stmt->Dump(fout, koopa_str);
    }
  }
};

class DeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constdecl;
  std::unique_ptr<BaseAST> vardecl;
  int type;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in decl" << endl;
    if (type == 1)
    {
      constdecl->Dump(fout, koopa_str);
    }
    else
    {
      vardecl->Dump(fout, koopa_str);
    }
  }
};

class ConstDeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constdef;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in const decl" << endl;
    constdef->Dump(fout, koopa_str);
  }
};

class ConstDefAST : public BaseAST
{
public:
  std::string lval;
  std::unique_ptr<BaseAST> constdef;
  std::unique_ptr<BaseAST> constinitval;
  int type;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in const def" << endl;
    if (type == 2)
    {
      constdef->Dump(fout, koopa_str);
    }
    constinitval->Dump(fout, koopa_str);
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
};

class ConstInitValAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> constexp;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in const init val" << endl;
    constexp->Dump(fout, koopa_str);
  }
};

class ConstExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in const exp" << endl;
    exp->Dump(fout, koopa_str);
  }
};

class VarDeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> vardef;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in var decl" << endl;
    vardef->Dump(fout, koopa_str);
  }
};

class VarDefAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> vardef;
  std::string lval;
  std::unique_ptr<BaseAST> initval;
  int type;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in var def" << endl;
    if (type == 1)
    {
        vardef->Dump(fout, koopa_str);
      if(saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end()){
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
      vardef->Dump(fout, koopa_str);
      initval->Dump(fout, koopa_str);
      if(saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end() && block_depth != 0){
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
      if(saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end()){
        if(block_depth != 0)
          koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc i32\n";
        else
          koopa_string += "global @" + lval + "_" + to_string(block_depth) + " = alloc i32, zeroinit\n";
        var_symtbl_b[block_depth][lval] = "";
        saved_var_symtbl_b[block_depth][lval] = "";
      }
    }
    if (type == 4)
    {
      initval->Dump(fout, koopa_str);
      if(saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end() && block_depth != 0){
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
  }
};

class InitValAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in init val" << endl;
    exp->Dump(fout, koopa_str);
  }
};

#endif
