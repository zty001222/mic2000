#ifndef MYAST_H
#define MYAST_H

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
using namespace std;

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump(FILE * fout) const = 0;
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;

  void Dump(FILE * fout) const override {
    std::cout << "CompUnitAST { ";
    func_def->Dump(fout);
    std::cout << " }";
  }
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump(FILE * fout) const override {
      std::cout << "FuncDefAST { ";
      fprintf(fout,"fun @");
      fprintf(fout,"%s",ident.c_str());
      fprintf(fout,"():");
      func_type->Dump(fout);
      std::cout << ", " << ident << ", ";
      block->Dump(fout);
      fprintf(fout,"}\n");
      std::cout << " }";
    }
};

class FuncTypeAST : public FuncDefAST{
    public:
        std::string functype;
    void Dump(FILE * fout) const override {
      std::cout << "FuncTypeAST { ";
      std::cout << functype ;
      fprintf(fout," %c32",functype.c_str()[0]);
      fprintf(fout," {\n");
      std::cout << " }";
  } 
};

class BlockAST :  public BaseAST{
    public:
        std::unique_ptr<BaseAST> stmt;
    void Dump(FILE * fout) const override {
      std::cout << "BlockAST { ";
      fprintf(fout,"%%entry:\n");
      stmt->Dump(fout);
      std::cout << " }";
    }
};
class StmtAST : public BaseAST{
    public:
        std::unique_ptr<BaseAST> number;
    void Dump(FILE * fout) const override {
      std::cout << "StmtAST { ";
      fprintf(fout,"  ret ");
      number->Dump(fout);
      std::cout << " }";  
    }  
};
class NumberAST : public BaseAST{
    public:
        int num;  
    void Dump(FILE * fout) const override {
      fprintf(fout,"%d\n",num);
      std::cout<<num;
    }  
};

#endif