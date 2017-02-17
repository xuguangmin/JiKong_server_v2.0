编译说明

1、先进入src目录编译sqlite库。执行aarm.sh或者aax86.sh脚本
2、从doc目录中复制liblua.a到lib目录
2、进入build目录。执行：
   rm -rf *
   cmake ..
   make
