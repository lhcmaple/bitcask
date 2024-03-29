# BitCask
### 简介
### 使用指南
### 性能测试
### 未来的改进

* automatic compaction: compaction过程根据文件冗余度q进行compact, 只有当q > QTHRESHOLD || file_size < MIN_FILE_SIZE时, 才进行compact(文件冗余度: q = 已失效的kv条目容量 / 总kv条目容量), 可选择[开启/关闭]

* 查询缓存[遗弃]: 缓存最近的查询kv对

* 文件句柄缓存[完成]: 缓存最近打开的文件句柄

* 更改判定hindex文件的记录是否为删除标志的策略[完成]: 采用判断记录的value的长度是否为零的方法来判定是记录是否为删除标志的策略[完成]

### 优点

* 一次put操作只写一次日志(哈希索引写入文件耗时被均摊给同一文件的所有put操作, 一个日志文件只对应一次哈希索引写入)
* 一次get操作至多一次磁盘读操作(缺失数据不触发磁盘读)
* 一次del操作至多写入一次日志(缺失数据不触发磁盘写, 即不会进行重复删除操作)
* 可以通过manual compaction清除所有失效数据(空闲时进行)
* 可以开启automatic compaction进行自动清除冗余度过大的文件中的失效条目

### 缺点

* 内存中哈希表大小与总数据是线性关系
* 无序性
* 写放大, 同一数据如果一直未被修改, 可能会被重复写入磁盘中
* 迭代器遍历数据库时可能需要一直打开文件(可能可以通过直接遍历文件来避免?)

### 开发中遇到的问题
* 并发控制的问题, 尤其是多个类之间协同时
* 原来的代码流程在增加功能时会出现一些冲突的情况
    比如在autocompaction时, 删除这一操作的原流程会与autocompaction不太适配
* 代码大小到达一定大小时, 功能的扩展往往只能缩手缩脚地添加到原代码中
