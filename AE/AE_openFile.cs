/// <summary>
/// 打开ShapeFile文件
/// </summary>
/// <param name="filePath">文件路径</param>
/// <param name="fileName">文件名</param>
/// <returns>IFeatureClass</returns>
public IFeatureClass GetShpFile(string filePath, string fileName)
{
    IFeatureWorkspace featureWorkspace;
    IFeatureClass featureClass;

    featureWorkspace = GetShapeWorkspace(filePath) as IFeatureWorkspace;

    try
    {
        featureClass = featureWorkspace.OpenFeatureClass(fileName);
    }
    catch
    {
        featureClass = null;
    }

    ReleaseAE.ReleaseAEObject(featureWorkspace);

    return featureClass;
}

/// <summary>
/// 打开Raster文件（如果数据格式为是ESRI GRID，fileName不带后缀，若为tiff则带后缀.tif）
/// </summary>
/// <param name="filePath">文件路径</param>
/// <param name="fileName">文件名</param>
/// <returns>IRaster</returns>
public IRaster GetRasterFile(string filePath, string fileName)
{
    IRasterWorkspace2 rasterWorkspace;
    IRasterDataset rasterDataSet;
    IRasterLayer rasterLayer;
    IRaster raster;

    rasterWorkspace = GetRasterWorkspace(filePath) as IRasterWorkspace2;

    try
    {
        rasterDataSet = rasterWorkspace.OpenRasterDataset(fileName);

        rasterLayer = new RasterLayerClass();
        rasterLayer.CreateFromDataset(rasterDataSet);

        raster = rasterLayer.Raster;

        //ReleaseAE.ReleaseAEObject(rasterDataSet);
        //ReleaseAE.ReleaseAEObject(rasterLayer);
    }
    catch
    {
        raster = null;
    }

    ReleaseAE.ReleaseAEObject(rasterWorkspace);

    return raster;
}

/// <summary>
/// 打开PersonalGeodatabase里的数据要素
/// </summary>
/// <param name="mdbFile">mdb文件（带后缀.mdb）</param>
/// <param name="featureName">要素名</param>
/// <returns></returns>
public IFeatureClass GetPersonalGeodatabase(string mdbFile, string featureName)
{
    IFeatureWorkspace featureWorkspace;
    IFeatureClass featureClass;

    workspaceFactory = new AccessWorkspaceFactoryClass();
    featureWorkspace = workspaceFactory.OpenFromFile(mdbFile, 0) as IFeatureWorkspace;
    featureClass = featureWorkspace.OpenFeatureClass(featureName);

    ReleaseAE.ReleaseAEObject(featureWorkspace);
    ReleaseAE.ReleaseAEObject(workspaceFactory);

    return featureClass;
}

/// <summary>
/// 打开FileGeodatabase里的数据要素
/// </summary>
/// <param name="gdbFile">gdb文件</param>
/// <param name="featureName">要素名</param>
/// <returns></returns>
public IFeatureClass GetFileGeodatabase(string gdbFile, string featureName)
{
    IFeatureWorkspace featureWorkspace;
    IFeatureClass featureClass;

    workspaceFactory = new FileGDBWorkspaceFactoryClass();
    featureWorkspace = workspaceFactory.OpenFromFile(gdbFile, 0) as IFeatureWorkspace;
    featureClass = featureWorkspace.OpenFeatureClass(featureName);

    ReleaseAE.ReleaseAEObject(featureWorkspace);
    ReleaseAE.ReleaseAEObject(workspaceFactory);

    return featureClass;
}

/// <summary>
/// 打开TIN文件
/// </summary>
/// <param name="tinFile">文件路径</param>
/// <param name="fileName">文件名</param>
/// <returns>Tin</returns>
public ITin GetTinFile(string tinFile, string fileName)
{
    ITinWorkspace pTinWorkspace;
    ITin pTin;
    workspaceFactory = new TinWorkspaceFactoryClass();
    pTinWorkspace = workspaceFactory.OpenFromFile(tinFile, 0) as ITinWorkspace;
    pTin = pTinWorkspace.OpenTin(fileName);

    ReleaseAE.ReleaseAEObject(pTinWorkspace);
    ReleaseAE.ReleaseAEObject(workspaceFactory);

    return pTin;
}

/// <summary>
/// 打开CAD文件（featurename要带扩展名）
/// </summary>
/// <param name="cadFile">文件路径</param>
/// <param name="featureName">文件名</param>
/// <returns>featureClass</returns>
public IFeatureClass GetCADFile(string cadFile, string featureName)
{
   IFeatureWorkspace featureWorkspace;
   IFeatureClass featureClass;

   workspaceFactory = new CadWorkspaceFactoryClass();
   featureWorkspace = workspaceFactory.OpenFromFile(cadFile, 0) as IFeatureWorkspace;
   featureClass = featureWorkspace.OpenFeatureClass(featureName);
   return featureClass;
}

/// <summary>
/// 打开数据表格
/// </summary>
/// <param name="connectionString">连接数据库字符串</param>
/// <param name="tableName">表名</param>
/// <returns>table</returns>
public ITable GetRDBMS(string connectionString, string tableName)
{
    IFeatureWorkspace featureWorkspace;

    workspaceFactory = new OLEDBWorkspaceFactoryClass();
    IPropertySet pPropset = new PropertySetClass();
    ITable pTable;
    pPropset.SetProperties("CONNECTSTRING", connectionString);
    featureWorkspace = workspaceFactory.Open(pPropset,0) as IFeatureWorkspace;
    pTable = featureWorkspace.OpenTable(tableName);


    ReleaseAE.ReleaseAEObject(featureWorkspace);
    ReleaseAE.ReleaseAEObject(pPropset);

    return pTable;
}

/// <summary>
/// 创建内存工作空间Name
/// </summary>
/// <param name="workspacename">工作空间名称</param>
/// <returns>工作空间Name</returns>
public IWorkspaceName CreatMemoryWorkspaceName(string workspacename)
{
    IWorkspaceName workspaceName;

    workspaceFactory = new InMemoryWorkspaceFactoryClass();

    workspaceName = workspaceFactory.Create("d:\\", workspacename, null, 0);

    ReleaseAE.ReleaseAEObject(workspaceFactory);

    return workspaceName;
}

/// <summary>
/// 创建内存工作空间
/// </summary>
/// <param name="workspacename">工作空间名称</param>
/// <returns>工作空间类型IWorkspace </returns>
public IWorkspace CreatMemoryWorkspace(string workspacename)
{
    IWorkspaceName workspaceName;
    IName name;
    IWorkspace workspace;

    workspaceName = CreatMemoryWorkspaceName(workspacename);

    name = workspaceName as IName;
    name.NameString = workspacename;
    workspace = name.Open() as IWorkspace;

    ReleaseAE.ReleaseAEObject(workspaceName);
    ReleaseAE.ReleaseAEObject(name);

    return workspace;
}

/// <summary>
/// 创建本地磁盘矢量工作空间
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
public IWorkspace GetShapeWorkspace(string filePath)
{
    IWorkspace workspace;

    workspaceFactory = new ShapefileWorkspaceFactoryClass();

    workspace = workspaceFactory.OpenFromFile(filePath, 0);

    ReleaseAE.ReleaseAEObject(workspaceFactory);
    workspaceFactory = null;

    return workspace;
}

public IWorkspaceName GetShapeWorkspaceName(string spaceName)
{
    IWorkspaceName workspaceName;

    workspaceFactory = new ShapefileWorkspaceFactoryClass();

    workspaceName = workspaceFactory.Create(Path.GetTempPath(), spaceName, null, 0);

    ReleaseAE.ReleaseAEObject(workspaceFactory);

    return workspaceName;
}

/// <summary>
/// 创建本地磁盘栅格工作空间
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
public IWorkspace GetRasterWorkspace(string filePath)
{
    IWorkspace workspace;

    workspaceFactory = new RasterWorkspaceFactoryClass();
    workspace = workspaceFactory.OpenFromFile(filePath, 0);

    ReleaseAE.ReleaseAEObject(workspaceFactory);

    return workspace;
}
