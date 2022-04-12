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
static vector<map<std::string, std::string> > var_symtbl_b;
static vector<map<std::string, std::string> > const_symtbl_b;
static vector<map<std::string, std::string> > saved_var_symtbl_b;
static vector<map<std::string, std::string> > saved_const_symtbl_b;
static int exp_depth;
static int block_depth;
static int max_block_depth;
static bool optrue = 0;
static vector<int> terminated;
static int ifcnt = 0;
static int elsecnt = 0;
static std::string koopa_string;
class BaseAST
{
public:
  virtual ~BaseAST() = default;
  virtual void Dump(FILE *fout, char *koopa_str) const = 0;
};

class CompUnitAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> func_def;

  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "CompUnitAST { ";
    func_def->Dump(fout, koopa_str);
    std::cout << " }";
  }
};

class FuncDefAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "FuncDefAST { ";
    koopa_string += "fun @" + ident + "():";
    func_type->Dump(fout, koopa_str);
    std::cout << ", " << ident << ", ";
    block->Dump(fout, koopa_str);
    koopa_string += "}\n";
    strcpy(koopa_str, koopa_string.c_str());
    std::cout << " }";
  }
};

class FuncTypeAST : public FuncDefAST
{
public:
  std::string functype;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    terminated.push_back(0);
    var_symtbl_b.push_back(*(new map<std::string, std::string>()));
    const_symtbl_b.push_back(*(new map<std::string, std::string>()));
    saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
    saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
    std::cout << "in functype" << endl;
    koopa_string += " i32 {\n%entry:\n";
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
      std::cout << "in block" << endl;
      blockitem->Dump(fout, koopa_str);
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
          koopa_string +=  to_string(imm_stack.top()) + "\n";
          imm_stack.pop();
        }
        else
        {
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
      if (ttype == -1)
      {
        std::cout << "give no expr";
        imm_stack.pop();
      }
    }
    else if (type == 5)
    {
      // happy 
    }
    else if(type == 6)
    {
      block_depth += 1;
      if(block_depth > max_block_depth){
        saved_var_symtbl_b.push_back(*(new map<std::string, std::string>()));
        saved_const_symtbl_b.push_back(*(new map<std::string, std::string>()));
        max_block_depth = block_depth;
      }
      var_symtbl_b.push_back(*(new map<std::string, std::string>()));
      const_symtbl_b.push_back(*(new map<std::string, std::string>()));
      max_block_depth = block_depth;
      block->Dump(fout, koopa_str);
      var_symtbl_b.pop_back();
      const_symtbl_b.pop_back();
      block_depth -= 1;
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
      landexp->Dump(fout, koopa_str);
      lorexp->Dump(fout, koopa_str);
      int ttype;
      string exp1;
      string exp2;
      ttype = cur_num.top();
      cur_num.pop();
      if (ttype == -1)
      {
        int tmp = imm_stack.top();
        exp1 = to_string(imm_stack.top());
        imm_stack.pop();
        if(tmp != 0){
          imm_stack.push(1);
          cur_num.push(-1);
          return;
        }
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
        exp2 = "%" +  to_string(ttype);
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
      eqexp->Dump(fout, koopa_str);
      landexp->Dump(fout, koopa_str);
      int ttype;
      string exp1;
      string exp2;
      ttype = cur_num.top();
      cur_num.pop();
      if (ttype == -1)
      {
        int tmp = imm_stack.top();
        exp1 = to_string(imm_stack.top());
        imm_stack.pop();
        if(tmp == 0){
          imm_stack.push(0);
          cur_num.push(-1);
          return;
        }
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
      mulexp->Dump(fout, koopa_str);
      addexp->Dump(fout, koopa_str);
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
};

class UnaryExpAST : public BaseAST
{
public:
  int type;
  std::unique_ptr<BaseAST> primaryexp;
  std::unique_ptr<BaseAST> unaryexp;
  void Dump(FILE *fout, char *koopa_str) const override
  {
    std::cout << "in unary" << endl;
    if (type == 1)
    {
      primaryexp->Dump(fout, koopa_str);
    }
    if (type != 1)
    {
      unaryexp->Dump(fout, koopa_str);
      if (type != 2)
      {
        int ttype = cur_num.top();
        cur_num.pop();
        string exp1;
        if (ttype == -1)
        {
          exp1 = to_string(imm_stack.top());
          imm_stack.pop();
        }
        else
        {
          exp1 = "%" + to_string(ttype);
        }
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
        koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc i32\n";
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
      if(saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end()){
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
        koopa_string += "  store " + var_symtbl_b[block_depth][lval] + " ,@" + lval + "_" + to_string(block_depth) + "\n";
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
        koopa_string += "  @" + lval + "_" + to_string(block_depth) + " = alloc i32\n";
        var_symtbl_b[block_depth][lval] = "";
        saved_var_symtbl_b[block_depth][lval] = "";
      }
    }
    if (type == 4)
    {
      initval->Dump(fout, koopa_str);
      if(saved_var_symtbl_b[block_depth].find(lval) == saved_var_symtbl_b[block_depth].end()){
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
        
        koopa_string += "  store " + var_symtbl_b[block_depth][lval] + " ,@" + lval + "_" + to_string(block_depth) + "\n";
        std::cout << "going out" << endl;
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
