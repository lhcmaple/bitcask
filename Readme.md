# BitCask
### 简介
### 使用指南
### 性能测试
### 未来的改进

* automatic compaction: compaction过程根据文件冗余度q进行compact, 只有当q > QTHRESHOLD || file_size < MIN_FILE_SIZE时, 才进行compact(文件冗余度: q = 已失效的kv条目容量 / 总kv条目容量), 可选择[开启/关闭]

* manual compaction: 手动清除全部失效条目

* 查询缓存: 缓存最近的查询kv对

* 文件句柄缓存: 缓存最近打开的文件句柄