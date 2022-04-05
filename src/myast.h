#ifndef MYAST_H
#define MYAST_H

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
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
static int exp_depth;
static int terminated = 0;
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
      std::cout << "in functype"<<endl; 
      strcat(koopa_str, " ");
      strcat(koopa_str, "i");
      strcat(koopa_str, "32");
      strcat(koopa_str, " {\n");
  } 
};
class BlockAST :  public BaseAST{
    public:
        std::unique_ptr<BaseAST> blockitem;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in block"<<endl; 
      strcat(koopa_str, "%entry:\n");
      blockitem->Dump(fout,koopa_str);
    }
};
class StmtAST : public BaseAST{
    public:
        std::unique_ptr<BaseAST> exp;
        std::string lval;
        int type;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in stmt"<<endl; 
      if(terminated){return;}
      if(type == 1 && !terminated){
        terminated = 1;
        std::cout << "StmtAST { ";
        exp->Dump(fout,koopa_str);
        strcat(koopa_str, "  ret ");
        if(cur_num.empty())
          strcat(koopa_str, "0\n");
        else{
          char exp1[20];
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
      }  
      //fuzhiyuju
      else if(type == 2){ 
        exp->Dump(fout,koopa_str);
        int ttype = cur_num.top();
        cur_num.pop();
        string exp1;
        if(ttype == -1){
          std::cout << "give no expr";  

          exp1 += to_string(imm_stack.top());
          imm_stack.pop();
        }
        else{
          std::cout << "give expr";  
          exp1 += "%";
          exp1 += to_string(ttype);
        }
        var_symtbl[lval] = exp1;
        strcat(koopa_str,"  store ");
        strcat(koopa_str,exp1.c_str());
        strcat(koopa_str," ,@");
        strcat(koopa_str,lval.c_str());
        strcat(koopa_str,"\n");
      }
    }
};

class ExpAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> lorexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    std::cout<<"in exp"<<endl;
    lorexp -> Dump(fout,koopa_str);
  } 

};

class LOrExpAST : public BaseAST{
  public:
    int type;
    std::unique_ptr<BaseAST> landexp;
    std::unique_ptr<BaseAST> lorexp;
  void Dump(FILE * fout, char * koopa_str) const override {
    std::cout << "in lorexp"<<endl; 
    if(type == 1){
      landexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      landexp -> Dump(fout, koopa_str);
      lorexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[20];
      char exp2[20];
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
      char myexp1[20];
      memset(myexp1,0,sizeof(myexp1));
      myexp1[0] = '%';
      strcat(myexp1, to_string(exp_depth).c_str());
      exp_depth += 1;
      char myexp2[20];
      memset(myexp2,0,sizeof(myexp2));
      myexp2[0] = '%';
      strcat(myexp2, to_string(exp_depth).c_str());
      exp_depth += 1;
      char myexp3[20];
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
    std::cout << "in landexp"<<endl; 
    if(type == 1){
      eqexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      eqexp -> Dump(fout, koopa_str);
      landexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[20];
      char exp2[20];
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
      char myexp1[20];
      memset(myexp1,0,sizeof(myexp1));
      myexp1[0] = '%';
      strcat(myexp1, to_string(exp_depth).c_str());
      exp_depth += 1;
      char myexp2[20];
      memset(myexp2,0,sizeof(myexp2));
      myexp2[0] = '%';
      strcat(myexp2, to_string(exp_depth).c_str());
      exp_depth += 1;
      char myexp3[20];
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
    std::cout << "in eqexp"<<endl; 
    if(type == 1){
      relexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      relexp -> Dump(fout, koopa_str);
      eqexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[20];
      char exp2[20];
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
      char myexp[20];
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
    std::cout << "in rel exp"<<endl; 
    if(type == 1){
      addexp -> Dump(fout, koopa_str);
    }
    if(type != 1){
      addexp -> Dump(fout, koopa_str);
      relexp -> Dump(fout, koopa_str);
      int ttype;
      char exp1[20];
      char exp2[20];
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
      char myexp[20];
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
      char exp1[20];
      char exp2[20];
      memset(exp1,0,sizeof(exp1));
      memset(exp2,0,sizeof(exp2));
      //std::cout << "finish parsing both id"<<endl;  
      //std::cout << cur_num.size()<<endl; 
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
      char myexp[20];
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
      std::cout<<"finish add without problem"<<endl;
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
      char exp1[20];
      char exp2[20];
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
      char myexp[20];
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
        char exp1[20];
        memset(exp1,0,sizeof(exp1));
        if(ttype == -1){
          strcpy(exp1, to_string(imm_stack.top()).c_str());
          imm_stack.pop();
        }
        else{
          exp1[0] = '%';
          strcat(exp1, to_string(ttype).c_str());
        }
        char myexp[20];
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
    std::string lval;
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> number;
  void Dump(FILE * fout, char * koopa_str) const override {
    std::cout << "in primaryexp"<<endl;
    if(type == 3)  {
      std::cout << "in primaryexp and going to parse lval"<<endl;
      map<string, string>::iterator it1;
      map<string, string>::iterator it2;
      it1 = const_symtbl.find(lval) ;
      it2 = var_symtbl.find(lval) ;
      //variable need to be load
      if(it1 == const_symtbl.end()){
        std::cout<<"no target in const symbol"<<endl;
        char myexp[20];
        memset(myexp,0,sizeof(myexp));
        myexp[0] = '%';
        strcat(myexp, to_string(exp_depth).c_str());
        cur_num.push(exp_depth);
        strcat(koopa_str, "  %");
        strcat(koopa_str, to_string(exp_depth).c_str());
        strcat(koopa_str, " = load @");
        strcat(koopa_str, lval.c_str());
        strcat(koopa_str, "\n");
        exp_depth++;

      }
      else{
        std::string target = it1->second;
        if(target[0] == '%'){
          std::cout<<"after this, target become:"<<target<<endl;
          target.erase(target.begin());
          int tmp = stoi(target);
          cur_num.push(tmp);
        }
        else{
          std::cout<<"target become:"<<target<<endl;
          cur_num.push(-1);
          imm_stack.push(stoi(target));
        }
      }
    }
    else if(type == 2)
      number -> Dump(fout, koopa_str);
    else
      exp -> Dump(fout, koopa_str);
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

//newly defined

class BlockItemAST : public BaseAST{
    public: 
      std::unique_ptr<BaseAST> stmt;
      std::unique_ptr<BaseAST> decl;
      std::unique_ptr<BaseAST> blockitem;
      int type;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in blockitem"<<endl;  
      if(type == 1){
        std::cout << "    type1"<<endl;  
        blockitem -> Dump(fout, koopa_str);
        stmt -> Dump(fout, koopa_str);
      }
      else if(type == 2){
        std::cout << "    type2"<<endl; 
        blockitem -> Dump(fout, koopa_str);
        decl -> Dump(fout, koopa_str);
      }
      else if(type == 3){
        std::cout << "    type3"<<endl; 
        decl -> Dump(fout, koopa_str);
      }
      else if(type == 4){
        std::cout << "    type4"<<endl; 
        stmt -> Dump(fout, koopa_str);
      }
    }  
};

class DeclAST : public BaseAST{
    public: 
      std::unique_ptr<BaseAST> constdecl;
      std::unique_ptr<BaseAST> vardecl;
      int type;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in decl"<<endl; 
      if(type == 1){
        constdecl -> Dump(fout, koopa_str);
      }
      else{
        vardecl -> Dump(fout, koopa_str);
      }
    }  
};

class ConstDeclAST : public BaseAST{
    public: 
      std::unique_ptr<BaseAST> constdef;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in const decl"<<endl; 
      constdef -> Dump(fout, koopa_str);
    }  
};


class ConstDefAST : public BaseAST{
    public: 
      std::string lval;
      std::unique_ptr<BaseAST> constdef;
      std::unique_ptr<BaseAST> constinitval;
      int type ;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in const def"<<endl; 
      if(type == 2){
        constdef -> Dump(fout, koopa_str);
      }
      constinitval -> Dump(fout, koopa_str);
      int ttype = cur_num.top();
      cur_num.pop();
      if(ttype == -1){
        int tmp = imm_stack.top();
        imm_stack.pop();
        const_symtbl[lval] = to_string(tmp);
      }
      else{
        const_symtbl[lval] = "%" + to_string(ttype);
      }

    }  
};

class ConstInitValAST : public BaseAST{
    public: 
      std::unique_ptr<BaseAST> constexp;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in const init val"<<endl; 
      constexp -> Dump(fout, koopa_str);
    }  
};

class ConstExpAST : public BaseAST{
    public: 
      std::unique_ptr<BaseAST> exp;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in const exp"<<endl; 
      exp -> Dump(fout, koopa_str);
    }  
};

class VarDeclAST : public BaseAST{
    public: 
      std::unique_ptr<BaseAST> vardef;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in var decl"<<endl; 
      vardef -> Dump(fout, koopa_str);
    }  
};

class VarDefAST : public BaseAST{
    public: 
      std::unique_ptr<BaseAST> vardef;
      std::string lval;
      std::unique_ptr<BaseAST> initval;
      int type;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in var def"<<endl; 
      if(type == 1){
        vardef -> Dump(fout, koopa_str);
        strcat(koopa_str, "  @");
        strcat(koopa_str, lval.c_str());
        strcat(koopa_str, " = alloc i32\n");
        var_symtbl[lval] = "";
      }
      if(type == 2){
        vardef -> Dump(fout, koopa_str);
        initval -> Dump(fout, koopa_str);  
        strcat(koopa_str, "  @");
        strcat(koopa_str, lval.c_str());
        strcat(koopa_str, " = alloc i32\n");
        int ttype = cur_num.top();
        cur_num.pop();
        std::cout<<"fine in type2"<<endl;
        if(ttype == -1){
          int tmp = imm_stack.top();
          imm_stack.pop();
          var_symtbl[lval] = to_string(tmp);
          strcat(koopa_str, "  store ");
          strcat(koopa_str, var_symtbl[lval].c_str());
          strcat(koopa_str, " ,@");
          strcat(koopa_str, lval.c_str());
          strcat(koopa_str, "\n");
        }
        else{
          var_symtbl[lval] = "%" + to_string(ttype);
          strcat(koopa_str, "  store ");
          strcat(koopa_str, var_symtbl[lval].c_str());
          strcat(koopa_str, " ,@");
          strcat(koopa_str, lval.c_str());
          strcat(koopa_str, "\n");
        }
      }
      if(type == 3){
        strcat(koopa_str, "  @");
        strcat(koopa_str, lval.c_str());
        strcat(koopa_str, " = alloc i32\n");
        var_symtbl[lval] = "";
      }
      if(type == 4){
        initval -> Dump(fout, koopa_str);
        strcat(koopa_str, "  @");
        strcat(koopa_str, lval.c_str());
        strcat(koopa_str, " = alloc i32\n");
        int ttype = cur_num.top();
        cur_num.pop();
        std::cout<<"fine in type2"<<endl;
        if(ttype == -1){
          int tmp = imm_stack.top();
          imm_stack.pop();
          var_symtbl[lval] = to_string(tmp);
          strcat(koopa_str, "  store ");
          strcat(koopa_str, var_symtbl[lval].c_str());
          strcat(koopa_str, " ,@");
          strcat(koopa_str, lval.c_str());
          strcat(koopa_str, "\n");
        }
        else{
          var_symtbl[lval] = "%" + to_string(ttype);
          strcat(koopa_str, "  store ");
          strcat(koopa_str, var_symtbl[lval].c_str());
          strcat(koopa_str, " ,@");
          strcat(koopa_str, lval.c_str());
          strcat(koopa_str, "\n");
        }
      }
    }  
};

class InitValAST : public BaseAST{
    public: 
      std::unique_ptr<BaseAST> exp;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "in init val"<<endl; 
      exp -> Dump(fout, koopa_str);
    }  
};


#endif


