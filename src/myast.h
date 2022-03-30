#ifndef MYAST_H
#define MYAST_H

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <koopa.h>
using namespace std;



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
    func_def->Dump(fout, koopa_str);
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
      block->Dump(fout, koopa_str);
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
        std::unique_ptr<BaseAST> number;
    void Dump(FILE * fout, char * koopa_str) const override {
      std::cout << "StmtAST { ";
      strcat(koopa_str, "  ret ");
      number->Dump(fout,koopa_str);
      std::cout << " }";  
    }  
};
class NumberAST : public BaseAST{
    public:
        int num;  
    void Dump(FILE * fout, char * koopa_str) const override {
      strcat(koopa_str, to_string(num).c_str());
      strcat(koopa_str, "\n");
      std::cout<<num;
    }  
};




#endif