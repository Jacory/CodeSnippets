/// <summary>
/// 缓冲区分析
/// </summary>
/// <param name="layer">图层</param>
/// <param name="bufferDistance">缓冲区半径</param>
/// <param name="outputPath">输出文件路径</param>
/// <returns>执行成功返回true</returns>
public bool bufferAnalysis(string inlayerPath, double bufferDistance, string unitStr, string outputPath)
{
    ILayer layer = getLayerFromFile(inlayerPath);
    if (!layer.Valid) return false;
    if (outputPath == "" || outputPath == null) return false;

    Geoprocessor gp = new Geoprocessor();
    gp.OverwriteOutput = true;
    try
    {
        ESRI.ArcGIS.AnalysisTools.Buffer buffer = new ESRI.ArcGIS.AnalysisTools.Buffer(
            layer, outputPath, bufferDistance.ToString() + " " + unitStr);

        IGeoProcessorResult results = (IGeoProcessorResult)gp.Execute(buffer, null);
        if (results.Status != esriJobStatus.esriJobSucceeded) return false;
    }
    catch (Exception ex)
    {
        // 输出GP工具信息，for debug
        System.Text.StringBuilder sb = new System.Text.StringBuilder();
        for (int i = 0; i < gp.MessageCount; i++)
        {
            sb.AppendLine(gp.GetMessage(i));
        }
        if (sb.Capacity > 0) System.Windows.Forms.MessageBox.Show(sb.ToString(), "GP Messages");
    }

    return true;
}

/// <summary>
/// 交集取反
/// </summary>
/// <param name="layer1">图层1</param>
/// <param name="layer2">图层2</param>
/// <param name="outputPath">输出结果图层路径</param>
/// <param name="tolerance">容差</param>
/// <param name="unitStr">容差单位</param>
/// <returns>执行成功返回true</returns>
public bool symDiff(string layer1_path, string layer2_path, string outputPath, double tolerance, string unitStr)
{
    ILayer layer1 = getLayerFromFile(layer1_path);
    ILayer layer2 = getLayerFromFile(layer2_path);

    if (!layer1.Valid || !layer2.Valid) return false;
    if (outputPath == "" || outputPath == null ||
        !System.IO.Directory.Exists(System.IO.Path.GetDirectoryName(outputPath)))
        return false;

    Geoprocessor gp = new Geoprocessor();
    gp.OverwriteOutput = true;
    try
    {
        ESRI.ArcGIS.AnalysisTools.SymDiff symdiff = new ESRI.ArcGIS.AnalysisTools.SymDiff(
            layer1, layer2, outputPath);

        symdiff.join_attributes = "ALL";
        if (tolerance != 0 && unitStr != "")
            symdiff.cluster_tolerance = tolerance.ToString() + " " + unitStr;

        IGeoProcessorResult results = (IGeoProcessorResult)gp.Execute(symdiff, null);
        if (results.Status != esriJobStatus.esriJobSucceeded) return false;
    }
    catch (Exception ex)
    {
        // 输出GP工具信息，for debug
        System.Text.StringBuilder sb = new System.Text.StringBuilder();
        for (int i = 0; i < gp.MessageCount; i++)
        {
            sb.AppendLine(gp.GetMessage(i));
        }
        if (sb.Capacity > 0) System.Windows.Forms.MessageBox.Show(sb.ToString(), "GP Messages");
        return false;
    }

    return true;
}

/// <summary>
/// 裁剪要素图层
/// </summary>
/// <param name="inLayer">待裁剪图层</param>
/// <param name="clipLayer">用于裁剪的图层</param>
/// <param name="outputPath">结果文件路径</param>
/// <param name="tolerance">容差（可选）</param>
/// <param name="unitStr">容差单位字符串</param>
/// <returns>执行成功返回true</returns>
public bool clip(string inLayer_path, string clipLayer_path, string outputPath, double tolerance, string unitStr)
{
    ILayer inLayer = getLayerFromFile(inLayer_path);
    ILayer clipLayer = getLayerFromFile(clipLayer_path);
    // 判断输入参数的合法性
    if (!inLayer.Valid || !clipLayer.Valid) return false;

    // 新建GP工具进行裁剪
    Geoprocessor gp = new Geoprocessor();
    try
    {
        gp.OverwriteOutput = true;
        ESRI.ArcGIS.AnalysisTools.Clip clip = new ESRI.ArcGIS.AnalysisTools.Clip(inLayer, clipLayer, outputPath);
        clip.cluster_tolerance = tolerance.ToString() + " " + unitStr;
        IGeoProcessorResult results = (IGeoProcessorResult)gp.Execute(clip, null);
        if (results.Status != esriJobStatus.esriJobSucceeded) return false;
    }
    catch (Exception ex)
    {
        // 输出GP工具信息，for debug
        System.Text.StringBuilder sb = new System.Text.StringBuilder();
        for (int i = 0; i < gp.MessageCount; i++)
        {
            sb.AppendLine(gp.GetMessage(i));
        }
        if (sb.Capacity > 0) System.Windows.Forms.MessageBox.Show(sb.ToString(), "GP Messages");
        return false;
    }
    return true;
}

/// <summary>
/// 合并多个图层
/// </summary>
/// <param name="inputLayerList">输入待合并图层,路径中不能包含多余的‘.’符号</param>
/// <param name="outputpath">输出图层路径</param>
/// <returns><c>true</c>执行成功, <c>false</c> 执行失败</returns>
public bool merge(List<string> inputLayerList, string outputpath)
{
    // parse input layer list
    string inputStr = "";
    for (int i = 0; i < inputLayerList.Count - 1;i++ )
    {
        inputStr += inputLayerList[i] + ";";
    }
    inputStr += inputLayerList[inputLayerList.Count-1];

    Geoprocessor gp = new Geoprocessor();
    gp.OverwriteOutput = true;
    try
    {
        ESRI.ArcGIS.DataManagementTools.Merge merge = new ESRI.ArcGIS.DataManagementTools.Merge();
        merge.inputs = inputStr;
        merge.output = outputpath;

        IGeoProcessorResult results = (IGeoProcessorResult)gp.Execute(merge, null);
        if (results.Status != esriJobStatus.esriJobSucceeded) return false;
    }
    catch (Exception ex)
    {
        // 输出GP工具信息，for debug
        System.Text.StringBuilder sb = new System.Text.StringBuilder();
        for (int i = 0; i < gp.MessageCount; i++)
        {
            sb.AppendLine(gp.GetMessage(i));
        }
        if (sb.Capacity > 0) System.Windows.Forms.MessageBox.Show(sb.ToString(), "GP Messages");
        return false;
    }
    return true;
}

/// <summary>
/// 点要素转线要素
/// </summary>
/// <param name="inputPath">点文件路径</param>
/// <param name="outputPath">输出线文件路径</param>
/// <param name="lineField">输出中的各个要素都将基于“线字段”中的唯一值</param>
/// <param name="sortField">点连接的排序字段</param>
/// <returns></returns>
public bool pointsToLine(string inputPath, string outputPath, string lineField, string sortField)
{
    Geoprocessor gp = new Geoprocessor();
    gp.OverwriteOutput = true;
    try
    {
        ESRI.ArcGIS.DataManagementTools.PointsToLine point2line = new ESRI.ArcGIS.DataManagementTools.PointsToLine();
        point2line.Input_Features = inputPath;
        point2line.Output_Feature_Class = outputPath;
        if (lineField != null && lineField != "") { point2line.Line_Field = lineField; }
        if (sortField != null && sortField != "") { point2line.Sort_Field = sortField; }
        point2line.Close_Line = "NO_CLOSE";

        IGeoProcessorResult result = gp.Execute(point2line, null) as IGeoProcessorResult;
        if (result.Status != esriJobStatus.esriJobSucceeded) return false;
    }
    catch (System.Exception ex)
    {
        System.Text.StringBuilder sb = new System.Text.StringBuilder();
        for (int i = 0; i < gp.MessageCount;i++ )
        {
            sb.AppendLine(gp.GetMessage(i));
        }
        if (sb.Capacity > 0) System.Windows.Forms.MessageBox.Show(ex.ToString() + "\n"+sb.ToString(), "GP Messages");
        return false;
    }
    return true;
}
