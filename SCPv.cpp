#include "SCPv.hpp"
#include <vector>
#include <iostream>
#include "Random.hpp"

extern Rand rnd;

//
//
//  Class SCPinstance
//
//

// コンストラクタ
SCPinstance::SCPinstance(FILE *SourceFile)
{
  if (SourceFile == NULL) throw DataException();
  else
  {
    int R, C, cost;
    fscanf(SourceFile, "%d", &R);
    fscanf(SourceFile, "%d", &C);
    numRows = R;
    numColumns = C;

    int* nCov = new int [numColumns];
    int* idx = new int [numColumns];
    for (int j = 0; j < numColumns; j++) {
      nCov[j] = 0;
      idx[j] = 0;
    }


    // read costs
    for(int j = 0; j < C; j++)
    {
      fscanf(SourceFile, "%d", &cost);
      Cost.push_back(cost);
    }

    // ファイルから各行の情報を読む
    int CoverNo, CoverID;
    std::vector<int> cov;

    for(int i = 0;  i < numRows; i++)
    {
      if (fscanf(SourceFile, "%d", &CoverNo) == EOF)
        throw (DataException());

      cov.clear();
      for(int j = 0; j < CoverNo; j++)
      {
        if (fscanf(SourceFile,"%d", &CoverID) == EOF)
          throw (DataException());

        if (CoverID >= 1 && CoverID <= numColumns)
        {
          cov.push_back(CoverID - 1);
          nCov[CoverID - 1]++;
        }
        else
          throw (DataException());
      }

      RowCovers.push_back(cov);
    }
    // ファイルの読み込み終了

    // 列の情報を作成
    for (int j = 0; j < numColumns; ++j) {
      std::vector<int> c(nCov[j]);
      ColEntries.push_back(c);
    }

    for (int i = 0; i < numRows; i++)
    {
      for (int c : RowCovers[i]) {
        ColEntries[c][idx[c]] = i;
        idx[c]++;
      }
    }
    // 列の情報の作成終了
    delete [] nCov;
    delete [] idx;
  }
  // 処理は終了


  // 簡単な方法でデータの正しさを確認
  long Sum1 = 0, Sum2 = 0;
  for (int i = 0; i < numRows; i++) Sum1 += RowCovers[i].size();
  for (int j = 0; j < numColumns; j++) Sum2 += ColEntries[j].size();

  //Incorrect Source File!
  if (Sum1 != Sum2)  throw (DataException());

  // 密度の計算
  Density = (float)Sum1/(numColumns * numRows);
}
// End: コンストラクタ


// デストラクタ
SCPinstance::~SCPinstance()
{
}
// End: デストラクタ



//
//
// Class SCPsolution
//
//

// コンストラクタ
SCPsolution::SCPsolution(SCPinstance &inst, int k)
{
  nRow = inst.numRows;
  nCol = inst.numColumns;
  K = k;
  num_Cover = 0;

  for (int i = 0; i < k; i++) CS.push_back(nCol + 1);
  for (int j = 0; j < nCol; ++j) {
    SOLUTION.push_back(0);
  }
  for (int i = 0; i < nRow; i++) COVERED.push_back(0);

  // cs には最初は大きい値を詰めておく
  for (int j = 0; j < k; j++)
  {
    CS[j] = nCol + 1;
  }
}


// デストラクタ
SCPsolution::~SCPsolution()
{
}

// 候補解を初期化
void SCPsolution::initialize(SCPinstance &inst)
{
  num_Cover = 0;

  for (int j = 0; j < nCol; ++j)
  {
    SOLUTION[j] = 0;
  }

  for (int i = 0; i < nRow; ++i)
  {
    COVERED[i] = 0;
  }

  // cs には最初は大きい値を詰めておく
  for (int j = 0; j < K; j++)
  {
    CS[j] = nCol + 1;
  }
}


// CSに列cを追加する
void SCPsolution::add_column(SCPinstance &inst,
                             int c)
{
  if (SOLUTION[c])
  {
    printf("Column %d has already contained in CS\n", c);
    exit(1);
  }

  SOLUTION[c] = 1;

  // CSの中がソート済みになるように列cを追加
  int j = 0;
  while (c > CS[j]) j++;
  for (int jj = K-1; jj > j; jj--) CS[jj] = CS[jj - 1];
  CS[j] = c;

  // スコア更新
  for (int r : inst.ColEntries[c]) // 列cがカバーする行
  {
    COVERED[r]++;

    // r行が初めてカバーされたら，rを含む行のスコアを減少
    if (COVERED[r] == 1)
    {
      num_Cover++;        // カバーされる行の数が増える
    } // End if covered[r] == 1
  }
} // End add_column



// CSから列cを削除する
void SCPsolution::remove_column(SCPinstance &inst, int c)
{
  if (SOLUTION[c] == 0)
  {
    printf("Column %d is not contained in CS\n", c);
    exit(1);
  }

  SOLUTION[c] = 0;

  // CSの中がソート済みになるように列cを削除
  int j = 0;
  while (c > CS[j]) j++;
  for (int jj = j; jj < K; ++jj) CS[jj] = CS[jj + 1];
  CS[K-1] = inst.numColumns + 1;

  for (int r : inst.ColEntries[c]) // 列cがカバーする行
  {
    COVERED[r]--;

    // r行がカバーされなくなったら，rを含む行のスコアを増加
    if (COVERED[r] == 0)
    {
      num_Cover--;        // カバーされる行の数が減る
    } // End if covered[r] == 0
  }
} // End remove_column


// CSの中身を表示
void SCPsolution::print_solution()
{
  for (int i = 0; i < K - 1; ++i)
  {
    printf("%d ", CS[i] + 1);
  }
  printf("\n");
} // End print_solution
