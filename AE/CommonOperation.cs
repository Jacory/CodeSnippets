/// <summary>
/// 从shp文件中得到图层
/// </summary>
/// <param name="filePath">文件路径</param>
/// <returns></returns>
public ILayer getLayerFormShpFile(string filefullpath)
{
    ILayer pLayer = null;

    int index = filefullpath.LastIndexOf("\\");
    string filedir = filefullpath.Substring(0, index);
    string filename = filefullpath.Substring(index + 1);

    IWorkspaceFactory pWorkspaceFactory = new ShapefileWorkspaceFactoryClass();
    IWorkspace pWorkspace = pWorkspaceFactory.OpenFromFile(filedir, 0);

    IFeatureWorkspace pFeatureWorkspace = pWorkspace as IFeatureWorkspace;
    IFeatureClass pFeatureClass = pFeatureWorkspace.OpenFeatureClass(filename);
    IFeatureLayer pFeatureLayer = new FeatureLayerClass();
    pFeatureLayer.FeatureClass = pFeatureClass;
    pFeatureLayer.Name = pFeatureClass.AliasName;
    pLayer = pFeatureLayer as ILayer;

    return pLayer;
}

/// <summary>
/// 从*.mdb的文件中得到图层
/// </summary>
/// <param name="mdbfilefullpath">mdb文件路径</param>
/// <returns></returns>
public ILayer getLayerFromMDBFile(string mdbfilefullpath)
{
    ILayer pLayer = null;

    int index = mdbfilefullpath.LastIndexOf("\\");
    string filedir = mdbfilefullpath.Substring(0, index);
    string filename = mdbfilefullpath.Substring(index + 1);

    IWorkspaceFactory pWorkspaceFactory = new AccessWorkspaceFactory();
    IWorkspace pWorkspace = pWorkspaceFactory.OpenFromFile(filedir, 0);

    IFeatureWorkspace pFeatureWorkspace = pWorkspace as IFeatureWorkspace;
    IFeatureClass pFeatureClass = pFeatureWorkspace.OpenFeatureClass(filename);
    IFeatureLayer pFeatureLayer = new FeatureLayerClass();
    pFeatureLayer.FeatureClass = pFeatureClass;
    pFeatureLayer.Name = pFeatureClass.AliasName;
    pLayer = pFeatureLayer as ILayer;

    return pLayer;
}

/// <summary>
/// 从文件中得到图层，支持.shp/.mdb/.gdb中的图层
/// </summary>
/// <param name="filename">图层文件路径</param>
/// <returns>ESRI ILayer对象</returns>
public ILayer getLayerFromFile(string filename)
{
    if (filename == "") return null;

    ILayer pLayer = null;

    int index = filename.LastIndexOf("\\");
    string filedir = filename.Substring(0, index);
    string name = filename.Substring(index + 1);

    string nameExtension = "";
    if (name.Contains('.'))
    {
        string[] temp = name.Split('.');
        nameExtension = temp[temp.Length - 1];
    }

    IWorkspaceFactory pWorkspaceFactory = null;
    if (nameExtension == "") // 使用GDB格式打开
    {
        string[] temp = filedir.Split('.');
        string filedirExtension = temp[temp.Length - 1];
        if (filedirExtension == "mdb")
        {
            pWorkspaceFactory = new AccessWorkspaceFactory();
        }
        else if (filedirExtension.ToLower() == "gdb")
        {
            pWorkspaceFactory = new FileGDBWorkspaceFactory();
        }
    }
    else if(nameExtension == "shp") // 使用shp格式打开
    {
        pWorkspaceFactory = new ShapefileWorkspaceFactory();
    }
    else
    {
        return null;
    }

    IWorkspace pWorkspace = pWorkspaceFactory.OpenFromFile(filedir, 0);

    IFeatureWorkspace pFeatureWorkspace = pWorkspace as IFeatureWorkspace;
    IFeatureClass pFeatureClass = pFeatureWorkspace.OpenFeatureClass(name);
    IFeatureLayer pFeatureLayer = new FeatureLayerClass();
    pFeatureLayer.FeatureClass = pFeatureClass;
    pFeatureLayer.Name = pFeatureClass.AliasName;
    pLayer = pFeatureLayer as ILayer;

    return pLayer;
}

/// <summary>
/// 获取一个图层文件的完整绝对路径
/// </summary>
/// <param name="layer">图层文件</param>
/// <returns>完整绝对路径字符串</returns>
public System.String GetPathForALayer(ESRI.ArcGIS.Carto.ILayer layer)
{
    if (layer == null || !(layer is ESRI.ArcGIS.Geodatabase.IDataset))
    {
        return null;
    }
    ESRI.ArcGIS.Geodatabase.IDataset dataset = (ESRI.ArcGIS.Geodatabase.IDataset)(layer); // Explicit Cast

    if (dataset.Workspace.Type == esriWorkspaceType.esriFileSystemWorkspace)
    {
        return (dataset.Workspace.PathName + "\\" + dataset.Name + ".shp");
    }
    else if (dataset.Workspace.Type == esriWorkspaceType.esriLocalDatabaseWorkspace)
    {
        return (dataset.Workspace.PathName + "\\" + dataset.Name);
    }
    return (dataset.Workspace.PathName + "\\" + dataset.Name);
}_
