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
SCPsolution::SCPsolution(SCPinstance &pData, int k)
{
  nRow = pData.numRows;
  nCol = pData.numColumns;
  K = k;
  num_Cover = 0;

  for (int i = 0; i < k; i++) CS.push_back(nCol + 1);
  for (int j = 0; j < nCol; ++j) {
    SOLUTION.push_back(0);
    SCORE.push_back(0);
  }
  for (int i = 0; i < nRow; i++) COVERED.push_back(0);

  for (int j = 0; j < nCol; ++j)
    {
      SOLUTION[j] = 0;
      SCORE[j] = pData.ColEntries[j].size(); // スコアの初期値
    }

  for (int i = 0; i < nRow; ++i)
    {
      COVERED[i] = 0;
    }

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
void SCPsolution::initialize(SCPinstance &pData)
{
  num_Cover = 0;

  for (int j = 0; j < nCol; ++j)
    {
      SOLUTION[j] = 0;
      SCORE[j] = pData.ColEntries[j].size(); // スコアの初期値
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


// CSに含まれない列から最大スコアのものを選んで返す
int SCPsolution::get_column_maxscore(SCPinstance &pData, Rand& rnd)
{
  std::vector<int> maxCols;
  int maxScore = 0, maxc = 0;

  for (int c = 0; c < pData.numColumns; c++)
    {
      if (SOLUTION[c]) { continue; }

      // 最大スコアの列をチェック
      if (maxScore < SCORE[c])
	{
	  maxScore = SCORE[c];
	  maxCols.clear();
	  maxCols.push_back(c);
	}
      else if (maxScore == SCORE[c])
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
int SCPsolution::get_column_grasp(SCPinstance &pData, double alpha, Rand& rnd)
{
  std::vector<int> Cols;
  int maxScore = 0, minScore = pData.numRows;

  for (int c = 0; c < pData.numColumns; c++)
    {
      if (SOLUTION[c]) { continue; }

      // 最大スコアと最小スコアを確認
      if (maxScore < SCORE[c]) maxScore = SCORE[c];
      else if (minScore > SCORE[c]) minScore = SCORE[c];
    } // End for c

  for (int c = 0; c < pData.numColumns; c++)
    {
      if (SOLUTION[c]) { continue; }

      // スコアが大きいものをColsへ格納
      if (SCORE[c] >= minScore + alpha * (maxScore - minScore))
	Cols.push_back(c);
    } // End for c

  int j = rnd(0, Cols.size() - 1);
  int col = Cols[j];

  return col;
}


// CSに列cを追加する
void SCPsolution::add_column(SCPinstance &pData, int c)
{
  if (SOLUTION[c])
    {
      printf("Column %d has already contained in CS\n", c);
      exit(1);
    }

  SCORE[c] = -SCORE[c];
  SOLUTION[c] = 1;

  // CSの中がソート済みになるように列cを追加
  int j = 0;
  while (c > CS[j]) j++;
  for (int jj = K-1; jj > j; jj--) CS[jj] = CS[jj - 1];
  CS[j] = c;

  // スコア更新
  for (int r : pData.ColEntries[c]) // 列cがカバーする行
    {
      COVERED[r]++;

      // r行が初めてカバーされたら，rを含む行のスコアを減少
      if (COVERED[r] == 1)
	{
	  num_Cover++;        // カバーされる行の数が増える
	  for (int rc : pData.RowCovers[r])
	    {
	      if (rc != c) SCORE[rc]--;
	    } // End: for ri
	} // End if covered[r] == 1

      // r行が2回カバーされたら，rを含むCSの要素のスコアを増加
      if (COVERED[r] == 2)
	{
	  for (int rc : pData.RowCovers[r]) // r行をカバーする列
	    {
	      if (SOLUTION[rc])
		{
		  SCORE[rc]++;
		  break;
		}
	    }
	} // End if covered[r] == 2
    }
} // End add_column


// CSから列cを削除する
void SCPsolution::remove_column(SCPinstance &pData, int c)
{
  if (SOLUTION[c] == 0)
    {
      printf("Column %d is not contained in CS\n", c);
      exit(1);
    }

  SCORE[c] = -SCORE[c];
  SOLUTION[c] = 0;

  // CSの中がソート済みになるように列cを削除
  int j = 0;
  while (c > CS[j]) j++;
  for (int jj = j; jj < K; ++jj) CS[jj] = CS[jj + 1];
  CS[K-1] = pData.numColumns + 1;

  // スコア更新
  for (int r : pData.ColEntries[c]) // 列cがカバーする行
    {
      COVERED[r]--;

      // r行がカバーされなくなったら，rを含む行のスコアを増加
      if (COVERED[r] == 0)
	{
	  num_Cover--;        // カバーされる行の数が減る
	  for (int rc : pData.RowCovers[r]) // r行をカバーする列
	    {
	      if (rc != c) SCORE[rc]++;
	    } // End: for ri
	} // End if covered[r] == 0

      // r行が1回カバーされたら，rを含むCSの要素のスコアを減少
      if (COVERED[r] == 1)
	{
	  for (int rc : pData.RowCovers[r]) // r行をカバーする列
	    {
	      if (SOLUTION[rc])
		{
		  SCORE[rc]--;
		  break;
		}
	    }
	} // End if covered[r] == 1
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

