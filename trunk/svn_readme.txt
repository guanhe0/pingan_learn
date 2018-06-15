[svn 创建]http://blog.csdn.net/vbirdbest/article/details/51122637
svn 回退某个文件或者文件夹
第一种情况：改动没有被提交（commit）。
这种情况下，使用svn revert就能取消之前的修改。
svn revert [-R] something
其中something可以是（目录或文件的）相对路径也可以是绝对路径。
当something为单个文件时，直接svn revert something就行了；当something为目录时，需要加上参数-R(Recursive,递归)，否则只会将something这个目录的改动。

在这种情况下也可以使用svn update命令来取消对之前的修改，但不建议使用。因为svn update会去连接仓库服务器，耗费时间。

第二种情况：改动已经被提交（commit）。
这种情况下，用svn merge命令来进行回滚。
1、保证我们拿到的是最新代码：
svn update
假设最新版本号是28。
2、然后找出要回滚的确切版本号：
svn log [something]
3、回滚到版本号25：
svn merge -r 28:25 something
为了保险起见，再次确认回滚的结果
svn diff [something]
发现正确无误，提交。
4、提交回滚：
svn commit -m ”Revert revision from r28 to r25,because of …”
提交后版本变成了29。

http://www.runoob.com/svn/svn-show-history.html

svn使用大步骤
[1]同步
[2]合并
[3]提交

