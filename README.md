# 项目说明
- 这是一个用TTL与非门电路自制CPU项目
- 微指令结构
- 8位数据总线(DBUS)
- 16位地址总线(ABUS)
- 1个8位指令寄存器(RI)
- 1个8位标记寄存器(RF)
- 1个8位指令地址寄存器(RP)
- 1个8位堆栈地址寄存器(RS)
- 1个8位ALU结果寄存器(RT)
- 3个8位高地址寄存器(SC,SD,SS)
- 4个8位通用寄存器(RA,RB,RC,RD)
- 使用Proteus做模拟

# 项目文件
|文件|说明|
|----|----|
|8bit-cpu.dsn|Proteus的模拟|
|doc/alu.png|ALU功能表|
|doc/cpu.png|逻辑架构图|
|doc/mi.xls|微指令详细|
|doc/pcb.txt|将ares转为CAM350步骤|
|mi/make.bat|生成微指令工具批处理文件|
|mi/makefile.nmake|生成微指令工具文件|
|mi/make-mi.exe|生成微指令工具,将mi.txt转化成mi_0-7.bin|
|mi/mi.c|make-mi.exe的源代码|
|mi/mi.txt|微指令源码,由doc/mi.xls复制可得|
|mi/mi_0-7.bin|微指令|
|mi/rom.bin|内存程序|

# 逻辑结构
![cmd-markdown-logo](https://github.com/xt9852/CPU/blob/master/doc/cpu.png?raw=true)

# 物理结构
- 微指令驱动部分
    由AN0-1,AC0-1组成,
    由IN-CLK,SIG-CLK_信号驱动,
    主要是对微指令的当前地址进行操作

- 微指令跳转部分
    由SEL0-5组成,
    由IN-INT0-1,CHK-BI,JL,JNE信号生成的SIG-SEL-A,SIG-SEL-B信号驱动,
    主要是对微指令的下一条地址进行操作,
    通过选择不同的微指令地址以实现微指令跳转

|SIG-SEL-B|SIG-SEL-A|说明|
|----|----|----|
|0|0|选取下一条地址为微指令地址|
|0|1|选取0x[BI]0组成微指令地址|
|1|0|选取中断微指令地址0x005|
|1|1|选取条件跳转微指令地址0x004|

- 微指令存储部分
    由MI0~7组成，
    由AC[]驱动

|微指令|说明|
|----|----|
|CHK-BI|选取指令寄存器做微指令地址|
|CHK-INT|检测是否发生中断|
|CHK-JE|检测是否等于跳转|
|CHK-JNE|检测是否不等于跳转|
|CHK-JB|检测是否大于跳转|
|CHK-JBE|检测是否大于等于跳转|
|CHK-JL|检测是否小于跳转|
|CHK-JLE|检测是否小于等于跳转|
|RA-A|连通A总线,0-输入,1-输出|
|RA-RW|操作:0-读,1写|
|RA-D|连通D总线,0-输入,1-输出|
|ALU-S0|ALU控制S0|
|ALU-S1|ALU控制S1|
|ALU-S2|ALU控制S2|
|ALU-S3|ALU控制S3|
|ALU-M|ALU运算类型:0-算数,1-逻辑|
|ALU-CN|ALU进位|
|MEM-CE|内存选通|
|MEM-RW|内存操作:0-读，1写|
|SIG-ROM|操作ROM|
|SIG-RAM|操作RAM|
|SIG-DEV|操作外设|
|SIG-SEL-A|微代码地址选择A|
|SIG-SEL-B|微代码地址选择B|

- 寄存器与总线部分
    RI,RID,RF,RFA,RFD,RP,RPA,RPD,RS,RSA,RSD,
    RA,RAA,RAD,RB,RBA,RBD,RC,RCA,RCD,RD,RDA,RDD,
    RT,RTD

- ALU与临时寄存器部分
    由ALU0-1，RT组成,
    由ALU-S0-3,M,CN,RT-RW,RT-F信号驱动

- 内存部分
    由ROM,RAM,ROMD,RAMD组成,
    由BA,BD,MEM-CE,RW,OE,WE信号驱动

# 数据流
- 微指令跳转部分=>微指令驱动部分=>微指令存储部分=>寄存器与总线部分,ALU与临时寄存器部分,内存部分

# 指令执行过程
- 清空各寄存器操作
- 检测中断操作
- 取指和IP++操作
- 更新IP操作
- 执行指令操作

# 微代码说明
- [微代码详细](https://github.com/xt9852/CPU/blob/master/mi/mi.xls)

# 所用芯片
|芯片|说明|
|----|----|
|74LS04|非门|
|74LS08|2输入与门|
|74LS32|2输入或门|
|74LS138|3-8译码器|
|74LS153|双4选1选择器|
|74LS165|并入串出|
|74LS181|逻辑运算器|
|74LS245|双向驱动器|
|74LS373|寄存器|
|2732|ROM存储器,存储微指令|
|27256|ROM存储器|
|62256|RAM存储器|

# ALU逻辑
|S0|S1|S2|S3|M|Cn|功能|
|----|----|----|----|----|----|----|
|H|H|L|L|H|*|0|
|L|L|H|H|H|*|0xFF|
|H|H|H|H|H|*|A|
|H|L|H|L|H|*|B|
|L|L|L|L|H|*|A非|
|H|H|L|H|H|*|A与B|
|L|H|H|H|H|*|A或B|
|L|H|H|L|H|*|A异或B|
|H|L|L|H|L|H|A加B|
|L|H|H|L|L|L|A减B|
|L|L|L|L|L|L|A++|
|H|H|H|H|L|H|A--|


