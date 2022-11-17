//---------------------------------------------------------------------------
// SCPインスタンスを扱うクラスの例
// vector
// 配列と構造体を使っています。
// クラスと言いながら，実際は構造体みたいな使い方
// Araki
//---------------------------------------------------------------------------
#pragma once

#include "Random.hpp"
#include <vector>
#include <cstdio>

//
//
//  Class SCPinstance  SCPのインスタンスを管理するクラス
//
//
class SCPinstance
{
 public:
  int     numRows;             // The num of rows
  int     numColumns;          // The num of columns
  double  Density;             // The density of the matrix

 public:
  SCPinstance(FILE *SourceFile);
  ~SCPinstance();

  std::vector<std::vector<int> > RowCovers;	// 行をカバーする列のリスト
  std::vector<std::vector<int> > ColEntries;	// 列がカバーする行のリスト
  std::vector<int> Cost;
};

class DataException {};



//
//
//  Class SCPsolution SCPの候補解を管理するクラス
//
//
class SCPsolution
{
 public:
  SCPinstance *SCP_Instance;
  int nRow;                     // 行数
  int nCol;                     // 列数
  int K;                        // 選択する列の数

  std::vector<int> CS;                   // CS: 候補解（列番号のリスト）
  std::vector<int> SOLUTION;             // SOLUTION[j] = 1: 列jが候補解に含まれる
  std::vector<int> COVERED;              // COVERED[i]: 行iがカバーされている回数
  int num_Cover;             // カバーされた行の数

 public:
  SCPsolution(SCPinstance &inst, int k); // 行数，列数，K
  ~SCPsolution();

  // 候補解を初期化
  void initialize(SCPinstance &inst);

  // CSに列cを追加する
  void add_column(SCPinstance &inst, int c);

  // CSから列cを削除する
  void remove_column(SCPinstance &inst, int c);

  // CSの中身を表示
  void print_solution();
};
