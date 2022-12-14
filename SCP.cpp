#include "SCP.hpp"
#include <vector>
#include "Random.hpp"

extern Rand rnd;

//
//
//  Class SCPDataset
//
//

// コンストラクタ
SCPinstance::SCPinstance(FILE *SourceFile)
{
  Rows = NULL; Cols = NULL;

  if (SourceFile == NULL) throw DataException();
  else
  {
    int R, C, Cost;
    fscanf(SourceFile, "%d", &R);
    fscanf(SourceFile, "%d", &C);
    numRows = R;
    numColumns = C;

    Rows = new RowData[R + 1];
    for (int i = 0; i <= R; i++)
    {
      Rows[i].numCover = 0;
      Rows[i].Covers  = NULL;
    }

    Cols = new ColData[C + 1];
    for (int j = 0; j <= C; j++)
    {
      Cols[j].numEntry = 0;
      Cols[j].Entries = NULL;
    }

    for(int j = 1; j <= C; j++)
    {
      fscanf(SourceFile, "%d", &Cost);
      Cols[j].Cost = Cost;
    }

    // ファイルから各行の情報を読む
    int CoverNo, CoverID;
    for(int i = 1;  i <= numRows; i++)
    {
      if (fscanf(SourceFile, "%d", &CoverNo) == EOF)
        throw (DataException());

      Rows[i].numCover = CoverNo;
      Rows[i].Covers  = new int[CoverNo + 1];
      for(int j = 1; j <= CoverNo; j++)
      {
        if (fscanf(SourceFile,"%d",&CoverID) == EOF)
          throw (DataException());

        if (CoverID >= 1 && CoverID <= numColumns)
        {
          Rows[i].Covers[j] = CoverID;
          Cols[CoverID].numEntry++;
        }
        else
          throw (DataException());
      }
    }
    // ファイルの読み込み終了

    // 列の情報を作成
    for (int j = 1; j <= numColumns; j++)
    {
      //Allocate memory
      int NoEntry = Cols[j].numEntry;
      if (NoEntry > 0)
      {
        Cols[j].Entries = new int[NoEntry + 1];
      }
    }

    int* Lindex = new int[numColumns + 1]; // 作業用配列
    for (int i = 1; i <= numColumns; i++) Lindex[i] = 0;

    for (int i = 1; i <= numRows; i++)
    {
      for (int j = 1; j <= Rows[i].numCover; j++)
      {
        int C = Rows[i].Covers[j];
        Lindex[C]++;
        Cols[C].Entries[Lindex[C]] = i;
      }
    }
    delete [] Lindex;
    // 列の情報の作成終了
  }
  // 処理は終了


  // 簡単な方法でデータの正しさを確認
  long Sum1 = 0, Sum2 = 0;
  for (int i = 1; i <= numRows; i++) Sum1 += Rows[i].numCover;
  for (int j = 1; j <= numColumns; j++) Sum2 += Cols[j].numEntry;

  //Incorrect Source File!
  if (Sum1 != Sum2)  throw (DataException());

  // 密度の計算
  Density = (float)Sum1/(numColumns * numRows);
}
// End: コンストラクタ


// デストラクタ
SCPinstance::~SCPinstance()
{
  for(int i = 1; i <= numRows; i++)
    if (Rows[i].Covers) delete [] (Rows[i].Covers);
  if (Rows) delete [] Rows;

  for (int j = 1; j <= numColumns; j++)
    if (Cols[j].Entries) delete [] (Cols[j].Entries);
  if (Cols) delete [] Cols;
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

  CS = new int [k + 2];
  SOLUTION = new int [nCol + 1];
  COVERED = new int [nRow + 1];
  SCORE = new int [nCol + 1];

  for (int j = 1; j <= nCol; ++j)
  {
    SOLUTION[j] = 0;
    SCORE[j] = pData.Cols[j].numEntry; // スコアの初期値
  }

  for (int i = 1; i <= nRow; ++i)
  {
    COVERED[i] = 0;
  }

  // cs には最初は大きい値を詰めておく
  for (int j = 0; j <= k; j++)
  {
    CS[j] = nCol + 1;
  }
}


// デストラクタ
SCPsolution::~SCPsolution()
{
  delete [] CS;
  delete [] SOLUTION;
  delete [] COVERED;
  delete [] SCORE;
}

// 候補解を初期化
void SCPsolution::initialize(SCPinstance &pData)
{
  num_Cover = 0;

  for (int j = 1; j <= nCol; ++j)
  {
    SOLUTION[j] = 0;
    SCORE[j] = pData.Cols[j].numEntry; // スコアの初期値
  }

  for (int i = 1; i <= nRow; ++i)
  {
    COVERED[i] = 0;
  }

  // cs には最初は大きい値を詰めておく
  for (int j = 0; j <= K; j++)
  {
    CS[j] = nCol + 1;
  }
}


// CSに含まれない列から最大スコアのものを選んで返す
int SCPsolution::get_column_maxscore(SCPinstance &pData, Rand& rnd)
{
  std::vector<int> maxCols;
  int maxScore = 0, maxc = 0;

  for (int c = 1; c <= pData.numColumns; c++)
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

  for (int c = 1; c <= pData.numColumns; c++)
  {
    if (SOLUTION[c]) { continue; }

    // 最大スコアと最小スコアを確認
    if (maxScore < SCORE[c]) maxScore = SCORE[c];
    else if (minScore > SCORE[c]) minScore = SCORE[c];
  } // End for c

  for (int c = 1; c <= pData.numColumns; c++)
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
  int j = 1;
  while (CS[j] < c) j++;
  for (int jj = K; jj >= j; jj--) CS[jj + 1] = CS[jj];
  CS[j] = c;

  // スコア更新
  for (int i = 1; i <= pData.Cols[c].numEntry; i++)
  {
    int r = pData.Cols[c].Entries[i]; // 列cがカバーする行

    COVERED[r]++;

    // r行が初めてカバーされたら，rを含む行のスコアを減少
    if (COVERED[r] == 1)
    {
      num_Cover++;        // カバーされる行の数が増える
      for (int ri = 1; ri <= pData.Rows[r].numCover; ri++)
      {
        int rc = pData.Rows[r].Covers[ri]; // r行をカバーする列
        if (rc != c) SCORE[rc]--;
      } // End: for ri
    } // End if covered[r] == 1

    // r行が2回カバーされたら，rを含むCSの要素のスコアを増加
    if (COVERED[r] == 2)
    {
      for (int ri = 1; ri <= pData.Rows[r].numCover; ri++)
      {
        int rc = pData.Rows[r].Covers[ri]; // r行をカバーする列
        if (SOLUTION[rc])
        {
          SCORE[rc]++;
          break;
        }
      }
    } // End if covered[r] == 2
  }
}


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
  int j = 1;
  while (CS[j] < c) j++;
  for (int jj = j; jj <= K; ++jj) CS[jj] = CS[jj + 1];
  CS[K] = pData.numColumns + 1;

  // スコア更新
  for (int i = 1; i <= pData.Cols[c].numEntry; i++)
  {
    int r = pData.Cols[c].Entries[i]; // 列cがカバーする行

    COVERED[r]--;

    // r行がカバーされなくなったら，rを含む行のスコアを増加
    if (COVERED[r] == 0)
    {
      num_Cover--;        // カバーされる行の数が減る
      for (int ri = 1; ri <= pData.Rows[r].numCover; ri++)
      {
        int rc = pData.Rows[r].Covers[ri]; // r行をカバーする列
        if (rc != c) SCORE[rc]++;
      } // End: for ri
    } // End if covered[r] == 0

    // r行が1回カバーされたら，rを含むCSの要素のスコアを減少
    if (COVERED[r] == 1)
    {
      for (int ri = 1; ri <= pData.Rows[r].numCover; ri++)
      {
        int rc = pData.Rows[r].Covers[ri]; // r行をカバーする列
        if (SOLUTION[rc])
        {
          SCORE[rc]--;
          break;
        }
      }
    } // End if covered[r] == 1
  }
}


// CSの中身を表示
void SCPsolution::print_solution()
{
  for (int i = 1; i <= K; ++i)
  {
    printf("%d ", CS[i]);
  }
  printf("\n");
}


// 解をコピーする
void SCPsolution::copy(const SCPsolution& cs)
{
  for (int i = 1; i <= K; ++i) CS[i] = cs.CS[i];
  for (int i = 1; i <= nCol; ++i) SOLUTION[i] = cs.SOLUTION[i];
  for (int i = 1; i <= nRow; ++i) COVERED[i] = cs.COVERED[i];
  for (int i = 1; i <= nCol; ++i) SCORE[i] = cs.SCORE[i];
  num_Cover = cs.num_Cover;
}
