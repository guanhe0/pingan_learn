D:\Users\Administrator\Documents\visual studio 2010\Projects\vs2010\trunk\MyDll\Debug
D:\Users\Administrator\Documents\visual studio 2010\Projects\vs2010\trunk\MyDll\lib
D:\Users\Administrator\Documents\visual studio 2010\Projects\vs2010\trunk\MyDll\MyDll\Debug

D:\Users\Administrator\Documents\visual studio 2010\Projects\vs2010\trunk\MyDll\Release .dll .lib生成目录

配置错误通常情况是，项目属性->链接器->常规->输出文件 里配置好了输出目录和文件名，但项目属性->常规里的输出目录和目标文件名用的默认值。
正确的做法是：
项目属性->常规->输出目录配置对应的目录
项目属性->常规->目标文件名配置对应的目标文件名
项目属性->链接器->常规->输出文件里配置$(OutDir)$(TargetName)$(TargetExt)

E:\XunLeiDownLoad\CH-HCNetSDK(Windows64)\Demo\MFC_FENGONGNENG\preview&capture&ptz\bin 【HCNetSDK.dll】