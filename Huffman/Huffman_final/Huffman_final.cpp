#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<fstream>
using namespace std;

typedef struct {
	int weight;
	int parent, lchild, rchild;
}HTNode;
typedef HTNode* HuffTree;

char Order;
unsigned char Char[300];//从1开始存储
int w_char[300];
int num_char, num_0, num_8;//字符数，补零数，完整的比特块
char HC[300][300];

bool Cal_weight(char** argv);//计数每个字符的出现频数
void Selete(HuffTree HT, int i, int& s1, int& s2);//选取当前权值最小的两个结点
void InitHuffman(HuffTree& HT);//建立Huffman树

void HuffmanCoding(HuffTree HT, int root, char stack[], int flag, int top);//寻找每个字符的Huffman编码
void Cal_unsigned(char zero_one[], unsigned char& a);//计算每个比特块的对应的二进制8bits数值
void EnCoding(char** argv);//Huffman编码
void Store_all(HuffTree HT);//存储完整的信息，附加信息在文件头

void Read_Text(HuffTree& HT);//将得到的文件进行拆分，为后续解码做准备，建立Huffman树
void Load_Info(HuffTree& HT);//更新Char[]文件
bool HuffmanDeCoding(FILE* fp1, FILE* fp2, HuffTree HT, int root);//面向01序列，进行Huffman解码
void DeCoding(HuffTree HT, char** argv);//先将二进制文件转化为01序列，再将01序列进行Huffman解码

void show_menu();

void Tree_Print(HuffTree HT, int root, int cnt);

int main(int argc, char** argv) 
{
	HuffTree HT = NULL;
	while (1)
	{
		show_menu();
		scanf("%c", &Order);
		char tmps; scanf("%c", &tmps);
		if (Order == 'E' || Order == 'e') {
			if (!Cal_weight(argv)) {//预处理原文件，统计字符出现次数
				return 0;
			}
			InitHuffman(HT);//建立Huffman树
			char stack[300];
			HuffmanCoding(HT, 2 * num_char - 1, stack, 2, 0);//求叶子节点的Huffman编码
			EnCoding(argv);
			Store_all(HT);
			cout << "编码已完成！" << endl;
			continue;
		}
		if (Order == 'D' || Order == 'd') {
			Read_Text(HT);
			Load_Info(HT);
			DeCoding(HT, argv);
			cout << "解码已完成！" << endl;
			continue;
		}
		if (Order == 'P' || Order == 'p') {
			Tree_Print(HT,2*num_char-1,0);
			cout << "Huffman树打印成功！" << endl;
			continue;
		}
		if (Order == 'Q' || Order == 'q') {
			cout << "程序退出" << endl;
			return 0;
		}
		else {
			cout << "无效的指令，请重新输入。" << endl;
		}
	}
	return 0;
	//在压缩阶段，需要完成个任务：
	//1.通过字符型读取原文件，设置数组char表示所有的字符，num_char记录已存储的字符个数，w_char数组表示出现多少次
	//2.建立Huffman树
	//3.对原文件进行压缩，二进制写入最终文件
	//4.将附加信息存入一个文件中 
	//
	//在解压阶段，需要完成
	//1.读附件信息文件，建立Huffman树
	//2.通过二进制读取压缩文件，处理，写入到新文件
}

bool Cal_weight(char** argv)
{
	ifstream inFile(argv[1], ios::in | ios::binary); //二进制读方式打开
	if (!inFile) {
		cout << "error" << endl;
		return 0;
	}
	unsigned char p;
	while (inFile.read((char*)&p, sizeof(p))) { //一直读到文件结束
		int find_flag = 0;
		for (int i = 1; i <= num_char; i++) {
			if (Char[i] == p) {
				find_flag = 1;
				w_char[i]++;
				break;
			}
		}
		if (!find_flag) {
			num_char++;
			Char[num_char] = p;
			w_char[num_char]++;
		}
	}
	inFile.close();
	return 1;
}

void Selete(HuffTree HT, int i, int& s1, int& s2)
{//在1...i-1中选取父节点为0，并且权值最小的两个结点
	s1 = s2 = 0;
	HT[0].weight = 0xfffffff;
	for (int j = 1; j < i; j++) {
		if (HT[j].parent) continue;
		if (HT[j].weight < HT[s1].weight) {
			s2 = s1;
			s1 = j;
		}
		else if (HT[j].weight < HT[s2].weight) {
			s2 = j;
		}
	}
}

void InitHuffman(HuffTree& HT)
{
	int m = num_char * 2 - 1;//树的节点个数
	HT = new HTNode[m + 1];
	for (int i = 1; i <= m; i++) {
		HT[i].weight = (i <= num_char ? w_char[i] : 0);
		HT[i].lchild = HT[i].rchild = HT[i].parent = 0;
	}
	for (int i = num_char + 1; i <= m; i++) {
		int s1, s2;
		Selete(HT, i, s1, s2);//在1...i-1中选取父节点为0，并且权值最小的两个结点
		HT[i].lchild = s1;
		HT[i].rchild = s2;
		HT[i].weight = HT[s1].weight + HT[s2].weight;
		HT[s1].parent = HT[s2].parent = i;
	}
}

void HuffmanCoding(HuffTree HT, int root, char stack[], int flag, int top)
{//用栈来记录01序列，flag==0,向左儿子走，flag==1,向右儿子走
	if (flag == 0) {
		stack[top] = '0';
		top++;
	}
	if (flag == 1) {
		stack[top] = '1';
		top++;
	}
	if (HT[root].lchild == 0) {//当前节点为叶子节点
		for (int i = 0; i < top; i++)
			HC[root][i] = stack[i];
		HC[root][top] = '\0';
		top--;
		return;
	}
	HuffmanCoding(HT, HT[root].lchild, stack, 0, top);
	HuffmanCoding(HT, HT[root].rchild, stack, 1, top);
}

void Cal_unsigned(char zero_one[], unsigned char& a)
{
	for (int i = 0; i < 8; i++)
	{
		a = a * 2;
		if (zero_one[i] == '1') a++;
	}
}

void EnCoding(char** argv)
{
	ifstream inFile(argv[1], ios::in | ios::binary);
	ofstream outFile("CodeFile1.txt", ios::out | ios::binary);//单纯存储Huffman编码后的文件
	char zero_one[10];//临时存储01编码序列
	int cnt_01 = 0;
	unsigned char a = 0;
	unsigned char tmp;
	while (inFile.read((char*)&tmp, sizeof(tmp))) {
		for (int i = 1; i <= num_char; i++) {
			if (tmp == Char[i]) {
				for (int j = 0; HC[i][j] != '\0'; j++) {
					zero_one[cnt_01] = HC[i][j];
					cnt_01++;
					if (cnt_01 == 8) {//达到一个字节，可以存入
						Cal_unsigned(zero_one, a);
						outFile.write((char*)&a, sizeof(a));
						a = 0;
						cnt_01 = 0;
						num_8++;
					}
				}
			}
		}
	}
	if (cnt_01) {//不够一个字节，末尾补零
		num_0 = 8 - cnt_01;
		for (int i = 0; i < cnt_01; i++)
		{
			a = a * 2;
			if (zero_one[i] == '1') a++;
		}
		for (int i = 0; i < num_0; i++)
			a = a * 2;
		outFile.write((char*)&a, sizeof(a));
	}
	inFile.close();
	outFile.close();
}

bool HuffmanDeCoding(FILE* fp1, FILE* fp2, HuffTree HT, int root)
{
	char p;
	if (!fp1 || !fp2) {
		cout << "error";
		return 0;
	}
	if (HT[root].lchild == 0) {
		fprintf(fp2, "%c", Char[root]);
		return 1;
	}
	else {
		p = fgetc(fp1);
		if (p == EOF) return 0;
	}
	if (p == '0') HuffmanDeCoding(fp1, fp2, HT, HT[root].lchild);
	if (p == '1') HuffmanDeCoding(fp1, fp2, HT, HT[root].rchild);
	return 1;
}

void DeCoding(HuffTree HT, char** argv)
{
	FILE* fp, * fp2;
	fp = fopen("CodeFile2.txt", "rb");
	fp2 = fopen("TempTxt.txt", "w");
	unsigned char p = fgetc(fp);
	while (num_8) {
		int cnt = 128;
		int a = (int)p;
		while (cnt) {
			if (a >= cnt) {
				a = a - cnt;
				fputc('1', fp2);
			}
			else {
				fputc('0', fp2);
			}
			cnt = cnt / 2;
		}
		p = fgetc(fp);
		num_8--;
	}
	if (num_0) {
		int cnt = 128;
		int tmp = 8;
		int a = (int)p;
		while (tmp > num_0) {
			if (a >= cnt) {
				a = a - cnt;
				fputc('1', fp2);
			}
			else {
				fputc('0', fp2);
			}
			cnt = cnt / 2;
			tmp--;
		}
	}
	fclose(fp);
	fclose(fp2);
	FILE* fp1;
	fp1 = fopen("TempTxt.txt", "r");
	fp2 = fopen(argv[2], "wb");
	while (HuffmanDeCoding(fp1, fp2, HT, 2 * num_char - 1));
	fclose(fp1);
	fclose(fp2);
}

void Store_all(HuffTree HT)
{
	FILE* fp1;
	fp1 = fopen("CodeFile.txt", "w");
	fprintf(fp1, "%d %d ", num_8, num_0);
	fprintf(fp1, "%d ", num_char);
	for (int i = 1; i <= 2 * num_char - 1; i++)
	{
		fprintf(fp1, "%d %d %d %d ", HT[i].weight, HT[i].parent, HT[i].lchild, HT[i].rchild);
	}
	char tmpp = EOF;
	fprintf(fp1, "%c", tmpp);//前后间断点
	fclose(fp1);

	fp1 = fopen("CodeFile.txt", "ab");
	for (int i = 1; i <= num_char; i++) fprintf(fp1, "%c", Char[i]);

	ifstream inFile("CodeFile1.txt", ios::in | ios::binary);
	unsigned char tmp;
	while (inFile.read((char*)&tmp, sizeof(tmp))) {
		fprintf(fp1, "%c", tmp);
	}
	inFile.close();
	fclose(fp1);
}

void Read_Text(HuffTree& HT)
{
	FILE* fp1, * fp2;
	fp1 = fopen("Pre_Info1.txt", "wb");
	ifstream inFile("CodeFile.txt", ios::in | ios::binary);
	unsigned char tmp;
	while (inFile.read((char*)&tmp, sizeof(tmp))) {
		if ((int)tmp == 255) break;
		fprintf(fp1, "%c", tmp);
	}
	fclose(fp1);

	fp1 = fopen("Pre_Info1.txt", "r");
	fscanf(fp1, "%d%d%d", &num_8, &num_0, &num_char);
	HT = new HTNode[2 * num_char];
	for (int i = 1; i <= 2 * num_char - 1; i++) {
		fscanf(fp1, "%d%d%d%d", &HT[i].weight, &HT[i].parent, &HT[i].lchild, &HT[i].rchild);
	}
	fclose(fp1);

	fp2 = fopen("Pre_Info2.txt", "wb");
	for (int i = 1; i <= num_char; i++) {
		inFile.read((char*)&tmp, sizeof(tmp));
		fprintf(fp2, "%c", tmp);
	}
	fclose(fp2);

	ofstream outFile("CodeFile2.txt", ios::out | ios::binary);//单纯存储Huffman编码后的文件
	while (inFile.read((char*)&tmp, sizeof(tmp))) {
		outFile.write((char*)&tmp, sizeof(tmp));
	}
	inFile.close();
	outFile.close();
}
\
void Load_Info(HuffTree& HT)
{
	ifstream inFile("Pre_Info2.txt", ios::in | ios::binary);
	unsigned char tmp;
	int cnt_char = 1;
	while (inFile.read((char*)&tmp, sizeof(tmp))) {
		Char[cnt_char] = (unsigned char)tmp;
		cnt_char++;
	}
	inFile.close();
}

void show_menu()
{
	cout << "========HuffmanCoding========" << endl;
	cout << "=====E:对文件编码============" << endl;
	cout << "=====D:对文件解码============" << endl;
	cout << "=====P:打印Huffman树=========" << endl;
	cout << "=====Q:退出编码系统==========" << endl;
	cout << "请输入指令：";
}

void Tree_Print(HuffTree HT, int root, int cnt)
{
	for (int i = 1; i <= cnt; i++)
		cout << "     ";
	if (root <= num_char) cout << (int)Char[root];
	else cout << root;
	cout << endl;
	if (HT[root].lchild == 0) return;
	Tree_Print(HT, HT[root].lchild, cnt + 1);
	Tree_Print(HT, HT[root].rchild, cnt + 1);
	return;
}
