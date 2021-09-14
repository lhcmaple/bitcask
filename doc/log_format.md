日志格式
-----
-----

### **日志文件**

[条目1]
[条目2]
[条目3]
...
[条目N]
[条目结束标志]
[冗余标志]
[冗余标志]
...
[冗余标志]

### **条目**

* log文件

    [FLAG:1] <[checksum:4] [Sequence number:8] [key_length:8] [value_length:8] [key] [value]>

* 备份文件, MANIFEST文件



### **FLAG**

* 0x00: 增加kv
* 0x01: 更改kv
* 0x02: 删除kv
* 0x03: 增加条目
* 0x04: 条目结束标志
* 0x05: 冗余标志
