#define _CRT_SECURE_NO_WARNINGS
#include<algorithm>
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<time.h>

typedef struct LNodeQ {//两个窗口的队列的元素，存储的是客户，按照先来后到顺序排列
	int ArriveTime;//到达时间
	int DurTime;//办理业务需要的时间
	int mount;//办理业务的金额，负为取，正为存
	struct LNodeQ* next;
}LNodeQ;

typedef struct {
	LNodeQ* head, * tail;
}LinklistQ;//窗口的队列

typedef struct LNodeE {//事件的元素
	int OccurTime;
	int NType;//事件类型，0为新客户到达，1为1号窗口客户办理业务，2为号窗口客户办理业务
	struct LNodeE* next;
}LNodeE;

int total;
int closetime;
LNodeE* EventList = new LNodeE;
LinklistQ Window1;
LinklistQ Window2;
int TotalTime, CustomerNum;
int flag = 1;//第二个窗口无法再办理下一个业务的标记，即有办不完的人一直占用窗口
int flag2 = 1;//第二个窗口无法再办理下一个业务的标记，即有办不完的人一直占用窗口
int cnt = 1;//事件数
int wait;//在考虑第二窗口办理时，需要额外等待的时间
int Lasttime;//在第二窗口办理业务时，即使第一窗口又来了人，且第一窗口只有一人，这一人也需要等到第二窗口办完才可以办理

void EventInsert(LNodeE en)
{//将某事件按时间由小到大的顺序插入到时间表中
	printf("————————%d   %d\n", en.NType, en.OccurTime);
	//上一条语句在debug中发挥了巨大作用
	LNodeE* q = EventList->next, * p = EventList;
	LNodeE* ev = new LNodeE;
	ev->NType = en.NType; ev->OccurTime = en.OccurTime;
	while (q) {
		if (q->OccurTime > en.OccurTime) {
			p->next = ev; ev->next = q;
			return;
		}
		if (q->OccurTime == en.OccurTime && (en.NType==4||en.NType==3)) {
			p->next = ev; ev->next = q;
			return;
		}//处理二号窗口走，下一位存钱客户同时进来的bug
		q = q->next;
		p = p->next;
	}
	p->next = ev;
	ev->next = NULL;
}

void OpenForDay()
{//初始化
	printf("请输入银行初始资金：\n");
	scanf("%d", &total);
	printf("请输入营业时长（单位：分钟）：\n");
	scanf("%d", &closetime);
	Window1.head = Window1.tail = new LNodeQ;
	Window2.head = Window2.tail = new LNodeQ;
	EventList->next = NULL; Window1.head->next = NULL; Window2.head->next = NULL;
	TotalTime = CustomerNum = 0;
	LNodeE en;
	en.NType = 0; en.OccurTime = 0;
	EventInsert(en);
	srand((unsigned int)time(NULL));
	printf("\n事件0:	");
	printf("6:00	银行开门，开始营业，营业时长为%d分钟\n", closetime);
	printf("		当前银行存有资金总额：%d元\n", total);
}

void PrintEventList()
{
	LNodeE* q = EventList->next;
	while (q) {
		printf("发生时间为：%d  事件类型为：%d\n", q->OccurTime, q->NType);
		q = q->next;
	}
}

bool EventEmpty()
{//时间表为空，返回1；时间表不空，返回0
	if (EventList->next) return 0;
	return 1;
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

int GetRandomM()
{
	int a = 0;
	int b = rand() % 2;
	int N = (rand() % 111 + rand() % 71) % 10;
	while (a == 0) {
		if (N <= 1)	a = rand() % 200 + 1;
		else if (N <= 5)	a = rand() % 1800 + 200;
		else if (N <= 7)	a = rand() % 3000 + 2000;
		else if (N <= 8)	a = rand() % 3000 + 5000;
		else a = rand() % 20001;
	}
	if (b == 1) return a;
	else return -a;
}

void CustomerArrive(LNodeE* q)
{
	printf("\n事件%d:	", cnt++);
	++CustomerNum;
	LNodeQ* nexcus = new LNodeQ;
	nexcus->ArriveTime = q->OccurTime;
	int event_hour = nexcus->ArriveTime / 60 + 6;
	int event_min = nexcus->ArriveTime % 60;
	printf("请输入办理时长：\n");
	scanf("%d", &nexcus->DurTime);
	printf("请输入办理金额：\n");
	scanf("%d", &nexcus->mount);
	if (nexcus->mount > 0)	printf("%d:%02d	一位顾客到达银行，办理的业务是存钱%d元,需要办理时长为：%d分钟。\n", event_hour, event_min, nexcus->mount, nexcus->DurTime);
	if (nexcus->mount < 0)	printf("%d:%02d	一位顾客到达银行，办理的业务是取钱%d元,需要办理时长为：%d分钟。\n", event_hour, event_min, -nexcus->mount, nexcus->DurTime);
	printf("		当前银行存有资金总额：%d元\n", total);
	Window1.tail->next = nexcus;
	Window1.tail = nexcus;
	Window1.tail->next = NULL;
	LNodeE en;
	en.OccurTime = q->OccurTime > Lasttime ? q->OccurTime : Lasttime;
	if (Window1.head->next == Window1.tail && en.OccurTime < closetime) {//第一个队伍只有一个顾客，需将这个顾客的办业务事件加入事件表

		en.NType = 1;
		EventInsert(en);
	}
	printf("请输入下一顾客到达时间：\n");
	scanf("%d", &en.OccurTime);
	en.NType = 0;
	if (en.OccurTime < closetime) EventInsert(en);
}

void DeleteWindow(int windownum, LNodeQ& customer)
{
	if (windownum == 1) {
		customer.ArriveTime = Window1.head->next->ArriveTime;
		customer.DurTime = Window1.head->next->DurTime;
		customer.mount = Window1.head->next->mount;
		if (Window1.head->next == Window1.tail) { Window1.head->next = NULL; Window1.tail = Window1.head; }
		else { Window1.head->next = Window1.head->next->next; }
	}
	if (windownum == 2) {
		customer.ArriveTime = Window2.head->next->ArriveTime;
		customer.DurTime = Window2.head->next->DurTime;
		customer.mount = Window2.head->next->mount;
		if (Window2.head->next == Window2.tail) { Window2.head->next = NULL; Window2.tail = Window2.head; }
		else { Window2.head->next = Window2.head->next->next; }
	}
}

void GetWindow(int windownum, LNodeQ& customer)
{
	if (windownum == 1) {
		customer.ArriveTime = Window1.head->next->ArriveTime;
		customer.DurTime = Window1.head->next->DurTime;
		customer.mount = Window1.head->next->mount;
	}
	if (windownum == 2) {
		customer.ArriveTime = Window2.head->next->ArriveTime;
		customer.DurTime = Window2.head->next->DurTime;
		customer.mount = Window2.head->next->mount;
	}
}

void CustomerHandle(LNodeE* q)//知道第几个窗口，以及发生时间，因为是开始办理业务，这里不计算总时间，银行总钱数也不会改变
{
	LNodeQ customer;
	GetWindow(q->NType, customer);//在窗口队列中获取第一个元素,保存在customer中，即customer要办理业务
	if (q->NType == 1)
		if (flag == 0 || q->OccurTime + customer.DurTime > closetime) {//第一个窗口办理不完，同时也会一直占用这个窗口
			flag = 0;
			LNodeE en;
			en.NType = 3; en.OccurTime = closetime + 1;
			EventInsert(en);
			return;
		}
	if (q->NType == 2)
		if (flag2 == 0 || q->OccurTime + customer.DurTime > closetime) {//第一个窗口办理不完，同时也会一直占用这个窗口
			flag2 = 0;
			LNodeE en;
			en.NType = 4; en.OccurTime = closetime + 1;
			EventInsert(en);
			return;
		}
	if (q->NType == 1) {//在第一个窗口办业务
		if (customer.mount >= 0) {
			printf("当前处于存钱客户在第一窗口办理业务。\n");
			wait = customer.DurTime;//此标记为，假如第二个窗口可以办业务，那么第一个窗口的下一个顾客需要再等待的时间
			int event_hour = q->OccurTime / 60 + 6;
			int event_min = q->OccurTime % 60;
			printf("\n事件%d:	", cnt++);
			printf("%d:%02d	一位顾客开始办理存钱%d元业务\n", event_hour, event_min, customer.mount);
			printf("		当前银行存有资金总额：%d元\n", total);
			int total_i = total;
			total += customer.mount;//第一个窗口发生了存事件，则立即处理第二个窗口的取事件
			//但是这里的存的金额需要恢复，需要等到办理完业务，这笔钱才算完全存入。
			LNodeE en;
			en.NType = 3; en.OccurTime = q->OccurTime + customer.DurTime;
			EventInsert(en);
			LNodeQ* tmp = Window2.head->next;//遍历第二个窗口的客户
			int Window2_num = 0;
			while (tmp) {
				Window2_num++;
				tmp = tmp->next;
			}
			printf("Window2_num = %d\n", Window2_num);
			tmp = Window2.head->next;
			LNodeQ* pre = Window2.head;//跟随指针
			while (Window2_num) {
				printf("tmp->mount = %d\n", tmp->mount);
				if (tmp->mount + total >= 0) {//假如当前客户的取钱请求可以满足,则将第二个窗口的办理时间加入事件表
					LNodeE en;
					en.NType = 2; en.OccurTime = q->OccurTime + wait;//这里也是wait
					if (en.OccurTime < closetime)	EventInsert(en);
					wait += tmp->DurTime;
					total = total + tmp->mount;
					if (total <= total_i)	break;
					else {
						tmp = tmp->next;
						pre = pre->next;
						Window2_num--;
					}
				}
				else {//当前顾客的取钱请求无法满足，则将其移动到队伍末尾
					pre->next = tmp->next;
					Window2.tail->next = tmp;
					Window2.tail = tmp;
					Window2.tail->next = NULL;//这里很重要
					tmp = pre->next;//跟随指针的作用在这里体现
					Window2_num--;
				}
			}
			total = total_i;//第二窗口新加的可以办业务的顾客还没有开始办业务，故还没有取钱
			printf("当前处于存钱客户在第一窗口办理业务的预处理结束。\n");
		}
		if (customer.mount < 0) {
			printf("当前处于取钱客户在第一窗口办理业务。\n");
			if (total + customer.mount >= 0) {
				printf("当前处于取钱客户在第一窗口办理业务，可以取钱。\n");
				wait = customer.DurTime;
				LNodeE en;
				en.NType = 3; en.OccurTime = q->OccurTime + customer.DurTime;
				EventInsert(en);
				int event_hour = q->OccurTime / 60 + 6;
				int event_min = q->OccurTime % 60;
				printf("\n事件%d:	", cnt++);
				printf("%d:%02d	一位顾客开始在第一窗口办理取钱%d元业务\n", event_hour, event_min, -customer.mount);
				printf("		当前银行存有资金总额：%d元\n", total);
			}
			else {//取钱，不够，将这个顾客加入第二个队列中
				printf("当前处于取钱客户在第一窗口办理业务，不可以取钱。\n");
				DeleteWindow(q->NType, customer);//这里很重要，调了很久的bug，这已经从第一队删除了
				LNodeQ* tmp = new LNodeQ;
				tmp->ArriveTime = customer.ArriveTime;
				tmp->DurTime = customer.DurTime;
				tmp->mount = customer.mount;
				Window2.tail->next = tmp;
				Window2.tail = tmp;
				Window2.tail->next = NULL;
				//以下步骤一开始被忽略了。当前这个人无法取钱，但是在他之后仍有人排队，需要将他之后的人加入事件中
				//且下一个人开始办理业务的时间与这个人相同
				if (Window1.head->next) {
					LNodeE en;
					en.NType = 1;
					en.OccurTime = q->OccurTime > Lasttime ? q->OccurTime : Lasttime;
					EventInsert(en);
				}
			}
		}
		Lasttime = q->OccurTime + wait;
		wait = 0;//这里很重要
		printf("Lasttime = %d\n", Lasttime);
		printf("至此，处理完了第一个窗口顾客的全部请求，要么顾客离开，要么进入了第二个队列\n");
		//至此，处理完了第一个窗口顾客的全部请求，要么顾客离开，要么进入了第二个队列
	}
	else if (q->NType == 2) {//第二窗口的顾客办理业务，注意，他们还没取钱，在队列中已经删除
		printf("当前处于取钱客户在第二窗口办理业务，可以取钱\n");
		LNodeE en;
		en.NType = 4;
		en.OccurTime = q->OccurTime + customer.DurTime;
		int event_hour = q->OccurTime / 60 + 6;
		int event_min = q->OccurTime % 60;
		printf("\n事件%d:	", cnt++);
		printf("%d:%02d	一位顾客开始在第二窗口办理取钱%d元业务\n", event_hour, event_min, -customer.mount);
		printf("		当前银行存有资金总额：%d元\n", total);
		EventInsert(en);
	}
}

void CustomerLeave(LNodeE* q)//当前窗口的顾客办理完业务，要离开
{
	LNodeQ customer;
	DeleteWindow(q->NType - 2, customer);//在窗口队列中删除第一个元素,保存在customer中，即customer要办理业务
	if (q->NType == 3) {
		if (q->OccurTime <= closetime)	total += customer.mount;
		TotalTime += q->OccurTime - customer.ArriveTime;
		if (Window1.head->next) {
			LNodeE en;
			en.NType = 1;
			en.OccurTime = q->OccurTime > Lasttime ? q->OccurTime : Lasttime;
			EventInsert(en);
		}
	}
	if (q->NType == 4) {
		if (q->OccurTime <= closetime) total += customer.mount;
		TotalTime += q->OccurTime - customer.ArriveTime;
	}
	int event_hour = q->OccurTime / 60 + 6;
	int event_min = q->OccurTime % 60;
	printf("\n事件%d:	", cnt++);
	if (q->OccurTime <= closetime)
		printf("%d:%02d	一位顾客办理完业务，离开银行，逗留时间：%d\n", event_hour, event_min, q->OccurTime - customer.ArriveTime);
	else {
		event_hour = closetime / 60 + 6;
		event_min = closetime % 60;
		printf("%d:%02d	一位顾客未能办理业务，离开银行，逗留时间：%d\n", event_hour, event_min, q->OccurTime - customer.ArriveTime);
	}
	printf("		当前银行存有资金总额：%d元\n", total);

}

//队列均为带头结点
int main()
{//办理业务与离开银行需分开处理，不然会出现bug
	OpenForDay();
	LNodeE* q = EventList->next;
	while (!EventEmpty()) {
		if (q->NType == 0) {
			CustomerArrive(q);
		}
		else if (q->NType == 3 || q->NType == 4) {
			CustomerLeave(q);
		}
		else {
			CustomerHandle(q);
		}
		EventList->next = q->next;
		q = q->next;
	}
	LNodeQ* tmp = Window2.head->next;//遍历第二个窗口的客户
	LNodeQ customer;
	while (tmp) {
		DeleteWindow(2, customer);
		TotalTime += closetime - customer.ArriveTime;
		int event_hour = closetime / 60 + 6;
		int event_min = closetime % 60;
		printf("\n事件%d:	", cnt++);
		printf("%d:%02d	一位顾客未能办理业务，离开银行，逗留时间：%d\n", event_hour, event_min, closetime - customer.ArriveTime);
		printf("		当前银行存有资金总额：%d元\n", total);
		tmp = tmp->next;
	}
	printf("\n事件%d:	", cnt++);
	int event_hour = closetime / 60 + 6;
	int event_min = closetime % 60;
	printf("%d:%02d	营业结束.共计%d位顾客来办理业务，在银行逗留的总时长为%d分钟，平均逗留时间为%.2f分钟\n", event_hour, event_min, CustomerNum, TotalTime, float(TotalTime) / float(CustomerNum));
	printf("		当前银行存有资金总额：%d元\n", total);
	return 0;
}