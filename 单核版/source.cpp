/*冯诺依曼式CPU模拟器
@author: Tian Zhenyu
@date:2021.5.21
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

void getOrder(char* code, FILE** fsPtr);//读取存储指令
void judgeOrder(char* code, int* data, int* ax, int* ip, int* ir, int* flag);//分析指令
int binaryNum(char code[], int start, int num);//把字符串转换成十进制数
void transOrder(int objfront, int objback, int lmmediate, int data[], int ax[]);//传输
void arithmeticOperation(int opcode, int frontReg, int backReg,
                         int lmmediate, int data[], int ax[]);//算术运算
void logicalOperation(int opcode, int frontReg, int backReg, int lmmediate,
                      int data[], int ax[]);//逻辑运算
void compareOrder(int opcode, int frontReg, int backReg, int lmmediate, int data[],
                  int ax[], int* flag);//比较
void skipOrder(int backReg, int lmmediate, int* ip, int* flag);//跳转
void inOutPut(int opcode, int frontReg, int ax[]);//输入输出
void outPut(char code[], int data[], int* ip);//输出

int main(int argc, char* argv[]) {
    FILE* fPtr;
    if ((fPtr = fopen("dict.dic", "r")) == NULL) {
        printf("open filed");
        return -1;// add1 打开文件失败后结束 
    }
    char code[16385 + 10];
    int ip, ir, flag;
    int data[32768 + 10] = {0}, ax[9] = {0};
    getOrder(code, &fPtr);
    fclose(fPtr);
    judgeOrder(code, data, ax, &ip, &ir, &flag);
    outPut(code, data, &ip);
    return 0;//add2 程序结束需要返回 
}

//读取存储全部指令aaa
void getOrder(char* code, FILE** fsPtr) {
    FILE* fp = *fsPtr;
    int i = 0;
    char c;
    while (!feof(fp)) {
        c = fgetc(fp);
        if (c == '0' || c == '1') {//过滤不正常的数据 
            code[i] = c;
            i++;
        }
    }
    code[i] = '\0';//m3 用结束符代替NULL 
}

//分析指令，判断指令类型
void judgeOrder(char* code, int* data, int* ax, int* ip, int* ir, int* flag) {
    int opcode = 0, frontReg = 0, backReg = 0, lmmediate = 0;   //操作码，前寄存器，后寄存器，立即数
    *ip = 0;//add 4 对ip进行初始化 
    *flag = 0;
    *ir = 0;
    int len = strlen(code);
    int start = 0;
    do {
        start = (*ip) / 4 * 32; //所需指令开始位置
        opcode = binaryNum(code, start, 8); //操作码
        start += 8;
        frontReg = binaryNum(code, start, 4);   //前寄存器
        start += 4;
        backReg = binaryNum(code, start, 4);    //后寄存器
        start += 4;
        lmmediate = binaryNum(code, start, 16); //立即数

        *ip += 4;   //标志寄存器更新
        *ir = opcode * 256 + frontReg * 16 + backReg;

        if (opcode == 0) {
            printf("ip = %d\nflag = %d\nir = %d\n", *ip, *flag, *ir);
            for (int i = 1; i <= 4; i++) {
                printf("ax%d = %d ", i, ax[i]);
            }
            printf("\n");
            for (int i = 5; i <= 8; i++) {
                printf("ax%d = %d ", i, ax[i]);
            }
            printf("\n");
            return;
        }
        else if (opcode == 1) {
            transOrder(frontReg, backReg, lmmediate, data, ax);
        }
        else if (opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5) {
            arithmeticOperation(opcode, frontReg, backReg, lmmediate, data, ax);
        }
        else if (opcode >= 6 && opcode <= 8) {
            logicalOperation(opcode, frontReg, backReg, lmmediate, data, ax);
        }
        else if (opcode == 9) {
            compareOrder(opcode, frontReg, backReg, lmmediate, data, ax, flag);
        }
        else if (opcode == 10) {
            skipOrder(backReg, lmmediate, ip, flag);
        }
        else if (opcode == 11 || opcode == 12){//opcode == 11||12
            inOutPut(opcode, frontReg, ax);
        }



        printf("ip = %d\nflag = %d\nir = %d\n", *ip, *flag, *ir);
        for (int i = 1; i <= 4; i++) {
            printf("ax%d = %d ", i, ax[i]);
        }
        printf("\n");
        for (int i = 5; i <= 8; i++) {
            printf("ax%d = %d ", i, ax[i]);
        }
        printf("\n");
    } while (start < len);
}

//将字符串转为二进制数，再调用systemShift返回十进制数
int binaryNum(char code[], int start, int num) {
    int decimalNum = 0, i = 0;
    for (i = 0; i < num; i++) {
        decimalNum = decimalNum * 2 + code[start + i] - '0';
    }
    return decimalNum;
}

//数据传送指令
void transOrder(int objfront, int objback, int lmmediate,
                int data[], int ax[]) {
    //立即数的传递 
    if (objback == 0) {
        ax[objfront] = lmmediate;
    }
    //寄存器之间的传递
    else {
        if (objfront <= 4) {
            ax[objfront] = data[(ax[objback] - 16384) / 2];
        }
        else {//objfront >= 5
            data[(ax[objfront] - 16384) / 2] = ax[objback];
        }
    }
}

//算数运算指令
void arithmeticOperation(int opcode, int frontReg, int backReg, int lmmediate,
                         int data[], int ax[]) {
    if (backReg == 0) {
        if (opcode == 2) {
            ax[frontReg] += lmmediate;
        }
        else if (opcode == 3) {
            ax[frontReg] -= lmmediate;
        }
        else if (opcode == 4) {
            ax[frontReg] *= lmmediate;
        }
        else {//opcode == 5
            ax[frontReg] /= lmmediate;
        }
    }
    else {//backReg != 0
        if (opcode == 2) {
            ax[frontReg] += data[(ax[backReg] - 16384) / 2];
        }
        else if (opcode == 3) {
            ax[frontReg] -= data[(ax[backReg] - 16384) / 2];
        }
        else if (opcode == 4) {
            ax[frontReg] *= data[(ax[backReg] - 16384) / 2];
        }
        else {//opcode == 5
            ax[frontReg] /= data[(ax[backReg] - 16384) / 2];
        }
    }
}

//逻辑运算指令
void logicalOperation(int opcode, int frontReg, int backReg, int lmmediate,
                      int data[], int ax[]) {
    if (backReg == 0) {
        if (opcode == 6) {
            if (ax[frontReg] && lmmediate) {
                ax[frontReg] = 1;
            }
            else {
                ax[frontReg] = 0;
            }
        }
        else if (opcode == 7) {
            if (ax[frontReg] || lmmediate) {
                ax[frontReg] = 1;
            }
            else {
                ax[frontReg] = 0;
            }
        }
        else if (opcode == 8) {
            if (ax[frontReg] = !ax[frontReg]) {
                ax[frontReg] = 1;
            }
            else {
                ax[frontReg] = 0;
            }
        }
    }
    else {
        if (opcode == 6) {
            if (ax[frontReg] && data[(ax[backReg] - 16384) / 2]) {
                ax[frontReg] = 1;
            }
            else {
                ax[frontReg] = 0;
            }
        }
        else if (opcode == 7) {
            if (ax[frontReg] || data[(ax[backReg] - 16384) / 2]) {
                ax[frontReg] = 1;
            }
            else {
                ax[frontReg] = 0;
            }
        }
        else if (opcode == 8) {
            if (data[(ax[backReg] - 16384) / 2] = !data[(ax[backReg] - 16384) / 2]) {
                ax[frontReg] = 1;
            }
            else {
                ax[frontReg] = 0;
            }
        }
    }
}

//比较指令
void compareOrder(int opcode, int frontReg, int backReg, int lmmediate, int data[],
                  int ax[], int* flag) {
    if (backReg == 0) {
        if (ax[frontReg] > lmmediate)
            *flag = 1;
        if (ax[frontReg] == lmmediate)
            *flag = 0;
        if (ax[frontReg] < lmmediate)
            *flag = -1;
    }
    //寄存器 之间的传递
    else {
        if (ax[frontReg] > data[(ax[backReg] - 16384) / 2])
            *flag = 1;
        if (ax[frontReg] == data[(ax[backReg] - 16384) / 2])
            *flag = 0;
        if (ax[frontReg] < data[(ax[backReg] - 16384) / 2])
            *flag = -1;
    }
}

//跳转指令
void skipOrder(int backReg, int lmmediate, int* ip, int* flag) {
    if (backReg == 0) {
        *ip += lmmediate - 4;
    }
    if (backReg == 1) {
        if (*flag == 0)
            *ip += lmmediate - 4;
    }
    if (backReg == 2) {
        if (*flag == 1)
            *ip += lmmediate - 4;
    }
    if (backReg == 3) {
        if (*flag == -1)
            *ip += lmmediate - 4;
    }

    if (*ip > 0xFFFF) {
        *ip = *ip - 0xFFFF - 1;
    }
}

//输入输出指令
void inOutPut(int opcode, int frontReg, int ax[]) {
    if (opcode == 11) {
        printf("in:\n");
        scanf("%d", &ax[frontReg]);
    }
    else {//opcode == 12
        printf("out:\n");
        printf("%d\n", ax[frontReg]);
    }
}

void outPut(char code[], int data[], int* ip) {
    int cnt, task, start, i;
    task = 1;
    cnt = 0;
    *ip = 0;
    //代码段  
    printf("\ncodeSegment :\n");
    while (task != 0) {
        start = (*ip) / 4 * 32;
        task = binaryNum(code, start, 32);
        printf("%d ", task);
        *ip = (*ip) + 4;
        cnt++;
        if (cnt % 8 == 0)
            printf("\n");
    }
    for (i = 16 * 8 - cnt; i > 0; i--) {
        printf("0 ");
        cnt++;
        if (cnt % 8 == 0)
            printf("\n");
    }
    //数据段 
    printf("\ndataSegment :\n");
    cnt = 0;
    for (i = 0; i <= 16 * 16 - 1; i++) {
        printf("%d ", data[i]);
        cnt++;
        if (cnt % 16 == 0)
            printf("\n");
    }
}
