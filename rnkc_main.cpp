#include "SCPv.hpp"
//#include "Random.hpp"
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
double alpha = 0.8;


// 列cを解に追加したときのscoreの更新
void add_update_score(SCPinstance& inst,
                      SCPsolution& cs,
                      int c,
                      vector<int>& score)
{
  score[c] = -score[c];
  // スコア更新
  for (int r : inst.ColEntries[c]) // 列cがカバーする行
  {
    cs.COVERED[r]++;

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


// 列cを解に追加したときのscoreの更新
void remove_update_score(SCPinstance& inst,
                         SCPsolution& cs,
                         int c,
                         vector<int>& score)
{
  score[c] = -score[c];

  // スコア更新
  for (int r : inst.ColEntries[c]) // 列cがカバーする行
  {
    cs.COVERED[r]--;

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


// 候補解csに含まれてない中で，スコア最大の列を返す
int get_column_maxscore(SCPinstance& inst,
                        SCPsolution& CS,
                        vector<int>& score,
                        mt19937_64& rnd)
{
  std::vector<int> maxCols;
  int maxScore = 0, maxc = 0;

  for (int c = 0; c < inst.numColumns; c++)
  {
    if (CS.SOLUTION[c]) { continue; }

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
    int j = rnd() % maxCols.size();
    maxc = maxCols[j];
  }

  return maxc;
}


// 候補解csに含まれてない中で，スコア最大の列を返す
int get_column_grasp(SCPinstance& inst,
                     SCPsolution& CS,
                     vector<int>& score,
                     double alpha,
                     mt19937_64& rnd)
{
  int c;
  int maxScore = score[0];
  int minScore = score[0];
  for (int c = 1; c < inst.numColumns; c++)
  {
    if (maxScore < score[c]) maxScore = score[c];
    else if (minScore > score[c]) minScore = score[c];
  }

  std::vector<int> Cols;

  for (int c = 0; c < inst.numColumns; c++)
  {
    if (CS.SOLUTION[c]) { continue; }

    // 最大スコアの列をチェック
    if (score[c] >= minScore + alpha * (maxScore - minScore))
    {
      Cols.push_back(c);
    }
  } // End for c

  int j = rnd() % Cols.size();
  c = Cols[j];

  return c;
}



// 貪欲法：スコア最大の列をK列選ぶ
// 引数の cs に結果が入る
void greedy_construction(SCPinstance& inst,
                         SCPsolution& cs,
                         vector<int>& score,
                         mt19937_64& rnd)
{
  int maxc;

  cs.initialize(inst);
  for (int k = 0; k < cs.K; k++)
  {
    maxc = get_column_maxscore(inst, cs, score, rnd);
    cs.add_column(inst, maxc);
    // スコアの更新
    add_update_score(inst, cs, maxc, score);
  } // End for k
}


// GRASP法：スコアが alpha * (最大値 - 最小値) 以上である列からランダムに一つ選ぶ
SCPsolution grasp_construction(SCPinstance &inst,
                               int K,
                               vector<int>& score,
                               double alpha,
                               mt19937_64& rnd)
{
  SCPsolution cs(inst, K);
  int c;
  for (int k = 0; k < cs.K; k++)
  {
    c = get_column_grasp(inst, cs, score, alpha, rnd);
    cs.add_column(inst, c);
    add_update_score(inst, cs, c, score);
  } // End for k

  return cs;
}


// 配列の順序をランダムに入れ替える
void random_permutation(vector<int>& A,
                        mt19937_64& rnd)
{
  int n = A.size();
  int j;
  for (int i = 0; i < n-1; ++i)
  {
    j = rnd() % (n-i);
    swap(A[i], A[i+j]);
  }
}


// 単純な改善法
// 引数の cs に結果が入る
void simple_neighborhood_search(SCPinstance &inst,
                                SCPsolution &cs,
                                vector<int>& score,
                                mt19937_64& rnd)
{
  int K = cs.K;
  int c1, cov1;
  int c2, cov2;

  vector<int> idx = cs.CS;
  random_permutation(idx, rnd);
  cov1 = cs.num_Cover;

  for (int i = 0; i < K; ++i)
  {
    c1 = idx[i];
    cs.remove_column(inst, c1);

    // 最大スコアの列
    c2 = get_column_maxscore(inst, cs, score, rnd);
    cs.add_column(inst, c2);
    add_update_score(inst, cs, c2, score);
    cov2 = cs.num_Cover;

    if (cov1 > cov2)
    {
      cs.remove_column(inst, c2);
      remove_update_score(inst, cs, c2, score);
      cs.add_column(inst, c1);
      add_update_score(inst, cs, c1, score);
    }
    else
    {
      cov1 = cs.num_Cover;
    }
  } // End for i
}


// GRASP初期解＋単純局所探索を niter 回繰り返し
SCPsolution grasp_neighborhood_search(SCPinstance &inst,
                                      int K,
                                      int alpha,
                                      int niter,
                                      mt19937_64& rnd)
{
  SCPsolution cs(inst, K);
  SCPsolution best_cs(inst, K);
  vector<int> score(inst.numColumns, 0);

  for (int iter = 1; iter <= niter; ++iter)
  {
    // スコアを初期化
    for (int j = 0; j < inst.numColumns; j++)
    {
      score[j] = inst.ColEntries[j].size();
    }

    // 初期解を生成
    cs = grasp_construction(inst, K, score, alpha, rnd);

    // 局所探索
    simple_neighborhood_search(inst, cs, score, rnd);

    if (best_cs.num_Cover < cs.num_Cover)
    {
      best_cs = cs;
    }
  } // End for iter

  return best_cs;
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
  SCPinstance  inst(SourceFile);

  // 候補解
  SCPsolution CS(inst, K);
  SCPsolution Best_CS_glo(inst, K);
  // End Initialize;

  //Rand rnd;
  // シードを固定したいときは以下のコメントをはずす
  //int seed = 0;
  //rnd.seed(seed);

  std::random_device rnd;    // 非決定的な乱数生成器
  std::mt19937_64 mt(rnd()); // メルセンヌ・ツイスタ



  // 何回か繰り返す
  // Best_CS_glo が最良解
  for (int i = 1; i <= 20; i++)
  {
    CS = grasp_neighborhood_search(inst, K, alpha, niter, mt);

    if (Best_CS_glo.num_Cover < CS.num_Cover)
    {
      Best_CS_glo = CS;
    }

    //CS.print_solution();
    // 結果
    printf("%d,%d,%d\n", i, CS.num_Cover, Best_CS_glo.num_Cover);
  }

  return 0;
}
