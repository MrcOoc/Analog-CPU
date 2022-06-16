/*��ŵ����ʽCPUģ����
@author: Tian Zhenyu
@date:2021.5.21
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

void getOrder(char* code, FILE** fsPtr);//��ȡ�洢ָ��
void judgeOrder(char* code, int* data, int* ax, int* ip, int* ir, int* flag);//����ָ��
int binaryNum(char code[], int start, int num);//���ַ���ת����ʮ������
void transOrder(int objfront, int objback, int lmmediate, int data[], int ax[]);//����
void arithmeticOperation(int opcode, int frontReg, int backReg,
                         int lmmediate, int data[], int ax[]);//��������
void logicalOperation(int opcode, int frontReg, int backReg, int lmmediate,
                      int data[], int ax[]);//�߼�����
void compareOrder(int opcode, int frontReg, int backReg, int lmmediate, int data[],
                  int ax[], int* flag);//�Ƚ�
void skipOrder(int backReg, int lmmediate, int* ip, int* flag);//��ת
void inOutPut(int opcode, int frontReg, int ax[]);//�������
void outPut(char code[], int data[], int* ip);//���

int main(int argc, char* argv[]) {
    FILE* fPtr;
    if ((fPtr = fopen("dict.dic", "r")) == NULL) {
        printf("open filed");
        return -1;// add1 ���ļ�ʧ�ܺ���� 
    }
    char code[16385 + 10];
    int ip, ir, flag;
    int data[32768 + 10] = {0}, ax[9] = {0};
    getOrder(code, &fPtr);
    fclose(fPtr);
    judgeOrder(code, data, ax, &ip, &ir, &flag);
    outPut(code, data, &ip);
    return 0;//add2 ���������Ҫ���� 
}

//��ȡ�洢ȫ��ָ��aaa
void getOrder(char* code, FILE** fsPtr) {
    FILE* fp = *fsPtr;
    int i = 0;
    char c;
    while (!feof(fp)) {
        c = fgetc(fp);
        if (c == '0' || c == '1') {//���˲����������� 
            code[i] = c;
            i++;
        }
    }
    code[i] = '\0';//m3 �ý���������NULL 
}

//����ָ��ж�ָ������
void judgeOrder(char* code, int* data, int* ax, int* ip, int* ir, int* flag) {
    int opcode = 0, frontReg = 0, backReg = 0, lmmediate = 0;   //�����룬ǰ�Ĵ�������Ĵ�����������
    *ip = 0;//add 4 ��ip���г�ʼ�� 
    *flag = 0;
    *ir = 0;
    int len = strlen(code);
    int start = 0;
    do {
        start = (*ip) / 4 * 32; //����ָ�ʼλ��
        opcode = binaryNum(code, start, 8); //������
        start += 8;
        frontReg = binaryNum(code, start, 4);   //ǰ�Ĵ���
        start += 4;
        backReg = binaryNum(code, start, 4);    //��Ĵ���
        start += 4;
        lmmediate = binaryNum(code, start, 16); //������

        *ip += 4;   //��־�Ĵ�������
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

//���ַ���תΪ�����������ٵ���systemShift����ʮ������
int binaryNum(char code[], int start, int num) {
    int decimalNum = 0, i = 0;
    for (i = 0; i < num; i++) {
        decimalNum = decimalNum * 2 + code[start + i] - '0';
    }
    return decimalNum;
}

//���ݴ���ָ��
void transOrder(int objfront, int objback, int lmmediate,
                int data[], int ax[]) {
    //�������Ĵ��� 
    if (objback == 0) {
        ax[objfront] = lmmediate;
    }
    //�Ĵ���֮��Ĵ���
    else {
        if (objfront <= 4) {
            ax[objfront] = data[(ax[objback] - 16384) / 2];
        }
        else {//objfront >= 5
            data[(ax[objfront] - 16384) / 2] = ax[objback];
        }
    }
}

//��������ָ��
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

//�߼�����ָ��
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

//�Ƚ�ָ��
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
    //�Ĵ��� ֮��Ĵ���
    else {
        if (ax[frontReg] > data[(ax[backReg] - 16384) / 2])
            *flag = 1;
        if (ax[frontReg] == data[(ax[backReg] - 16384) / 2])
            *flag = 0;
        if (ax[frontReg] < data[(ax[backReg] - 16384) / 2])
            *flag = -1;
    }
}

//��תָ��
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

//�������ָ��
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
    //�����  
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
    //���ݶ� 
    printf("\ndataSegment :\n");
    cnt = 0;
    for (i = 0; i <= 16 * 16 - 1; i++) {
        printf("%d ", data[i]);
        cnt++;
        if (cnt % 16 == 0)
            printf("\n");
    }
}
