#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <BasicExcel.hpp>
using namespace YExcel;

// 寄存器ID,与REG_NAME对应
enum REG_ID {
    REG_READ,
    REG_WRITE,
    REG_RI,
    REG_RF,
    REG_SC,
    REG_SD,
    REG_SS,
    REG_RP,
    REG_RS,
    REG_RA,
    REG_RB,
    REG_RC,
    REG_RD,
    REG_RT,
    REG_NOT_RT,
    REG_NOT_STACK,
    REG_MEM,
    BUS_ADDR,
    BUS_DATA,
    ALU_00,
    ALU_FF,
    ALU_A_ADD_ADD,
    ALU_A_SUB_SUB,
    ALU_A_SUB_D,
    ADDR_SEL,
    REG_COUNT
    };

// 寄存器名称,与REG_ID对应
const char *REG_NAME[] = {
    "READ",
    "WRITE",
    "RI",
    "RF",
    "SC",
    "SD",
    "SS",
    "RP",
    "RS",
    "RA",
    "RB",
    "RC",
    "RD",
    "RT",
    "NOT-RT",
    "NOT-STACK",
    "MEM",
    "ABUS",
    "DBUS",
    "ALU(00)",
    "ALU(FF)",
    "ALU(A++)",
    "ALU(A--)",
    "ALU(A-D)",
    "ADDR-SEL"
    };

#pragma pack(push)  // 将当前pack设置压栈保存
#pragma pack(1)     // 必须在结构体定义之前使用

typedef struct _mi
{
    unsigned short next:        12;

    unsigned short select_ri:   1;
    unsigned short check_int:   1;
    unsigned short check_je:    1;
    unsigned short check_jne:   1;
    unsigned char  check_jb:    1;
    unsigned char  check_jbe:   1;
    unsigned char  check_jl:    1;
    unsigned char  check_jle:   1;

    unsigned char  ri_oe:       1;
    unsigned char  ri_le:       1;

    unsigned char  rf_a:        1;
    unsigned char  rf_rw:       1;
    unsigned char  rf_d:        1;

    unsigned char  sc_a:        1;
    unsigned char  sc_rw:       1;
    unsigned char  sc_d:        1;

    unsigned char  sd_a:        1;
    unsigned char  sd_rw:       1;
    unsigned char  sd_d:        1;

    unsigned char  ss_a:        1;
    unsigned char  ss_rw:       1;
    unsigned char  ss_d:        1;

    unsigned char  rp_a:        1;
    unsigned char  rp_rw:       1;
    unsigned char  rp_d:        1;

    unsigned char  rs_a:        1;
    unsigned char  rs_rw:       1;
    unsigned char  rs_d:        1;

    unsigned char  ra_a:        1;
    unsigned char  ra_rw:       1;
    unsigned char  ra_d:        1;

    unsigned char  rb_a:        1;
    unsigned char  rb_rw:       1;
    unsigned char  rb_d:        1;

    unsigned char  rc_a:        1;
    unsigned char  rc_rw:       1;
    unsigned char  rc_d:        1;

    unsigned char  rd_a:        1;
    unsigned char  rd_rw:       1;
    unsigned char  rd_d:        1;

    unsigned char  rt_oe:       1;
    unsigned char  rt_le:       1;

    unsigned char  alu_s0:      1;
    unsigned char  alu_s1:      1;
    unsigned char  alu_s2:      1;
    unsigned char  alu_s3:      1;
    unsigned char  alu_m:       1;
    unsigned char  alu_cn:      1;

    unsigned char  mem_ce:      1;
    unsigned char  mem_oe:      1;
    unsigned char  mem_we:      1;

    unsigned char  int_d:       1;
    unsigned char  int_clean:   1;

}mi_t, *p_mi;

#define BIN_LEN   0x1000
#define BIN_COUNT 9

typedef union _mi_bin
{
    mi_t            mi;
    unsigned char   bin[BIN_COUNT];

}mi_bin_t, *p_mi_bin;

#pragma pack(pop)   // 恢复先前的pack设置


static mi_bin_t       s_mi                      = { 0 };
static unsigned char  s_bin[BIN_COUNT][BIN_LEN] = { 0 };
static int            s_last_addr               = -1;

/**
 * \brief      转成多字节字符串
 * \param[in]  const wchar_t   *src     源串
 * \param[in]  int              len     源串长度
 * \param[in]  char            *des     目标串
 * \param[in]  int              size    目标串缓存大小
 * \return     void                     无
 */
void to_string(const wchar_t *src, int len, char *des, int size)
{
    int count = WideCharToMultiByte(CP_ACP, 0, src, len, des, size, 0, 0);
    des[count] = '\0';
}

/**
 * \brief      分割字符串
 * \param[in]  char         *str    源字符串
 * \param[in]  const char   *sep    分割符
 * \param[out] char         *ptr[]  分割后的指针
 * \return     int 0-成功，其它失败
 */
int split_string(char *str, const char *sep, char *ptr[])
{
    int i;

    ptr[0] = str;

    for (i = 0; ptr[i] != NULL; i++)
    {
        ptr[i + 1] = strstr(ptr[i], sep);

        if (NULL != ptr[i + 1])
        {
            *ptr[i + 1] = '\0';
            ptr[i + 1] += strlen(sep);
        }
    }

    return i;
}

/**
 * \brief      清空微指令数据
 * \param[in]  unsigned short next 下一条微指令地址
 * \return     无
 */
void clean_micro_inst_data(unsigned short next)
{
    // 清空mi
    memset(&s_mi, 0, sizeof(s_mi));

    // 下一条微指令地址
    s_mi.mi.next = next;

    // 设置默认值
    s_mi.mi.ri_oe     = 1;
    s_mi.mi.rf_a      = 1;
    s_mi.mi.rf_d      = 1;
    s_mi.mi.sc_a      = 1;
    s_mi.mi.sc_d      = 1;
    s_mi.mi.sd_a      = 1;
    s_mi.mi.sd_d      = 1;
    s_mi.mi.ss_a      = 1;
    s_mi.mi.ss_d      = 1;
    s_mi.mi.rp_a      = 1;
    s_mi.mi.rp_d      = 1;
    s_mi.mi.rs_a      = 1;
    s_mi.mi.rs_d      = 1;
    s_mi.mi.ra_a      = 1;
    s_mi.mi.ra_d      = 1;
    s_mi.mi.rb_a      = 1;
    s_mi.mi.rb_d      = 1;
    s_mi.mi.rc_a      = 1;
    s_mi.mi.rc_d      = 1;
    s_mi.mi.rd_a      = 1;
    s_mi.mi.rd_d      = 1;
    s_mi.mi.rt_oe     = 1;
    s_mi.mi.mem_ce    = 1;
    s_mi.mi.mem_oe    = 1;
    s_mi.mi.mem_we    = 1;
    s_mi.mi.int_d     = 1;
    s_mi.mi.int_clean = 1;
}

/**
 * \brief      写入bin数据
 * \param[in]  unsigned short addr 微指令地址
 * \return     void                无
 */
void write_micro_inst_buff(unsigned short addr)
{
    int i;
    int j;

    for (j = 0; j < BIN_COUNT; j++)
    {
        for (i = s_last_addr + 1; i <= addr; i++)
        {
            s_bin[j][i] = s_mi.bin[j];
        }
    }

    printf("%03X %03X %03X\n", addr, s_mi.mi.next, addr - s_last_addr);

    s_last_addr = addr;
}

/**
 * \brief      将bin数据写入文件
 * \return     void 无
 */
void write_micro_inst_file()
{
    int i;
    int len;
    char name[512];
    FILE *fp;

    for (i = 0; i < BIN_COUNT; i++)
    {
        sprintf(name, "mi_%d.bin", i);

        fp = fopen(name, "wb+");

        if (NULL == fp)
        {
            printf("open file %s error\n", name);
            return;
        }

        len = fwrite(s_bin[i], 1, BIN_LEN, fp);

        fclose(fp);

        if (len != BIN_LEN)
        {
            printf("fwrite file %s error\n", name);
        }
    }
}

/**
 * \brief      得到寄存器ID
 * \param[in]  const char  *reg    字符串
 * \return     int 寄存器ID,enum REG_ID
 */
int get_reg_id(const char *reg)
{
    int i;

    for (i = 0; i < REG_COUNT; i++)
    {
        if (0 == strcmp(reg, REG_NAME[i]))
        {
            return i;
        }
    }

    if (i == REG_COUNT)
    {
        printf("get reg:%s id error\n", reg);
    }

    return REG_COUNT;
}

/**
 * \brief      设置微指令
 * \param[in]  int reg_id   寄存器ID,enum REG_ID
 * \param[in]  int rw       读写:REG_READ,REG_WRITE
 * \param[in]  int bus      总线:BUS_ADDR,BUS_DATA
 * \return     int 0-成功，其它失败
 */
int set_reg_data(int reg_id, int rw, int bus)
{
    switch (reg_id)
    {
        case REG_READ:
        {
            break;
        }
        case REG_RI:
        {
            s_mi.mi.ri_le = rw;
            s_mi.mi.ri_oe = rw;
            break;
        }
        case REG_RF:
        {
            s_mi.mi.rf_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.rf_a = 0 : s_mi.mi.rf_d = 0;
            break;
        }
        case REG_SC:
        {
            s_mi.mi.sc_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.sc_a = 0 : s_mi.mi.sc_d = 0;
            break;
        }
        case REG_SD:
        {
            s_mi.mi.sd_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.sd_a = 0 : s_mi.mi.sd_d = 0;
            break;
        }
        case REG_SS:
        {
            s_mi.mi.ss_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.ss_a = 0 : s_mi.mi.ss_d = 0;
            break;
        }
        case REG_RP:
        {
            s_mi.mi.rp_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.rp_a = 0 : s_mi.mi.rp_d = 0;
            break;
        }
        case REG_RS:
        {
            s_mi.mi.rs_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.rs_a = 0 : s_mi.mi.rs_d = 0;
            break;
        }
        case REG_RA:
        {
            s_mi.mi.ra_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.ra_a = 0 : s_mi.mi.ra_d = 0;
            break;
        }
        case REG_RB:
        {
            s_mi.mi.rb_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.rb_a = 0 : s_mi.mi.rb_d = 0;
            break;
        }
        case REG_RC:
        {
            s_mi.mi.rc_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.rc_a = 0 : s_mi.mi.rc_d = 0;
            break;
        }
        case REG_RD:
        {
            s_mi.mi.rd_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.rd_a = 0 : s_mi.mi.rd_d = 0;
            break;
        }
        case REG_RT:
        {
            s_mi.mi.rt_oe = rw;
            s_mi.mi.rt_le = rw;
            break;
        }
        case REG_MEM:
        {
            s_mi.mi.mem_ce = 0;
            s_mi.mi.mem_oe = (REG_WRITE == rw);
            s_mi.mi.mem_we = (REG_READ == rw);
            break;
        }
        case REG_NOT_RT:
        {
            s_mi.mi.ss_rw = rw;
            s_mi.mi.rs_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.ss_a = 0 : s_mi.mi.ss_d = 0;
            (BUS_ADDR == bus) ? s_mi.mi.rs_a = 0 : s_mi.mi.rs_d = 0;
        } // 没有break
        case REG_NOT_STACK:
        {
            s_mi.mi.ri_le = rw;
            s_mi.mi.ri_oe = rw;
            s_mi.mi.rf_rw = rw;
            s_mi.mi.sc_rw = rw;
            s_mi.mi.sd_rw = rw;
            s_mi.mi.rp_rw = rw;
            s_mi.mi.ra_rw = rw;
            s_mi.mi.rb_rw = rw;
            s_mi.mi.rc_rw = rw;
            s_mi.mi.rd_rw = rw;
            (BUS_ADDR == bus) ? s_mi.mi.rf_a = 0 : s_mi.mi.rf_d = 0;
            (BUS_ADDR == bus) ? s_mi.mi.sc_a = 0 : s_mi.mi.sc_d = 0;
            (BUS_ADDR == bus) ? s_mi.mi.sd_a = 0 : s_mi.mi.sd_d = 0;
            (BUS_ADDR == bus) ? s_mi.mi.rp_a = 0 : s_mi.mi.rp_d = 0;
            (BUS_ADDR == bus) ? s_mi.mi.ra_a = 0 : s_mi.mi.ra_d = 0;
            (BUS_ADDR == bus) ? s_mi.mi.rb_a = 0 : s_mi.mi.rb_d = 0;
            (BUS_ADDR == bus) ? s_mi.mi.rc_a = 0 : s_mi.mi.rc_d = 0;
            (BUS_ADDR == bus) ? s_mi.mi.rd_a = 0 : s_mi.mi.rd_d = 0;
            break;
        }
        default:
        {
            printf("set_reg_data reg:%s error\n", REG_NAME[reg_id]);
            return -1;
        }
    }

    return 0;
}

/**
 * \brief      处理三目微指令,格式:REG=>BUS=>REG
 * \param[in]  const char *ptr[]   微指令字符串
 * \return     int 0-成功，其它失败
 */
int proc_three_item(char *ptr[])
{
    int i;
    int id[4] = { 0 };
    char *p[16];

    int count = split_string(ptr[0], ":", p);

    if (count > 2)
    {
        printf("proc_three_item left:%s error\n", ptr[0]);
    }

    for (i = 0; i < count; i++)
    {
        id[i] = get_reg_id(p[i]);
    }

    id[2] = get_reg_id(ptr[1]);
    id[3] = get_reg_id(ptr[2]);

    switch (id[2])
    {
        case BUS_ADDR:
        {
            set_reg_data(id[0], REG_READ,  BUS_ADDR);
            set_reg_data(id[1], REG_READ,  BUS_ADDR);
            set_reg_data(id[3], REG_WRITE, BUS_ADDR);
            break;
        }
        case BUS_DATA:
        {
            set_reg_data(id[0], REG_READ,  BUS_DATA);
            set_reg_data(id[1], REG_READ,  BUS_DATA);
            set_reg_data(id[3], REG_WRITE, BUS_DATA);
            break;
        }
        default:
        {
            printf("proc_three_item left:%s midd:%s right:%s error\n",
                    ptr[0], ptr[1], ptr[2]);
            break;
        }
    }

    return 0;
}

/**
 * \brief      处理二目微指令,格式:BUS=>REG,REG=>BUS,ALU(**)=>RT,RI=>SEL
 * \param[in]  const char *ptr[]   微指令字符串
 * \return     int 0-成功，其它失败
 */
int proc_two_item(char *ptr[])
{
    int i;
    int id[4] = { 0 };
    char *p[16];

    int count = split_string(ptr[0], ":", p);

    if (count > 2)
    {
        printf("proc_two_item left:%s error\n", ptr[0]);
    }

    for (i = 0; i < count; i++)
    {
        id[i] = get_reg_id(p[i]);
    }

    id[2] = get_reg_id(ptr[1]);

    switch (id[0])
    {
        case BUS_ADDR:
        {
            set_reg_data(id[2], REG_WRITE, BUS_ADDR);
            break;
        }
        case BUS_DATA:
        {
            set_reg_data(id[2], REG_WRITE, BUS_DATA);
            break;
        }
        case ALU_00:
        {
            s_mi.mi.alu_s0 = 1;
            s_mi.mi.alu_s1 = 1;
            s_mi.mi.alu_s2 = 0;
            s_mi.mi.alu_s3 = 0;
            s_mi.mi.alu_m  = 1;
            s_mi.mi.alu_cn = 1;
            s_mi.mi.rt_oe  = REG_WRITE;
            s_mi.mi.rt_le  = REG_WRITE;
            break;
        }
        case ALU_FF:
        {
            s_mi.mi.alu_s0 = 0;
            s_mi.mi.alu_s1 = 0;
            s_mi.mi.alu_s2 = 1;
            s_mi.mi.alu_s3 = 1;
            s_mi.mi.alu_m  = 1;
            s_mi.mi.alu_cn = 1;
            s_mi.mi.rt_oe  = REG_WRITE;
            s_mi.mi.rt_le  = REG_WRITE;
            break;
        }
        case ALU_A_ADD_ADD:
        {
            s_mi.mi.alu_s0 = 0;
            s_mi.mi.alu_s1 = 0;
            s_mi.mi.alu_s2 = 0;
            s_mi.mi.alu_s3 = 0;
            s_mi.mi.alu_m  = 0;
            s_mi.mi.alu_cn = 0;
            s_mi.mi.rt_oe  = REG_WRITE;
            s_mi.mi.rt_le  = REG_WRITE;
            break;
        }
        case ALU_A_SUB_SUB:
        {
            s_mi.mi.alu_s0 = 1;
            s_mi.mi.alu_s1 = 1;
            s_mi.mi.alu_s2 = 1;
            s_mi.mi.alu_s3 = 1;
            s_mi.mi.alu_m  = 0;
            s_mi.mi.alu_cn = 1;
            s_mi.mi.rt_oe  = REG_WRITE;
            s_mi.mi.rt_le  = REG_WRITE;
            break;
        }
        case ALU_A_SUB_D:
        {
            s_mi.mi.alu_s0 = 0;
            s_mi.mi.alu_s1 = 0;
            s_mi.mi.alu_s2 = 1;
            s_mi.mi.alu_s3 = 1;
            s_mi.mi.alu_m  = 0;
            s_mi.mi.alu_cn = 0;
            s_mi.mi.rt_oe  = REG_WRITE;
            s_mi.mi.rt_le  = REG_WRITE;
            break;
        }
        default:
        {
            switch (id[2])
            {
                case BUS_ADDR:
                {
                    set_reg_data(id[0], REG_READ, BUS_ADDR);
                    set_reg_data(id[1], REG_READ, BUS_ADDR);
                    break;
                }
                case BUS_DATA:
                {
                    set_reg_data(id[0], REG_READ, BUS_DATA);
                    set_reg_data(id[1], REG_READ, BUS_DATA);
                    break;
                }
                case ADDR_SEL:
                {
                    set_reg_data(id[0], REG_READ, 0);
                    break;
                }
                default:
                {
                    printf("proc_two_item left:%s right:%s error\n", ptr[0], ptr[1]);
                    break;
                }
            }

            break;
        }
    }

    return 0;
}

/**
 * \brief      处理一目微指令,格式:CHECK-*
 * \param[in]  const char *ptr[]   微指令字符串
 * \return     int 0-成功，其它失败
 */
int proc_one_item(char *ptr[])
{
    if (0 == strcmp(ptr[0], "INT-CLEAN"))
    {
        s_mi.mi.int_clean = 0; // 0有效
    }
    else if (0 == strcmp(ptr[0], "INT-D"))
    {
        s_mi.mi.int_d = 0; // 0有效
    }
    else if (0 == strcmp(ptr[0], "SELECT-RI"))
    {
        s_mi.mi.select_ri = true;
    }
    else if (0 == strcmp(ptr[0], "CHECK-INT"))
    {
        s_mi.mi.check_int = true;
    }
    else if (0 == strcmp(ptr[0], "CHECK-A==B"))
    {
        s_mi.mi.check_je = true;
    }
    else if (0 == strcmp(ptr[0], "CHECK-A!=B"))
    {
        s_mi.mi.check_jne = true;
    }
    else if (0 == strcmp(ptr[0], "CHECK-A>B"))
    {
         s_mi.mi.check_jb = true;
    }
    else if (0 == strcmp(ptr[0], "CHECK-A>=B"))
    {
        s_mi.mi.check_jbe = true;
    }
    else if (0 == strcmp(ptr[0], "CHECK-A<B"))
    {
        s_mi.mi.check_jl = true;
    }
    else if (0 == strcmp(ptr[0], "CHECK-A<=B"))
    {
        s_mi.mi.check_jle = true;
    }
    else if (0 == strcmp(ptr[0], "ALU(A-D)")) // CMP比较命令使用
    {
        s_mi.mi.alu_s0 = 0;
        s_mi.mi.alu_s1 = 1;
        s_mi.mi.alu_s2 = 1;
        s_mi.mi.alu_s3 = 0;
        s_mi.mi.alu_m  = 0;
        s_mi.mi.alu_cn = 0;
    }
    else
    {
        printf("proc_one_item %s error\n", ptr[0]);
        return -1;
    }

    return 0;
}

/**
 * \brief      处理微指令
 * \param[in]  char *micro_inst 微指令字符: CS:RP=>ABUS
 * \return     int 0-成功，其它失败
 */
int proc_micro_inst(char *micro_inst)
{
    char *ptr[16];

    int count = split_string(micro_inst, "=>", ptr);

    switch (count)
    {
        case 1:
        {
            proc_one_item(ptr);
            break;
        }
        case 2:
        {
            proc_two_item(ptr);
            break;
        }
        case 3:
        {
            proc_three_item(ptr);
            break;
        }
        default:
        {
            printf("micro inst count:%d error\n", count);
            break;
        }
    }

    return 0;
}

/**
 * \brief      处理微指令,以豆号为界
 * \param[in]  char *micro_inst_list  微指令字符串: CS:RP=>ABUS=>MEM=>RI,RP=>ALU(++)=>TR
 * \return     int 0-成功，其它失败
 */
int proc_micro_inst_list(char *micro_inst_list)
{
    int i;
    int count;
    char *ptr[16];

    if (0 == strcmp(micro_inst_list, ""))
    {
        return 0;
    }

    count = split_string(micro_inst_list, ",", ptr);

    for (i = 0; i < count; i++)
    {
        proc_micro_inst(ptr[i]);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    const char *filename = "mi.xls";

    BasicExcel e;

    if (!e.Load(filename, ios_base::in)) // 只读
    {
        printf("load %s error\n", filename);
        return -1;
    }

    BasicExcelWorksheet* sheet = e.GetWorksheet(L"8bit");

    if (NULL == sheet)
    {
        printf("get sheet 8bit error\n");
        return -2;
    }

    size_t max_row = sheet->GetTotalRows();
    size_t max_col = sheet->GetTotalCols();

    printf("max_row:%d max_col:%d\n", max_row, max_col);

    if (BIN_COUNT != sizeof(s_mi))
    {
        printf("mi_size:%d != %d\n", sizeof(s_mi), BIN_COUNT);
        return -3;
    }

    char inst[512];
    char micro_inst_list[512];
    unsigned short addr = 0;
    unsigned short next = 0;
    unsigned short inst_id = -1;
    unsigned short micro_inst_id = 0;
    BasicExcelCell *c0;
    BasicExcelCell *c1;
    BasicExcelCell *c2;

    // 从第6行开始
    for (size_t r = 5; r < max_row; ++r)
    {
        c0 = sheet->Cell(r, 0);     // 得到单元格
        c1 = sheet->Cell(r, 2);
        c2 = sheet->Cell(r + 1, 0); // 下一条数据

        if (c0->Type() == BasicExcelCell::UNDEFINED)
        {
            micro_inst_id++;        // 同一条指令,新的微指令
        }
        else
        {
            inst_id++;              // 一条新的指令
            micro_inst_id = 0;
            to_string(c0->GetWString(), c0->GetStringLength(), inst, sizeof(inst));
        }

        to_string(c1->GetWString(), c1->GetStringLength(), micro_inst_list, sizeof(micro_inst_list));

        printf("%-15s %-40s", inst, micro_inst_list);

        if (0 == strcmp(inst, "SYSTEM")) // 系统保留
        {
            addr = 0xFF9 + micro_inst_id;

            switch (addr)
            {
                case 0xFFE:
                {
                    next = 0xFFA;
                    break;
                }
                case 0xFFF:
                {
                    next = 0xFFC;
                    break;
                }
                default:
                {
                    next = addr + 1;
                    break;
                }
            }
        }
        else
        {
            addr = inst_id * 16 + micro_inst_id;
            next = (c2->Type() != BasicExcelCell::UNDEFINED) ? 0xFF9 : addr + 1;
        }

        // 清空数据
        clean_micro_inst_data(next);

        // 得到微指令数据
        proc_micro_inst_list(micro_inst_list);

        // 保存数据
        write_micro_inst_buff(addr);
    }

    // 将数据写入文件
    write_micro_inst_file();
    return 0;
}
