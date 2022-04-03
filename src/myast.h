#ifndef MYAST_H
#define MYAST_H

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <stack>
using namespace std;


static stack<int> cur_num;
static stack<int> imm_stack;
static int exp_depth;
class BaseAST {
 public:
  
  virtual ~BaseAST() = default;
  virtual void Dump(FILE * fout, char * koopa_str) const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

  void Dump(FILE * fout, char * koopa_str) const override {
    std::cout << "CompUnitAST { ";
    func_def->Dump(fout,koopa_str);
    std::cout << " }";
  }
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "FuncDefAST { ";
      strcat(koopa_str, "fun @");
      strcat(koopa_str, ident.c_str());
      strcat(koopa_str, "():");
      func_type->Dump(fout, koopa_str);
      std::cout << ", " << ident << ", ";
      block->Dump(fout,koopa_str);
      strcat(koopa_str, "}\n");
      std::cout << " }";
    }
};

class FuncTypeAST : public FuncDefAST{
    public:
        std::string functype;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "FuncTypeAST { ";
      std::cout << functype ;
      strcat(koopa_str, " ");
      strcat(koopa_str, "i");
      strcat(koopa_str, "32");
      strcat(koopa_str, " {\n");
      std::cout << " }";
  } 
};
class BlockAST :  public BaseAST{
    public:
        std::unique_ptr<BaseAST> stmt;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "BlockAST { ";
      strcat(koopa_str, "%entry:\n");
      stmt->Dump(fout,koopa_str);
      std::cout << " }";
    }
};
class StmtAST : public BaseAST{
    public:
        std::unique_ptr<BaseAST> exp;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "StmtAST { ";
      exp->Dump(fout,koopa_str);
      strcat(koopa_str, "  ret ");
      char exp1[10];
      memset(exp1,0,sizeof(exp1));
      int ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        std::cout << "no expr";  

        strcpy(exp1, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        std::cout << "expr";  
        exp1[0] = '%';
        strcat(exp1, to_string(ttype).c_str());
      }
      strcat(koopa_str, exp1);
      strcat(koopa_str, "\n");
      std::cout << " }";  
    }  
};

class ExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> lorexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    std::cout<<"debug in exp"<<endl;
    lorexp -> Dump(fout,koopa_str);
  } 

};

class LOrExpAST : public BaseAST{
  public:
    int type;
    std::unique_ptr<BaseAST> landexp;
    std::unique_ptr<BaseAST> lorexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    if(type == 1){
      landexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      landexp -> Dump(fout, koopa_str);
      lorexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[10];
      char exp2[10];
      memset(exp1,0,sizeof(exp1));
      memset(exp2,0,sizeof(exp2));
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
      ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        strcpy(exp2, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        exp2[0] = '%';
        strcat(exp2, to_string(ttype).c_str());
      }
      char myexp1[10];
      memset(myexp1,0,sizeof(myexp1));
      myexp1[0] = '%';
      strcat(myexp1, to_string(exp_depth).c_str());
      exp_depth += 1;
      char myexp2[10];
      memset(myexp2,0,sizeof(myexp2));
      myexp2[0] = '%';
      strcat(myexp2, to_string(exp_depth).c_str());
      exp_depth += 1;
      char myexp3[10];
      memset(myexp3,0,sizeof(myexp3));
      myexp3[0] = '%';
      strcat(myexp3, to_string(exp_depth).c_str());
      exp_depth += 1;
      strcat(koopa_str, "  ");
      strcat(koopa_str, myexp1);
      strcat(koopa_str, " = ne 0, ");
      strcat(koopa_str, exp1);
      strcat(koopa_str, "\n");

      strcat(koopa_str, "  ");
      strcat(koopa_str, myexp2);
      strcat(koopa_str, " = ne 0, ");
      strcat(koopa_str, exp2);
      strcat(koopa_str, "\n");

      strcat(koopa_str, "  ");
      strcat(koopa_str, myexp3);
      strcat(koopa_str, " = or ");
      strcat(koopa_str, myexp1);
      strcat(koopa_str, ", ");
      strcat(koopa_str, myexp2);
      strcat(koopa_str, "\n");
      cur_num.push(exp_depth-1);
    }
  } 
};

class LAndExpAST : public BaseAST{
  public:
    int type;

    std::unique_ptr<BaseAST> eqexp;
    std::unique_ptr<BaseAST> landexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    if(type == 1){
      eqexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      eqexp -> Dump(fout, koopa_str);
      landexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[10];
      char exp2[10];
      memset(exp1,0,sizeof(exp1));
      memset(exp2,0,sizeof(exp2));
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
      ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        strcpy(exp2, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        exp2[0] = '%';
        strcat(exp2, to_string(ttype).c_str());
      }
      char myexp1[10];
      memset(myexp1,0,sizeof(myexp1));
      myexp1[0] = '%';
      strcat(myexp1, to_string(exp_depth).c_str());
      exp_depth += 1;
      char myexp2[10];
      memset(myexp2,0,sizeof(myexp2));
      myexp2[0] = '%';
      strcat(myexp2, to_string(exp_depth).c_str());
      exp_depth += 1;
      char myexp3[10];
      memset(myexp3,0,sizeof(myexp3));
      myexp3[0] = '%';
      strcat(myexp3, to_string(exp_depth).c_str());
      exp_depth += 1;
      strcat(koopa_str, "  ");
      strcat(koopa_str, myexp1);
      strcat(koopa_str, " = ne 0, ");
      strcat(koopa_str, exp1);
      strcat(koopa_str, "\n");

      strcat(koopa_str, "  ");
      strcat(koopa_str, myexp2);
      strcat(koopa_str, " = ne 0, ");
      strcat(koopa_str, exp2);
      strcat(koopa_str, "\n");

      strcat(koopa_str, "  ");
      strcat(koopa_str, myexp3);
      strcat(koopa_str, " = and ");
      strcat(koopa_str, myexp1);
      strcat(koopa_str, ", ");
      strcat(koopa_str, myexp2);
      strcat(koopa_str, "\n");
      cur_num.push(exp_depth-1);
    }
  } 
    
};

class EqExpAST : public BaseAST{
  public:
    int type;
    std::unique_ptr<BaseAST> eqexp;
    std::unique_ptr<BaseAST> relexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    if(type == 1){
      relexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      relexp -> Dump(fout, koopa_str);
      eqexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[10];
      char exp2[10];
      memset(exp1,0,sizeof(exp1));
      memset(exp2,0,sizeof(exp2));
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
      ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        strcpy(exp2, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        exp2[0] = '%';
        strcat(exp2, to_string(ttype).c_str());
      }
      char myexp[10];
      memset(myexp,0,sizeof(myexp));
      myexp[0] = '%';
      strcat(myexp, to_string(exp_depth).c_str());
      exp_depth += 1;
      if(type == 2){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = eq ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      if(type == 3){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = ne ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      cur_num.push(exp_depth-1);
    }
  } 
    
};

class RelExpAST : public BaseAST{
  public:
    int type;
    std::unique_ptr<BaseAST> addexp;
    std::unique_ptr<BaseAST> relexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    if(type == 1){
      addexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      addexp -> Dump(fout, koopa_str);
      relexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[10];
      char exp2[10];
      memset(exp1,0,sizeof(exp1));
      memset(exp2,0,sizeof(exp2));
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
      ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        strcpy(exp2, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        exp2[0] = '%';
        strcat(exp2, to_string(ttype).c_str());
      }
      char myexp[10];
      memset(myexp,0,sizeof(myexp));
      myexp[0] = '%';
      strcat(myexp, to_string(exp_depth).c_str());
      exp_depth += 1;
      if(type == 2){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = lt ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      if(type == 3){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = gt ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      if(type == 4){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = le ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      if(type == 5){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = ge ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      cur_num.push(exp_depth-1);
    }
  } 
    
};

class AddExpAST : public BaseAST{
  public:
    int type;
    std::unique_ptr<BaseAST> addexp;
    std::unique_ptr<BaseAST> mulexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    std::cout << "in add"<<endl;  
    if(type == 1){
      mulexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      mulexp -> Dump(fout, koopa_str);
      addexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[10];
      char exp2[10];
      memset(exp1,0,sizeof(exp1));
      memset(exp2,0,sizeof(exp2));
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
      ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        strcpy(exp2, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        exp2[0] = '%';
        strcat(exp2, to_string(ttype).c_str());
      }
      char myexp[10];
      memset(myexp,0,sizeof(myexp));
      myexp[0] = '%';
      strcat(myexp, to_string(exp_depth).c_str());
      exp_depth += 1;
      if(type == 2){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = add ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      if(type == 3){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = sub ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      cur_num.push(exp_depth-1);
    }
  }
    
};

class MulExpAST : public BaseAST{
  public:
    int type;
    std::unique_ptr<BaseAST> mulexp;
    std::unique_ptr<BaseAST> unaryexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    std::cout << "in mul"<<endl;  
    if(type == 1){
      unaryexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      unaryexp -> Dump(fout, koopa_str);
      mulexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[10];
      char exp2[10];
      memset(exp1,0,sizeof(exp1));
      memset(exp2,0,sizeof(exp2));
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
      ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        strcpy(exp2, to_string(imm_stack.top()).c_str());
        imm_stack.pop();
      }
      else{
        exp2[0] = '%';
        strcat(exp2, to_string(ttype).c_str());
      }
      char myexp[10];
      memset(myexp,0,sizeof(myexp));
      myexp[0] = '%';
      strcat(myexp, to_string(exp_depth).c_str());
      exp_depth += 1;
      if(type == 2){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = mul ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      if(type == 3){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = div ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      if(type == 4){
        strcat(koopa_str, "  ");
        strcat(koopa_str, myexp);
        strcat(koopa_str, " = mod ");
        strcat(koopa_str, exp1);
        strcat(koopa_str, ", ");
        strcat(koopa_str, exp2);
        strcat(koopa_str, "\n");
      }
      cur_num.push(exp_depth-1);
    }
  } 
    
};

class UnaryExpAST : public BaseAST{
  public:
    int type;
    std::unique_ptr<BaseAST> primaryexp;
    std::unique_ptr<BaseAST> unaryexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    std::cout << "in unary"<<endl;  
    if(type == 1){
      primaryexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      unaryexp -> Dump(fout, koopa_str);
      if(type != 2){  
        int ttype = cur_num.top();
        cur_num.pop();
        char exp1[10];
        memset(exp1,0,sizeof(exp1));
        if(ttype == -1){
          strcpy(exp1, to_string(imm_stack.top()).c_str());
          imm_stack.pop();
        }
        else{
          exp1[0] = '%';
          strcat(exp1, to_string(ttype).c_str());
        }
        char myexp[10];
        memset(myexp,0,sizeof(myexp));
        myexp[0] = '%';
        strcat(myexp, to_string(exp_depth).c_str());
        exp_depth += 1;
        if(type == 3){
          strcat(koopa_str, "  ");
          strcat(koopa_str, myexp);
          strcat(koopa_str, " = sub 0, ");
          strcat(koopa_str, exp1);
          strcat(koopa_str, "\n");
        }
        if(type == 4){
          strcat(koopa_str, "  ");
          strcat(koopa_str, myexp);
          strcat(koopa_str, " = eq 0, ");
          strcat(koopa_str, exp1);
          strcat(koopa_str, "\n");
        }
        cur_num.push(exp_depth-1);
      }
    }
  } 
    
};

class PrimaryExpAST : public BaseAST{
  public:
    int type;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> number;
  void Dump(FILE * fout, char * koopa_str) const override {
    if(type == 2)
      number -> Dump(fout, koopa_str);
    else
      exp -> Dump(fout, koopa_str);
    std::cout << "in primaryexp"<<endl;  
  } 
    
};

class NumberAST : public BaseAST{
    public:
      int num;  
    void Dump(FILE * fout, char * koopa_str) const override {
    std::cout << "in number"<<endl;  
      cur_num.push(-1);
      imm_stack.push(num);
      std::cout<<num;
    }  
};




#endif