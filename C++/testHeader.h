

// 将Eigen矩阵写入文件，方便检查算法计算的中间结果
#include <Eigen/Dense>
inline printEigenMatToFile( std::string matName, Eigen::MatrixXd &mat，std::string dir )
{
    ofstream out( dir + "\\\\" + matName + ".txt", ios::out );
    if ( !out.is_open() ) { throw std::string( "write file error." ); }
    out << mat ;
    out.close();
}
