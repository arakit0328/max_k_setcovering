#include "SCPv.hpp"
#include "Random.hpp"
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <cassert>
#include <vector>
#include <algorithm>
#include <random>
using namespace std;

// greedy_neighborhood_search で使う繰り返しの回数
int niter = 1000;

// graspで使う alpha の値
double alpha = 0.85;


// CSに含まれない列から最大スコアのものを選んで返す
int get_column_maxscore(SCPinstance &inst,
                        SCPsolution& cs,
                        vector<int>& score,
                        Rand& rnd)
{
  std::vector<int> maxCols;
  int maxScore = 0, maxc = 0;

  for (int c = 0; c < inst.numColumns; c++)
  {
    if (cs.SOLUTION[c]) { continue; }

    // 最大スコアの列をチェック
    if (maxScore < score[c])
    {
      maxScore = score[c];
      maxCols.clear();
      maxCols.push_back(c);
    }
    else if (maxScore == score[c])
      maxCols.push_back(c);
  } // End for c


  if (maxCols.size() == 1) maxc = maxCols[0];
  else
  {
    int j = rnd(0, maxCols.size() - 1);
    maxc = maxCols[j];
  }

  return maxc;
}


// CSに含まれない列から alpha * 最大スコア 以上のものを選んで返す
int get_column_grasp(SCPinstance &inst,
                     double alpha,
                     SCPsolution& cs,
                     vector<int>& score,
                     Rand& rnd)
{
  std::vector<int> Cols;
  int maxScore = 0, minScore = inst.numRows;
  
  for (int c = 0; c < inst.numColumns; c++)
  {
    if (cs.SOLUTION[c]) { continue; }

    // 最大スコアと最小スコアを確認
    if (maxScore < score[c]) maxScore = score[c];
    else if (minScore > score[c]) minScore = score[c];
  } // End for c

  for (int c = 0; c < inst.numColumns; c++)
  {
    if (cs.SOLUTION[c]) { continue; }

    // スコアが大きいものをColsへ格納
    if (score[c] >= minScore + alpha * (maxScore - minScore))
      Cols.push_back(c);
  } // End for c

  int j = rnd(0, Cols.size() - 1);
  int col = Cols[j];

  return col;
}


// Update score when column c is added
void add_update_score(const SCPinstance& inst,
                      const int c,
                      const SCPsolution& cs,
                      vector<int>& score)
{
  score[c] = -score[c];

  // スコア更新
  for (int r : inst.ColEntries[c]) // 列cがカバーする行
  {
    // r行が初めてカバーされたら，rを含む行のスコアを減少
    if (cs.COVERED[r] == 1)
    {
      for (int rc : inst.RowCovers[r])
      {
        if (rc != c) score[rc]--;
      } // End: for ri
    } // End if covered[r] == 1

    // r行が2回カバーされたら，rを含むCSの要素のスコアを増加
    if (cs.COVERED[r] == 2)
    {
      for (int rc : inst.RowCovers[r]) // r行をカバーする列
      {
        if (cs.SOLUTION[rc])
        {
          score[rc]++;
          break;
        }
      }
    } // End if covered[r] == 2
  }
}


// Update score when column c is removed
void remove_update_score(const SCPinstance& inst,
                         const int c,
                         const SCPsolution& cs,
                         vector<int>& score)
{
  score[c] = -score[c];

  // スコア更新
  for (int r : inst.ColEntries[c]) // 列cがカバーする行
  {
    // r行がカバーされなくなったら，rを含む行のスコアを増加
    if (cs.COVERED[r] == 0)
    {
      for (int rc : inst.RowCovers[r]) // r行をカバーする列
      {
        if (rc != c) score[rc]++;
      } // End: for ri
    } // End if covered[r] == 0

    // r行が1回カバーされたら，rを含むCSの要素のスコアを減少
    if (cs.COVERED[r] == 1)
    {
      for (int rc : inst.RowCovers[r]) // r行をカバーする列
      {
        if (cs.SOLUTION[rc])
        {
          score[rc]--;
          break;
        }
      }
    } // End if covered[r] == 1
  }
}


// 貪欲法：スコア最大の列をK列選ぶ
// 引数の cs に結果が入る
void greedy_construction(SCPinstance& inst,
                         SCPsolution& cs,
                         vector<int>& score,
                         Rand &rnd)
{
  int maxc;

  cs.initialize(inst);
  for (int j = 0; j < inst.numColumns; ++j)
    score[j] = inst.ColEntries[j].size(); // スコアの初期値

  for (int k = 0; k < cs.K; k++)
  {
    maxc = get_column_maxscore(inst, cs, score, rnd);
    cs.add_column(inst, maxc);
    add_update_score(inst, maxc, cs, score);
  } // End for k
}


// GRASP法：スコアが alpha * (最大値 - 最小値) 以上である列からランダムに一つ選ぶ
// 引数の cs に結果が入る
void grasp_construction(SCPinstance &inst,
                        SCPsolution &cs,
                        double alpha,
                        vector<int>& score,
                        Rand &rnd)
{
  cs.initialize(inst);
  for (int j = 0; j < inst.numColumns; ++j)
    score[j] = inst.ColEntries[j].size(); // スコアの初期値

  int c;
  for (int k = 0; k < cs.K; k++)
  {
    c = get_column_grasp(inst, alpha, cs, score, rnd);
    cs.add_column(inst, c);
    add_update_score(inst, c, cs, score);
  } // End for k
}


// 配列の順序をランダムに入れ替える
void random_permutation(vector<int>& A, int n, Rand& rnd)
{
  int j;
  for (int i = 0; i < n-1; ++i)
  {
    j = rnd(i, n-1);
    swap(A[i], A[j]);
  }
}


// 単純な改善法
// 引数の cs に結果が入る
void simple_neighborhood_search(SCPinstance& inst,
                                SCPsolution& cs,
                                vector<int>& score,
                                Rand& rnd)
{
  int K = cs.K;
  int c1, cov1;
  int c2, cov2;

  vector<int> idx = cs.CS;
  random_permutation(idx, K, rnd);
  cov1 = cs.num_Cover;

  for (int i = 0; i < K; ++i)
  {
    c1 = idx[i];

    cs.remove_column(inst, c1);
    remove_update_score(inst, c1, cs, score);
    
    // 最大スコアの列
    c2 = get_column_maxscore(inst, cs, score, rnd);
    cs.add_column(inst, c2);

    add_update_score(inst, c2, cs, score);
    cov2 = cs.num_Cover;

    if (cov1 > cov2)
    {
      cs.remove_column(inst, c2);
      remove_update_score(inst, c2, cs, score);

      cs.add_column(inst, c1);
      add_update_score(inst, c1, cs, score);
    }
    else
    {
      cov1 = cs.num_Cover;
    }
  } // End for i
}


// 貪欲初期解＋単純局所探索を niter 回繰り返し
// 引数の cs と best_cs は変更される
// 引数の best_cs に結果が入る
void greedy_neighborhood_search(SCPinstance &inst,
                                int K,
                                int niter,
                                SCPsolution &cs,
                                SCPsolution &best_cs,
                                Rand& rnd)
{
  vector<int> score(inst.numColumns);
  for (int j = 0; j < inst.numColumns; ++j)
  {
    score[j] = inst.ColEntries[j].size(); // スコアの初期値
  }
  
  // 貪欲法
  greedy_construction(inst, cs, score, rnd);
    
  for (int iter = 1; iter <= niter; ++iter)
  {
    // 局所探索
    simple_neighborhood_search(inst, cs, score, rnd);

    if (best_cs.num_Cover < cs.num_Cover)
    {
      best_cs = cs;
    }
  } // End for iter
}



// GRASP初期解＋単純局所探索を niter 回繰り返し
// 引数の cs と best_cs は変更される
// 引数の best_cs に結果が入る
void grasp_neighborhood_search(SCPinstance &inst,
                               int K,
                               int alpha,
                               int niter,
                               SCPsolution &cs,
                               SCPsolution &best_cs,
                               Rand& rnd)
{
  vector<int> score(inst.numColumns);
  for (int j = 0; j < inst.numColumns; ++j)
    score[j] = inst.ColEntries[j].size(); // スコアの初期値

  for (int iter = 1; iter <= niter; ++iter)
  {
    // 初期解を生成
    grasp_construction(inst, cs, alpha, score, rnd);
    
    // 局所探索
    simple_neighborhood_search(inst, cs, score, rnd);

    if (best_cs.num_Cover < cs.num_Cover)
    {
      best_cs = cs;
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
  SCPinstance  instance(SourceFile);

  // initialize
  SCPsolution CS(instance, K);
  //SCPsolution Best_CS(instance, K);
  SCPsolution Best_graspCS(instance, K);
  // End Initialize;

  //int seed = 0;
  Rand rnd;
  //rnd.seed(seed);

  //greedy_neighborhood_search(instance, K, niter, CS, Best_CS, rnd);
  //printf("Covers %d rows.\n", Best_CS.num_Cover);
  //Best_CS.print_solution();

  grasp_neighborhood_search(instance, K, alpha, niter, CS, Best_graspCS, rnd);
  printf("Covers %d rows.\n", Best_graspCS.num_Cover);
  Best_graspCS.print_solution();

  return 0;
}
