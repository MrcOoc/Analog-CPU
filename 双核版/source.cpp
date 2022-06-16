/*冯诺依曼式CPU模拟器
@author: Tian ZY
@date:2021.6.5
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<windows.h>

void getOrder(char* code, FILE** fsPtr);//读取存储指令
int judgeOrder(char* code, int* data, int* ax, int* ip, int* ir, int* flag,int *lockAddress,int id);//分析指令
int binaryNum(char code[], int start, int num);//把字符串转换成二进制数，再转换成十进制
void transOrder(int objfront, int objback, int lmmediate, int data[], int ax[]);//传输
void arithmeticOperation(int opcode, int frontReg, int backReg,
                         int lmmediate, int data[], int ax[]);//算术运算
void logicalOperation(int opcode, int frontReg, int backReg, int lmmediate,
                      int data[], int ax[]);//逻辑运算
void compareOrder(int opcode, int frontReg, int backReg, int lmmediate, int data[],
                  int ax[], int* flag);//比较
void skipOrder(int backReg, int lmmediate, int* ip, int* flag);//跳转
void lockOrder(int lmmediate,int *lockAddress);//锁定 
void unlockOrder(int lmmediate,int *lockAddress);//解锁 
void sleepOrder(int lmmediate);//休眠 
void inOutPut(int opcode, int frontReg, int ax[],int id);//输入输出
//int systemShift(int binaryNum);//将二进制数转为十进制
void outPut(char code[], int data[], int* ip1,int* ip2);//输出

int main(int argc, char* argv[]) {
    FILE* fPtr1;
    FILE* fPtr2;
    if ((fPtr1 = fopen("dict1.dic", "r")) == NULL) {
        printf("open filed");
        return -1;//打开文件失败后结束 
    }
    if ((fPtr2 = fopen("dict2.dic", "r")) == NULL) {
        printf("open filed");
        return -1;//打开文件失败后结束 
    }
    char code1[16385 + 10];
    char code2[16385 + 10];
    int data[32768 + 10] = {0};
    int ip1 = 0, ir1 = 0, flag1 = 0, cpu1 = 1;//程序、指令、标志寄存器 
    int ax1[9] = {0};//1-4数据寄存器5-8地址寄存器 
    int ip2 = 0, ir2 = 0, flag2 = 0, cpu2 = 1;
    int ax2[9] = {0};    
    
    int lockAddress = -1;//锁定的内存 -1为没有锁
    data[0] = 100; //初始化内存地址16384 100张票 
    
    getOrder(code1, &fPtr1);
    fclose(fPtr1);
    getOrder(code2, &fPtr2);
    fclose(fPtr2);
    while(1==cpu1 || 1==cpu2){//双核cpu 
    	if(1 == cpu1){
    		cpu1 = judgeOrder(code1, data, ax1, &ip1, &ir1, &flag1,&lockAddress,1);
		}
    	if(1 == cpu2){
    		cpu2 = judgeOrder(code2, data, ax2, &ip2, &ir2, &flag2,&lockAddress,2);
		}
	}

    outPut(code1, data, &ip1,&ip2);
    return 0;
}

//读取存储全部指令
void getOrder(char* code, FILE** fsPtr) {
    FILE* fp = *fsPtr;
    int i = 0;
    char c;
    while (!feof(fp)) {
        c = fgetc(fp);
        if(c == '0' || c == '1'){//过滤异常数据 
        	code[i] = c;
        	i++;
		}
    }
    code[i] = '\0';//用结束符代替NULL 
}

//分析指令，判断指令类型
int judgeOrder(char* code, int* data, int* ax, int* ip, int* ir, int* flag,int *lockAddress,int id) {
    int opcode = 0, frontReg = 0, backReg = 0, lmmediate = 0;   //操作码，前寄存器，后寄存器，立即数
    int len = strlen(code);
    int start = (*ip) / 4 * 32; //所需指令开始位置
    opcode = binaryNum(code, start, 8); //操作码
    start += 8;
    frontReg = binaryNum(code, start, 4);   //前寄存器
    start += 4;
    backReg = binaryNum(code, start, 4);    //后寄存器
    start += 4;
    lmmediate = binaryNum(code, start, 16); //立即数

    *ip += 4;   //标志寄存器更新
    *ir = opcode * 256 + frontReg * 16 + backReg;
    
	if(opcode == 0){
		if(1 == id){
 			printf("\nid = %d\nip = %d\nflag = %d\nir = %d\n", id,*ip, *flag, *ir);			
		}
		else{
        	printf("\nid = %d\nip = %d\nflag = %d\nir = %d\n", id,*ip+256, *flag, *ir);			
		}
        for (int i = 1; i <= 4; i++) {
            printf("ax%d = %d ", i, ax[i]);
        }
        printf("\n");
        for (int i = 5; i <= 8; i++) {
            printf("ax%d = %d ", i, ax[i]);
        }
        printf("\n");			
		return 0;
	}
   else if (opcode == 1) {//传输	
        transOrder(frontReg, backReg, lmmediate, data, ax);
    }
    else if (opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5) {//算数运算
        arithmeticOperation(opcode, frontReg, backReg, lmmediate, data, ax);
    }
   else if (opcode >= 6 && opcode <= 8) {//逻辑运算
        logicalOperation(opcode, frontReg, backReg, lmmediate, data, ax);
    }
    else if (opcode == 9) {//比较
        compareOrder(opcode, frontReg, backReg, lmmediate, data, ax, flag);
    }
     else if (opcode == 10) {//跳转
        skipOrder(backReg, lmmediate, ip, flag);
    }
    else if (opcode == 11 || opcode == 12) {//输入输出
        inOutPut(opcode, frontReg, ax,id);
    }
    else if (opcode == 13) {//锁定
	    if(*lockAddress != -1){
	    	*ip -= 4;
	    	return 1;
		}      	
    	lockOrder(lmmediate,lockAddress);
	}
    else if (opcode == 14) {//解锁
    	unlockOrder(lmmediate,lockAddress);
	}		
    else if (opcode == 16) {//休眠
    	sleepOrder(lmmediate);
	}
    
	if(1 == id){
		printf("\nid = %d\nip = %d\nflag = %d\nir = %d\n", id, *ip, *flag, *ir);			
	}
	else{
    	printf("\nid = %d\nip = %d\nflag = %d\nir = %d\n", id, *ip+256, *flag, *ir);			
	}
    

    for (int i = 1; i <= 4; i++) {
        printf("ax%d = %d ", i, ax[i]);
    }
    printf("\n");
    for (int i = 5; i <= 8; i++) {
        printf("ax%d = %d ", i, ax[i]);
    }
    printf("\n");
    return 1;
}

//将字符串转为二进制数，再返回对应十进制数
int binaryNum(char code[], int start, int num) {
	int decimalNum = 0, i = 0;
	for (i=0; i<num; i++) {
		decimalNum = decimalNum *2 + code[start + i] - '0';
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
            ax[objfront] = data[(ax[objback] - 16384)/2];
        }
        else {//objfront >= 5
            data[(ax[objfront] - 16384)/2] = ax[objback];
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
            ax[frontReg] += data[(ax[backReg] - 16384)/2];
        }
        else if (opcode == 3) {
            ax[frontReg] -= data[(ax[backReg] - 16384)/2];
        }
        else if (opcode == 4) {
            ax[frontReg] *= data[(ax[backReg] - 16384)/2];
        }
        else {//opcode == 5
            ax[frontReg] /= data[(ax[backReg] - 16384)/2];
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
            if (ax[frontReg] && data[(ax[backReg] - 16384)/2]) {
                ax[frontReg] = 1;
            }
            else {
                ax[frontReg] = 0;
            }
        }
        else if (opcode == 7) {
            if (ax[frontReg] || data[(ax[backReg] - 16384)/2]) {
                ax[frontReg] = 1;
            }
            else {
                ax[frontReg] = 0;
            }
        }
        else if (opcode == 8) {
            if (data[(ax[backReg] - 16384)/2] = !data[(ax[backReg] - 16384)/2]) {
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
    //寄存器之间的传递
    else {
        if (ax[frontReg] > data[(ax[backReg] - 16384)/2])
            *flag = 1;
        if (ax[frontReg] == data[(ax[backReg] - 16384)/2])
            *flag = 0;
        if (ax[frontReg] < data[(ax[backReg] - 16384)/2])
            *flag = -1;
    }
}

//跳转指令
void skipOrder(int backReg, int lmmediate, int* ip, int* flag) {
    if (backReg == 0) {
        *ip += lmmediate-4;
    }
    if (backReg == 1) {
        if (*flag == 0)
            *ip += lmmediate-4;
    }
    if (backReg == 2) {
        if (*flag == 1)
            *ip += lmmediate-4;
    }
    if (backReg == 3) {
        if (*flag == -1)
            *ip += lmmediate-4;
    }
    
    if(*ip > 0xFFFF){
    	*ip = *ip - 0xFFFF - 1;
	}
}

//锁定指令 
void lockOrder(int lmmediate,int *lockAddress) {
	*lockAddress = lmmediate;
}

//解锁指令 
void unlockOrder(int lmmediate,int *lockAddress) {
	if(lmmediate == *lockAddress){
		*lockAddress = -1;
	}
}

//休眠指令 
void sleepOrder(int lmmediate) {
	Sleep(lmmediate);
}

//输入输出指令
void inOutPut(int opcode, int frontReg, int ax[],int id) {
    if (opcode == 11) {
        printf("in:\n");
        scanf("%d", &ax[frontReg]);
    }
    else if(opcode == 12){
        printf("id = %d    out: %d\n", id,ax[frontReg]);
    }
}

void outPut(char code[], int data[], int* ip1,int* ip2) {
    int cnt, task, start, i;
    task = 1;
    cnt = 0;
    *ip1 = 0;
    *ip2 = 0;
    //代码段  
    printf("\ncodeSegment :\n");
    while (task != 0) {
        start = (*ip1) / 4 * 32;
        task = binaryNum(code, start, 32);
        printf("%d ", task);
        *ip1 = (*ip1) + 4;
        cnt++;
        if (cnt % 8 == 0)
            printf("\n");
    }
    for (i = 8 * 8 - cnt; i > 0; i--) {
        printf("0 ");
        cnt++;
        if (cnt % 8 == 0)
            printf("\n");
    }
    task = 1;
    cnt = 0;
    while (task != 0) {
        start = (*ip2) / 4 * 32;
        task = binaryNum(code, start, 32);
        printf("%d ", task);
        *ip2 = (*ip2) + 4;
        cnt++;
        if (cnt % 8 == 0)
            printf("\n");
    }
    for (i = 8 * 8 - cnt; i > 0; i--) {
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
