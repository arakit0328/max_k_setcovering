//---------------------------------------------------------------------------
// SCPインスタンスを扱うクラスの例
// 配列と構造体を使っています。
// クラスと言いながら，実際は構造体みたいな使い方
// Araki
//---------------------------------------------------------------------------
#pragma once

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

  typedef struct _RowData       // 行を管理する構造体
  {
    int numCover;               // 行をカバーする列の数
    int* Covers;                // 行をカバーする列のリスト（配列）
  } RowData;
  typedef struct _ColData       // 列を管理する構造体
  {
    int   Cost;                 // 列のコスト
    int   numEntry;             // 列がカバーする行の数
    int*  Entries;              // 列がカバーする行のリスト（配列）
  } ColData;

  RowData *Rows;                // 行の情報 [1..numRows]
  ColData *Cols;                // 列の情報 [1..numColumns]
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

  int *CS;                   // CS: 候補解（列番号のリスト）
  int *SOLUTION;             // SOLUTION[j] = 1: 列jが候補解に含まれる
  int *COVERED;              // COVERED[i]: 行iがカバーされている回数
  int *SCORE;                // SCORE[j]: 各列のスコア
  int num_Cover;             // カバーされた行の数

public:
  SCPsolution(SCPinstance &pData, int k); // 行数，列数，K
  ~SCPsolution();

  // 候補解を初期化
  void initialize(SCPinstance &pData);

  // CSに含まれない列から最大スコアのものを選んで返す
  int get_column_maxscore(SCPinstance &pData);

  // CSに含まれない列から alpha * 最大スコア 以上のものを選んで返す
  int get_column_grasp(SCPinstance &pData, double alpha);

  // CSに列cを追加する
  void add_column(SCPinstance &pData, int c);

  // CSから列cを削除する
  void remove_column(SCPinstance &pData, int c);

  // CSの中身を表示
  void print_solution();

  // 解をコピーする
  void copy(const SCPsolution& cs);
};
