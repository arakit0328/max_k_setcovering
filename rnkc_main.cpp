#include "SCP.hpp"
#include "Random.hpp"
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

// 乱数発生クラス Random.hpp を参照
// rnd() で整数乱数
// rnd(a, b) とするとa以上b以下の一様乱数を生成できる
// Rand rnd;
// mt19937_64 rnd(seed);

// greedy_neighborhood_search で使う繰り返しの回数
int niter = 500;

// graspで使う alpha の値
double alpha = 0.95;

// 貪欲法：スコア最大の列をK列選ぶ
// 引数の cs に結果が入る
void greedy_construction(SCPinstance &pData, SCPsolution &cs, Rand &rnd)
{
  int maxc;

  cs.initialize(pData);
  for (int k = 1; k <= cs.K; k++)
  {
    maxc = cs.get_column_maxscore(pData, rnd);
    cs.add_column(pData, maxc);
  } // End for k
}


// GRASP法：スコアが alpha * (最大値 - 最小値) 以上である列からランダムに一つ選ぶ
// 引数の cs に結果が入る
void grasp_construction(SCPinstance &pData, SCPsolution &cs, double alpha, Rand &rnd)
{
  cs.initialize(pData);

  int c;
  for (int k = 1; k <= cs.K; k++)
  {
    c = cs.get_column_grasp(pData, alpha, rnd);
    cs.add_column(pData, c);
  } // End for k
}


// 配列の順序をランダムに入れ替える
void random_permutation(int *A, int n, Rand& rnd)
{
  int j;
  for (int i = 1; i <= n-1; ++i)
  {
    j = rnd(i, n);
    swap(A[i], A[j]);
  }
}


// 単純な改善法
// 引数の cs に結果が入る
void simple_neighborhood_search(SCPinstance &pData,
                                SCPsolution &cs,
                                Rand& rnd)
{
  int K = cs.K;
  int *idx = new int [cs.K + 1];
  int c1, cov1;
  int c2, cov2;

  copy_n(cs.CS, K + 1, idx);
  random_permutation(idx, K, rnd);
  cov1 = cs.num_Cover;

  for (int i=1; i <= K; ++i)
  {
    c1 = idx[i];
    cs.remove_column(pData, c1);

    // 最大スコアの列
    c2 = cs.get_column_maxscore(pData, rnd);
    cs.add_column(pData, c2);
    cov2 = cs.num_Cover;

    if (cov1 > cov2)
    {
      cs.remove_column(pData, c2);
      cs.add_column(pData, c1);
    }
    else
    {
      cov1 = cs.num_Cover;
    }
  } // End for i

  delete [] idx;
}


// 貪欲初期解＋単純局所探索を niter 回繰り返し
// 引数の cs と best_cs は変更される
// 引数の best_cs に結果が入る
void greedy_neighborhood_search(SCPinstance &pData,
                                int K,
                                int niter,
                                SCPsolution &cs,
                                SCPsolution &best_cs,
                                Rand& rnd)
{
  // 貪欲法
  for (int iter = 1; iter <= niter; ++iter)
  {
    // 貪欲法
    greedy_construction(pData, cs, rnd);

    // 局所探索
    simple_neighborhood_search(pData, cs, rnd);

    if (best_cs.num_Cover < cs.num_Cover)
    {
      best_cs.copy(cs);
    }
  } // End for iter
}



// GRASP初期解＋単純局所探索を niter 回繰り返し
// 引数の cs と best_cs は変更される
// 引数の best_cs に結果が入る
void grasp_neighborhood_search(SCPinstance &pData,
                               int K,
                               int alpha,
                               int niter,
                               SCPsolution &cs,
                               SCPsolution &best_cs,
                               Rand& rnd)
{
  for (int iter = 1; iter <= niter; ++iter)
  {
    // 初期解を生成
    grasp_construction(pData, cs, alpha, rnd);

    // 局所探索
    simple_neighborhood_search(pData, cs, rnd);

    if (best_cs.num_Cover < cs.num_Cover)
    {
      best_cs.copy(cs);
    }
  } // End for iter
}




// メイン関数
int main(int argc, char** argv)
{
  //コマンドライン引数の数が少なければ強制終了
  if (argc < 3){
    cout << "Usage: ./command filename K(int)" << endl;
    return 0;
  }
  char *FileName = argv[1];
  FILE *SourceFile = fopen(FileName,"r");
  int K = atoi(argv[2]);

  // SCPのインスタンスを読み込む
  SCPinstance  pData(SourceFile);

  // initialize
  SCPsolution CS(pData, K);
  SCPsolution Best_CS(pData, K);
  SCPsolution Best_graspCS(pData, K);
  // End Initialize;

  int seed = 0;
  Rand rnd;
  rnd.seed(seed);

  greedy_neighborhood_search(pData, K, niter, CS, Best_CS, rnd);

  // 結果
  printf("Covers %d rows.\n", Best_CS.num_Cover);
  Best_CS.print_solution();


  grasp_neighborhood_search(pData, K, alpha, niter, CS, Best_graspCS, rnd);

  // 結果
  printf("Covers %d rows.\n", Best_graspCS.num_Cover);
  Best_graspCS.print_solution();

  return 0;
}
