/// <summary>
/// 利用雅格比(Jacobi)方法求实对称矩阵的全部特征值及特征向量.
/// </summary>
/// <param name="a">长度为n*n的数组，存放实对称矩阵，返回时对角线存放n个特征值.</param>
/// <param name="n">矩阵的阶数.</param>
/// <param name="v">长度为n*n的数组，返回特征向量(按列存储).</param>
/// <param name="eps">控制精度要求.</param>
/// <param name="jt">整型变量，控制最大迭代次数.</param>
/// <returns>返回false表示超过迭代jt次仍未达到精度要求，返回true表示正常返回.</returns>
bool PcaFusion::eejcb( double a[], int n, double v[], double eps, int jt )
{
    int i, j, p, q, u, w, t, s, l;
    double fm, cn, sn, omega, x, y, d;

    l = 1;
    //初始化特征向量矩阵使其全为0
    for( i = 0; i <= n - 1; i++ )
    {
        v[i * n + i] = 1.0;
        for( j = 0; j <= n - 1; j++ )
        {
            if( i != j )
            {
                v[i * n + j] = 0.0;
            }
        }
    }

    while( true ) //循环
    {
        fm = 0.0;
        for( i = 0; i <= n - 1; i++ )   // 出, 矩阵a( 特征值 ), 中除对角线外其他元素的最大绝对值
        {
            //这个最大值是位于a[p][q] ,等于fm
            for( j = 0; j <= n - 1; j++ )
            {
                d = fabs( a[i * n + j] );

                if( ( i != j ) && ( d > fm ) )
                {
                    fm = d;
                    p = i;
                    q = j;
                }
            }
        }

        if( fm < eps )   //精度复合要求
        {
            return true;    //正常返回
        }

        if( l > jt )     //迭代次数太多
        {
            return false;    //失败返回
        }

        l ++;       //   迭代计数器
        u = p * n + q;
        w = p * n + p;
        t = q * n + p;
        s = q * n + q;
        x = -a[u];
        y = ( a[s] - a[w] ) / 2.0;		//x y的求法不同
        omega = x / sqrt( x * x + y * y );	//sin2θ

        //tan2θ=x/y = -2.0*a[u]/(a[s]-a[w])
        if( y < 0.0 )
        {
            omega = -omega;
        }

        sn = 1.0 + sqrt( 1.0 - omega * omega );
        sn = omega / sqrt( 2.0 * sn );		//sinθ
        cn = sqrt( 1.0 - sn * sn );			//cosθ

        fm = a[w];   //   变换前的a[w]   a[p][p]
        a[w] = fm * cn * cn + a[s] * sn * sn + a[u] * omega;
        a[s] = fm * sn * sn + a[s] * cn * cn - a[u] * omega;
        a[u] = 0.0;
        a[t] = 0.0;

        //   以下是旋转矩阵,旋转了了p行,q行,p列,q列
        //   但是四个特殊点没有旋转(这四个点在上述语句中发生了变化)
        //   其他不在这些行和列的点也没变
        //   旋转矩阵,旋转p行和q行
        for( j = 0; j <= n - 1; j++ )
        {
            if( ( j != p ) && ( j != q ) )
            {
                u = p * n + j;
                w = q * n + j;
                fm = a[u];
                a[u] = a[w] * sn + fm * cn;
                a[w] = a[w] * cn - fm * sn;
            }
        }

        //旋转矩阵,旋转p列和q列
        for( i = 0; i <= n - 1; i++ )
        {
            if( ( i != p ) && ( i != q ) )
            {
                u = i * n + p;
                w = i * n + q;
                fm = a[u];
                a[u] = a[w] * sn + fm * cn;
                a[w] = a[w] * cn - fm * sn;
            }
        }

        //记录旋转矩阵特征向量
        for( i = 0; i <= n - 1; i++ )
        {
            u = i * n + p;
            w = i * n + q;
            fm = v[u];
            v[u] = v[w] * sn + fm * cn;
            v[w] = v[w] * cn - fm * sn;
        }
    }

    return true;
}

/// Description:
/// 求图像自相关矩阵
/// param: 图像数据集
/// Return: 自相关矩阵指针数据
double* calCorrMatrix( GDALDataset* dataset )
{
  double *corrMat = new double[bandCount * bandCount];
  int index = 0;

  GDALDataType eDT = dataset->GetRasterBand( 1 )->GetRasterDataType();

  // initialize sum array
  double** sum = new double*[this->bandCount];
  for ( int i = 0; i < bandCount; i++ )
  {
      sum[i] = new double[this->bandCount];
      for ( int j = 0; j < bandCount; j++ )
      {
          sum[i][j] = 0.0;
      }
  }

  int lineCount = 4;
  if ( width > 1000 ) { lineCount = 50; }
  if ( width > 10000 ) { lineCount = 5 ; }
  int currentLineCount = lineCount;
  int tileCount = height / lineCount;

  int numY;
  for ( numY = 0; numY < tileCount + 1; numY++ )
  {
      // deal with last piece of tile
      if ( numY == tileCount )
      {
          currentLineCount = height % lineCount;
          if ( currentLineCount == 0 ) { break; }
      }

      // read tile data out
      float** data = new float*[bandCount];
      for ( int band = 0; band < bandCount; band++ )
      {
          int bandList = {band + 1};
          data[band] = new float[currentLineCount * width];
          dataset->RasterIO( GF_Read, 0, numY * lineCount, width, currentLineCount, data[band], width,
                             currentLineCount, GDT_Float32, 1, &bandList, 0, 0, 0 );
          int noDataValue;
          GDALGetRasterNoDataValue( inputDataset->GetRasterBand( bandList ), &noDataValue );
          for ( int j = 0; j < this->width * currentLineCount; j++ )
          {
              if( data[band][j] == noDataValue ||
                      !_finite( data[band][j] ) )
              { data[band][j] = 0; }
          }
      }

      // calculate sum
      for ( int band1 = 0; band1 < bandCount; band1++ )
      {
          for ( int band2 = 0; band2 < bandCount; band2++ )
          {
              for ( int i = 0; i < currentLineCount * width; i++ )
              {
                  sum[band1][band2] += ( data[band1][i] ) * ( data[band2][i] );
              }
          }
      }

      for ( int i = 0; i < bandCount; i++ )
      {
          delete[] data[i];
      }
      delete[] data;

  }

  for ( int i = 0; i < bandCount; i++ )
  {
      for ( int j = 0; j < bandCount; j++ )
      {
          corrMat[i * bandCount + j] = sum[i][j] * 1.0 / ( width * height );
      }
  }
  return corrMat;
}

/// Description:
/// 矩阵求逆，采用主要特征值所对应的特征向量求得
/// param: 对称方阵指针数据
/// Return: Eigen数组逆矩阵
Eigen::MatrixXd eigenInverse( double* matrix )
{
  Map<MatrixXd> covMat( matrix, bandCount, bandCount );
  Eigen::SelfAdjointEigenSolver<MatrixXd> es;
  es.compute( covMat );
  VectorXd eva = es.eigenvalues().real();
  MatrixXd eigenVectorMat = es.eigenvectors().real();

  double rate = 0.9999;
  double sumVal = rate * eva.sum();
  int index = bandCount;
  double countVal = 0;
  while( countVal < sumVal )
  {
      index = index - 1;
      if ( index < 0 ) { throw std::string( "something wrong..." ); }
      countVal += eva( index );
  }
  MatrixXd eigenvalueMat = eva.asDiagonal();
  writeMatOut2( "eigenvalue", eigenvalueMat );
  MatrixXd vc = eigenVectorMat.rightCols( bandCount - index );
  MatrixXd va = eigenvalueMat.bottomRightCorner( bandCount - index, bandCount - index );
  MatrixXd Cinv = vc * va.inverse() * vc.transpose();
  matrix = Cinv.data();
  return Cinv;
}
