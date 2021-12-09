#define _CRT_SECURE_NO_WARNINGS
#include<algorithm>
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<time.h>
#include <windows.h>
#define Clock_wait 800

typedef struct LNodeQ {//客户节点
	int ArriveTime;
	int DurTime;
	int mount;//办理业务的金额，负为取，正为存
	struct LNodeQ* next;
}LNodeQ;

typedef struct {
	LNodeQ* head, * tail;
}LinklistQ;//窗口的队列，带头结点

typedef struct LNodeE {//事件的元素
	int OccurTime;
	int NType;//事件类型，0为新客户到达，1为1号窗口客户办理业务，2为号窗口客户办理业务,3为1号窗口离开，4为2号窗口离开
	int WinLocate;//窗口号
	LNodeQ* now_customer;//指针类型，与窗口相一致
	struct LNodeE* next;
}LNodeE;

int TotalMoney, TotalTime, CustomerNum;
int ReMoney;//实际上的银行资金总额，开始办理的取钱业务，取的那一部分钱已经不属于银行现存资金。而存入的钱，只有当存钱业务办理完成后，这笔钱才算作是银行存储的资金。
int CloseTime;
LNodeE* EventList = new LNodeE;
LinklistQ Window1[100];
LinklistQ Window2;
int LastTime;//仅用于二号窗口
int Window1Num;
int cnt = 1;//事件数
char flag;//为y，需要延迟；为n，不需要延迟
int flag_busy;//判断忙碌与否


void OpenForDay();
void EventInsert(LNodeE* ev);//将事件按时间顺序加入到事件表中，且时间相同时，离开早于到达
int GetRandomT();
int GetRandomM();
int GetShortestLine();
void CustomerArrive(LNodeE* q);//类型=0,新客户到来，生成他的业务，选择最短窗口.且若窗口无人，则将他的办理事件加入事件表。生成下一客户到达时间。
void CustomerHandle1(LNodeE* q); //类型=1，某个一号窗口开始办理业务；若取钱，且足够，那么直接取，将其离开事件加入事件表。离开时再将下一人开始办理事件加入事件表；若取钱，但不够，将其加入2号窗口，从当前的窗口删除，并将他的下一个人办理业务加入事件表；若存钱，将其离开事件加入事件表
void DeleteWindow(int WinType, int WinNum);
void CustomerLeave1(LNodeE* q);//类型=3,某一号窗口客户离开，若为晚于closetime，只更新时间，并且不进行以下操作。更新银行钱数，总等待时间，将其下一人办理事件加入到事件表；若为存钱置LastTime，表示第二窗口下一人若能开始办业务，他的开始时间。顺序检查第二窗口，若可取，则将该人开始办理业务事件加入事件表，并从二号窗口删除
void CustomerHandle2(LNodeE* q);//类型=2,二号窗口办理业务，取钱，将其离开事件加入事件表	
void CustomerLeave2(LNodeE* q);//类型=4,二号窗口客户离开，若为晚于closetime，只更新时间，并且不进行以下操作。更新银行钱数，总等待时间


int main()
{
	OpenForDay();
	//任何时刻，各窗口均可以办理业务
	//存钱客户离开时，才处理二号窗口，更符合逻辑，且更公平 
	//原来是事件和客户队列双驱动，较混乱，现改为纯粹事件驱动
	// 
	//每个事件节点，保存当前节点发生时间，类型，客户节点，最后一个在客户离开时用来更新
	// 
	//若离开事件晚于银行关门时间，将其离开事件的时间设置为大于closetime，视为无法完成业务
	// 
	//一号窗口，离开时从当前窗口删除，将下一人办理事件加入事件表
	//二号窗口，加入办理事件时，从当前窗口删除,防止下一次顺序检查时重复处理
	//任何窗口，都是离开时，更新银行总钱数，总等待时长
	// 
	LNodeE* q = EventList->next;
	while (q) {
		if (flag == 'y')	Sleep(Clock_wait);
		if (q->NType == 0)	CustomerArrive(q);
		if (q->NType == 1)	CustomerHandle1(q);
		if (q->NType == 2)	CustomerHandle2(q);
		if (q->NType == 3)	CustomerLeave1(q);
		if (q->NType == 4)	CustomerLeave2(q);
		EventList->next = q->next;
		q = EventList->next;
	}
	LNodeQ* tmp = Window2.head->next;
	while (tmp) {
		TotalTime += CloseTime - tmp->ArriveTime;
		int event_hour = CloseTime / 60 + 6;
		int event_min = CloseTime % 60;
		if (flag == 'y')	Sleep(Clock_wait);
		printf("\n事件%d:	\n", cnt++);
		printf("	%d:%02d	一位客户未能在第二类窗口开始办理取钱%d业务，离开银行，逗留时间：%d分钟\n", event_hour, event_min, -tmp->mount, CloseTime - tmp->ArriveTime);
		printf("	当前银行存有资金总额：%d元\n", ReMoney);
		Window2.head->next = tmp->next;
		free(tmp);
		tmp = Window2.head->next;
	}
	printf("\n事件%d:	\n", cnt++);
	int event_hour = CloseTime / 60 + 6;
	int event_min = CloseTime % 60;
	printf("	%d:%02d	营业结束.共计%d位顾客来办理业务，在银行逗留的总时长为%d分钟，平均逗留时间为%.2f分钟\n", event_hour, event_min, CustomerNum, TotalTime, float(TotalTime) / float(CustomerNum));
	printf("	当前银行存有资金总额：%d元\n", ReMoney);
	//不断取事件表的第一个元素
	//类型=0	
	//	新客户到来，生成他的业务，选择最短窗口，且若窗口无人，则将他的办理事件加入事件表。生成下一客户到达时间。
	// 	   
	//类型=1	需要额外窗口号标号
	//	某个一号窗口开始办理业务
	//		若取钱，且足够，那么直接取，将其离开事件加入事件表。离开时再将下一人开始办理事件加入事件表
	//		若取钱，但不够，将其加入2号窗口，从当前的窗口删除，并将他的下一个人办理业务加入事件表			
	//		若存钱，将其离开事件加入事件表
	//
	//类型=2			
	//	二号窗口办理业务，取钱，将其离开事件加入事件表		
	//	
	//类型=3
	//	某一号窗口客户离开，若为晚于closetime，只更新时间，并且不进行以下操作
	//		更新银行钱数，总等待时间，将其下一人办理事件加入到事件表
	// 		若为存钱置LastTime，表示第二窗口下一人若能开始办业务，他的开始时间
	//		顺序检查第二窗口，若可取，则将该人开始办理业务事件加入事件表，并从二号窗口删除
	// 
	//类型=4
	//	二号窗口客户离开，若为晚于closetime，只更新时间，并且不进行以下操作
	//		更新银行钱数，总等待时间
	// 
	//最后检查各窗口，若有人，使其离开，无法办理业务
}

void OpenForDay()
{
	//初始化各窗口
	//初始化事件表
	//初始化随机数种子
	//第一位客户进入
	srand((unsigned int)time(NULL));
	flag_busy = rand() % 3 + 1;
//	flag_busy = 1;
	if (flag_busy == 1)printf("今天将会是忙碌的一天。\n");
	if (flag_busy == 2)printf("今天将会是普通的一天。\n");
	if (flag_busy == 3)printf("今天将会是清闲的一天。\n");
	printf("请输入第一类窗口的数目：\n");
	scanf("%d", &Window1Num);
	printf("请输入银行初始资金：\n");
	scanf("%d", &ReMoney);
	printf("请输入营业时长（单位：分钟）：\n");
	scanf("%d", &CloseTime);
	getchar();
	printf("是否需要延迟输出事件？y/n\n");
	scanf("%c", &flag);
	Window2.head = Window2.tail = new LNodeQ;
	EventList->next = NULL; Window2.head->next = NULL;
	for (int i = 1; i <= Window1Num; i++) {
		Window1[i].head = Window1[i].tail = new LNodeQ;
		Window1[i].head->next = NULL;
	}
	LNodeE* en = new LNodeE;
	en->now_customer = NULL;	en->NType = 0;	en->OccurTime = 0;	en->next = NULL;	en->WinLocate = 0;
	EventInsert(en);
	if (flag == 'y')	Sleep(Clock_wait);
	printf("\n事件0:	\n");
	printf("	6:00	银行开门，开始营业，营业时长为%d分钟\n", CloseTime);
	printf("	当前银行存有资金总额：%d元\n", ReMoney);
}

void EventInsert(LNodeE* ev)
{//将事件按时间顺序加入到事件表中，且时间相同时，离开早于到达
//	printf("————————%d   %d\n", ev->NType, ev->OccurTime);
//	上一条语句在debug中发挥了巨大作用
	LNodeE* q = EventList->next, * p = EventList;
	int start = time(0);
	while (q) {
		if (q->OccurTime > ev->OccurTime) {
			p->next = ev; ev->next = q;
			return;
		}
		if (q->OccurTime == ev->OccurTime && (ev->NType == 4 || ev->NType == 3)) {
			p->next = ev; ev->next = q;
			return;
		}
		q = q->next;
		p = p->next;
	}
	p->next = ev;
	ev->next = NULL;
}

int GetRandomT()//办理业务时长
{
	int a = 0;
	int N;
	N = (rand() % 111 + rand() % 71) % 15;
	while (a == 0) {
		if (N <= 1)	a = rand() % 15 + 1;
		else if (N <= 4)	a = rand() % 11 + 15;
		else if (N <= 10)	a = rand() % 11 + 25;
		else if (N <= 13)	a = rand() % 11 + 35;
		else a = rand() % 16 + 45;
	}
	return a;
}

int GetRandomT_arrive()
{
	if (flag_busy == 1) return (rand() % 9 + 7);
	if (flag_busy == 2) return (rand() % 19 + 13);
	if (flag_busy == 3) return (rand() % 37 + 23);
}

int GetRandomM()
{
	int a = 0;
	int b = rand() % 2;
	int N = (rand() % 111 + rand() % 71) % 10;
	while (a == 0) {
		if (N <= 1)	a = rand() % 200 + 101;
		else if (N <= 5)	a = rand() % 1800 + 200;
		else if (N <= 7)	a = rand() % 3000 + 2000;
		else if (N <= 8)	a = rand() % 3000 + 5000;
		else a = rand() % 20001;
	}
	if (b == 1) return a;
	else return -a;
}

int GetShortestLine()
{
	int tmp_window = 1;
	int tmp_num = 0;
	int tmp_cmp = 0x3f3f3f;
	LNodeQ* tmp;
	for (int i = 1; i <= Window1Num; i++) {
		tmp = Window1[i].head->next;
		while (tmp) {
			tmp_num++;
			tmp = tmp->next;
		}
		if (tmp_num == 0) {
			return i;
		}
		if (tmp_num < tmp_cmp) {
			tmp_window = i;
			tmp_cmp = tmp_num;
		}
		tmp_num = 0;
	}
	return tmp_window;
}

void CustomerArrive(LNodeE* q)
{
	if (q->OccurTime >= CloseTime)	return;
	printf("\n事件%d:	\n", cnt++);
	++CustomerNum;
	LNodeQ* nexcus = new LNodeQ;
	nexcus->ArriveTime = q->OccurTime;
	int event_hour = nexcus->ArriveTime / 60 + 6;
	int event_min = nexcus->ArriveTime % 60;
	nexcus->DurTime = GetRandomT();
	nexcus->mount = GetRandomM();
	if (nexcus->mount > 0)	printf("	%d:%02d	一位客户到达银行，办理的业务是存钱%d元,需要办理时长为：%d分钟。\n", event_hour, event_min, nexcus->mount, nexcus->DurTime);
	if (nexcus->mount < 0)	printf("	%d:%02d	一位客户到达银行，办理的业务是取钱%d元,需要办理时长为：%d分钟。\n", event_hour, event_min, -nexcus->mount, nexcus->DurTime);
	int now_window = GetShortestLine();
	printf("	该客户在第一类%d号窗口排队等待\n", now_window);
	printf("	当前银行存有资金总额：%d元\n", ReMoney);
	Window1[now_window].tail->next = nexcus;
	Window1[now_window].tail = nexcus;
	Window1[now_window].tail->next = NULL;
	if (Window1[now_window].head->next == Window1[now_window].tail) {
		LNodeE* new_ev = new LNodeE;
		new_ev->NType = 1;	new_ev->next = NULL;	new_ev->now_customer = nexcus;
		new_ev->OccurTime = q->OccurTime;	new_ev->WinLocate = now_window;
		EventInsert(new_ev);
	}
	LNodeE* new_ev = new LNodeE;//产生下一客户到达事件
	new_ev->next = NULL;	new_ev->now_customer = NULL;	new_ev->NType = 0;
	new_ev->OccurTime = q->OccurTime + GetRandomT_arrive();
	if (new_ev->OccurTime < CloseTime)	EventInsert(new_ev);
}

void CustomerHandle1(LNodeE* q) //类型=1，某个一号窗口开始办理业务；若取钱，且足够，那么直接取，将其离开事件加入事件表。离开时再将下一人开始办理事件加入事件表；若取钱，但不够，将其加入2号窗口，从当前的窗口删除，并将他的下一个人办理业务加入事件表；若存钱，将其离开事件加入事件表
{
	int event_hour = q->OccurTime / 60 + 6;
	int event_min = q->OccurTime % 60;
	printf("\n事件%d:	\n", cnt++);
	if (q->now_customer->mount < 0) {
		if (q->now_customer->mount + ReMoney >= 0) {
			ReMoney += q->now_customer->mount;
			printf("	%d:%02d	一位客户在第一类%d号窗口开始办理取钱%d元业务\n", event_hour, event_min, q->WinLocate, -q->now_customer->mount);
			printf("	当前银行存有资金总额：%d元\n", ReMoney);
			LNodeE* new_ev = new LNodeE;
			new_ev->next = NULL;	new_ev->now_customer = q->now_customer;
			new_ev->NType = 3;		new_ev->OccurTime = q->OccurTime + q->now_customer->DurTime;
			new_ev->WinLocate = q->WinLocate;
			EventInsert(new_ev);
		}
		else {
			printf("	%d:%02d	一位客户在第一类%d号窗口开始办理取钱%d元业务\n", event_hour, event_min, q->WinLocate, -q->now_customer->mount);
			printf("	但是当前银行存有资金总额不足，该客户前往第二类窗口等待。\n");
			printf("	当前银行存有资金总额：%d元\n", ReMoney);
			if (Window1[q->WinLocate].head->next->next) {
				LNodeE* new_ev = new LNodeE;
				new_ev->next = NULL;	new_ev->now_customer = Window1[q->WinLocate].head->next;
				new_ev->NType = 1;		new_ev->OccurTime = q->OccurTime;
				new_ev->WinLocate = q->WinLocate;
				EventInsert(new_ev);
			}
			DeleteWindow(1, q->WinLocate);
			Window2.tail->next = q->now_customer;
			Window2.tail = q->now_customer;
			Window2.tail->next = NULL;
		}
	}
	else {//存钱
		printf("	%d:%02d	一位客户在第一类%d号窗口开始办理存钱%d元业务\n", event_hour, event_min, q->WinLocate, q->now_customer->mount);
		printf("	当前银行存有资金总额：%d元\n", ReMoney);
		LNodeE* new_ev = new LNodeE;
		new_ev->next = NULL;	new_ev->now_customer = q->now_customer;
		new_ev->NType = 3;		new_ev->OccurTime = q->OccurTime + q->now_customer->DurTime;
		new_ev->WinLocate = q->WinLocate;
		EventInsert(new_ev);
	}
}

void CustomerLeave1(LNodeE* q)
{
	LNodeQ* tmp = Window1[q->WinLocate].head->next;
	if (q->OccurTime > CloseTime) {
		TotalTime += CloseTime - tmp->ArriveTime;
		int event_hour = CloseTime / 60 + 6;
		int event_min = CloseTime % 60;
		if (q->now_customer->mount < 0) {
			ReMoney = ReMoney - q->now_customer->mount;
			printf("\n事件%d:	\n", cnt++);
			printf("	%d:%02d	一位客户未能在第一类%d号窗口办理完取钱%d元业务，离开银行，逗留时间：%d分钟\n", event_hour, event_min, q->WinLocate, -q->now_customer->mount, CloseTime - tmp->ArriveTime);
			printf("	当前银行存有资金总额：%d元\n", ReMoney);
		}
		else {
			printf("\n事件%d:	\n", cnt++);
			printf("	%d:%02d	一位客户未能在第一类%d号窗口办理完存钱%d元业务，离开银行，逗留时间：%d分钟\n", event_hour, event_min, q->WinLocate, q->now_customer->mount, CloseTime - tmp->ArriveTime);
			printf("	当前银行存有资金总额：%d元\n", ReMoney);
		}
		if (Window1[q->WinLocate].head->next->next) {//之后排在这个窗口的，直接加离开事件
			LNodeE* new_ev = new LNodeE;
			new_ev->next = NULL;	new_ev->now_customer = Window1[q->WinLocate].head->next;
			new_ev->NType = 3;		new_ev->OccurTime = q->OccurTime;
			new_ev->WinLocate = q->WinLocate;
			EventInsert(new_ev);
		}
		DeleteWindow(1, q->WinLocate);
	}
	else {
		TotalTime += q->OccurTime - tmp->ArriveTime;
		int event_hour = q->OccurTime / 60 + 6;
		int event_min = q->OccurTime % 60;
		if (tmp->mount > 0) {
			ReMoney += tmp->mount;
			printf("\n事件%d:	\n", cnt++);
			printf("	%d:%02d	一位客户在第一类%d号窗口办理完存钱%d元业务，离开银行，逗留时间：%d分钟\n", event_hour, event_min, q->WinLocate, q->now_customer->mount, q->OccurTime - tmp->ArriveTime);
			printf("	当前银行存有资金总额：%d元\n", ReMoney);
			LastTime = LastTime > q->OccurTime ? LastTime : q->OccurTime;
			LNodeQ* tmp_win2 = Window2.head->next;//遍历第二个窗口的客户
			LNodeQ* pre = Window2.head;//跟随指针
			while (tmp_win2 && LastTime < CloseTime) {
				if (tmp_win2->mount + ReMoney >= 0) {//假如当前客户的取钱请求可以满足,则将第二个窗口的办理时间加入事件表
					LNodeE* en = new LNodeE;
					en->NType = 2;		en->OccurTime = LastTime;
					en->next = NULL;	en->now_customer = tmp_win2;
					en->WinLocate = 1;
					if (en->OccurTime < CloseTime) {
						EventInsert(en);
						LastTime += tmp_win2->DurTime;
						ReMoney += tmp_win2->mount;
					}
					pre->next = tmp_win2->next;
					tmp_win2 = pre->next;
					if (!tmp_win2)	Window2.tail = pre;//这里很重要
				}
				else {
					pre = pre->next;
					tmp_win2 = tmp_win2->next;
				}
			}
		}
		else {
			printf("\n事件%d:	\n", cnt++);
			printf("	%d:%02d	一位客户在第一类%d号窗口办理完取钱%d元业务，离开银行，逗留时间：%d分钟\n", event_hour, event_min, q->WinLocate, -q->now_customer->mount, q->OccurTime - tmp->ArriveTime);
			printf("	当前银行存有资金总额：%d元\n", ReMoney);
		}
		DeleteWindow(1, q->WinLocate);
		if (Window1[q->WinLocate].head->next) {
			LNodeE* new_ev = new LNodeE;
			new_ev->next = NULL;	new_ev->now_customer = Window1[q->WinLocate].head->next;
			new_ev->NType = 1;		new_ev->OccurTime = q->OccurTime;
			new_ev->WinLocate = q->WinLocate;
			EventInsert(new_ev);
		}
	}
	free(q->now_customer);
}

void DeleteWindow(int WinType, int WinNum)
{
	if (WinType == 1) {
		if (Window1[WinNum].head->next == Window1[WinNum].tail) {
			Window1[WinNum].tail = Window1[WinNum].head;
			Window1[WinNum].head->next = Window1[WinNum].tail->next = NULL;
		}
		else {
			Window1[WinNum].head->next = Window1[WinNum].head->next->next;
		}
	}
	else {
		if (Window2.head->next == Window2.tail) {
			Window2.tail = Window2.head;
			Window2.head->next = Window2.tail->next = NULL;
		}
		else {
			Window2.head->next = Window2.head->next->next;
		}
	}
}

void CustomerHandle2(LNodeE* q)//类型=2,二号窗口办理业务，取钱，将其离开事件加入事件表		
{
	int event_hour = q->OccurTime / 60 + 6;
	int event_min = q->OccurTime % 60;
	printf("\n事件%d:	\n", cnt++);
	printf("	%d:%02d	一位客户在第二类窗口开始办理取钱%d元业务\n", event_hour, event_min, -q->now_customer->mount);
	printf("	当前银行存有资金总额：%d元\n", ReMoney);
	LNodeE* new_ev = new LNodeE;
	new_ev->next = NULL;	new_ev->now_customer = q->now_customer;
	new_ev->NType = 4;		new_ev->OccurTime = q->OccurTime + q->now_customer->DurTime;
	new_ev->WinLocate = q->WinLocate;
	EventInsert(new_ev);
}

void CustomerLeave2(LNodeE* q)
{
	LNodeQ* tmp = q->now_customer;
	if (q->OccurTime > CloseTime) {
		TotalTime += CloseTime - tmp->ArriveTime;
		int event_hour = CloseTime / 60 + 6;
		int event_min = CloseTime % 60;
		ReMoney = ReMoney - q->now_customer->mount;
		printf("\n事件%d:	\n", cnt++);
		printf("	%d:%02d	一位顾客未能在第二类窗口办理完取钱%d元业务，离开银行，逗留时间：%d分钟\n", event_hour, event_min, -q->now_customer->mount, CloseTime - tmp->ArriveTime);
		printf("	当前银行存有资金总额：%d元\n", ReMoney);
	}
	else {
		TotalTime += q->OccurTime - tmp->ArriveTime;
		int event_hour = q->OccurTime / 60 + 6;
		int event_min = q->OccurTime % 60;
		printf("\n事件%d:	\n", cnt++);
		printf("	%d:%02d	一位顾客在第二类窗口办理完取钱%d元业务，离开银行，逗留时间：%d分钟\n", event_hour, event_min, -q->now_customer->mount, q->OccurTime - tmp->ArriveTime);
		printf("	当前银行存有资金总额：%d元\n", ReMoney);
	}
	free(q->now_customer);
}

//通常而言，越忙碌，需要开设的第一类窗口数目越多
//还有一种极端情况会导致客户平均等待时间较长，那就是取钱人多，有人需要取钱的金额过大，导致一直等待
