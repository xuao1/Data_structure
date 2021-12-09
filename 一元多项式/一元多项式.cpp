#define _CRT_SECURE_NO_WARNINGS
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<algorithm>
#include<iostream>

using namespace std;

typedef struct LNode{
    float a;
    int c;
    struct LNode* next;
}LNode,*LinkList;

void InsertList(LinkList& L, LinkList x)
{//升序
    LinkList q = L->next, pre = L;
    while (q) {
        if (q->c > x->c) {
            pre->next = x;
            x->next = q;
            return;
        }
        if (q->c == x->c) {
            q->a += x->a;
            return;
        }
        q = q->next;
        pre = pre->next;
    }
    pre->next = x;
    x->next = NULL;
}

void InterFace(int &order) {
    printf("----------------------------------------------\n");
    printf("-----------Univariate Polynomial--------------\n");
    printf("-----------1.Addition-------------------------\n");
    printf("-----------2.Subtraction----------------------\n");
    printf("-----------3.Multiplication-------------------\n");
    printf("-----------4.Calculate x----------------------\n");
    printf("-----------0.Esc------------------------------\n");
    printf("Please Input Instructions: ");
    scanf("%d", &order);
}

void ScanfList(LinkList& A, LinkList& B)
{
    printf("Please Input the First Number: \n");
    int n;
    scanf("%d", &n);
    for (register int i = 1; i <= n; i++) {
        LinkList q = new LNode;
        scanf("%f%d", &q->a, &q->c);
        InsertList(A, q);
    }
    printf("Please Input the Second Number: \n");
    scanf("%d", &n);
    for (register int i = 1; i <= n; i++) {
        LinkList q = new LNode;
        scanf("%f%d", &q->a, &q->c);
        InsertList(B, q);
    }
}

void ScanfList_2(LinkList &A, double &x)
{
    printf("Please Input the First Number: \n");
    int n;
    scanf("%d", &n);
    for (register int i = 1; i <= n; i++) {
        LinkList q = new LNode;
        scanf("%f%d", &q->a, &q->c);
        InsertList(A, q);
    }
    printf("Please Input x: \n");
    scanf("%lf", &x);
}

void Cal_x(LinkList A,double num_x)
{
    double ans=0.0;
    LinkList q = A->next;
    while (q) {
        double tmp = 1.0;
        for (register int i = 1; i <= q->c; i++) tmp *= num_x;
        ans += (tmp * (double)q->a);
        q = q->next;
    }
    printf("The answer is: %lf\n",ans);
}

void Clear(LinkList& L)
{
    LinkList q = L->next;
    L->next = NULL;
    while (q) {
        LinkList p = q->next;
        free(q);
        q = p;
    }
}

void AddList(LinkList A, LinkList B, LinkList &C)
{//A加B，结果存储到C中
    LinkList p = A->next, q = B->next, r = C;
    while (p && q) {
        if (p->c < q->c) {
            LinkList tmp = new LNode;
            tmp->a = p->a;  tmp->c = p->c;
            r->next = tmp;  r = tmp;  
            p = p->next;
            continue;
        }
        if (q->c < p->c) {
            LinkList tmp = new LNode;
            tmp->a = q->a;  tmp->c = q->c;
            r->next = tmp;  r = tmp;
            q = q->next;
            continue;
        }
        if (p->c == q->c) {
            LinkList tmp = new LNode;
            tmp->a = p->a+q->a;  tmp->c = p->c;
            r->next = tmp;  r = tmp;
            p = p->next;    q = q->next;
            continue;
        }
    }
    while (p) {
        LinkList tmp = new LNode;
        tmp->a = p->a;  tmp->c = p->c;
        r->next = tmp;  r = tmp;
        p = p->next;
    }
    while (q) {
        LinkList tmp = new LNode;
        tmp->a = q->a;  tmp->c = q->c;
        r->next = tmp;  r = tmp; 
        q = q->next;
    }
    r->next = NULL;
}

void SubList(LinkList A, LinkList B, LinkList& C)
{//把B的每一位取相反数
    LinkList p = A->next, q = B->next, r = C;
    while (p && q) {
        if (p->c < q->c) {
            LinkList tmp = new LNode;
            tmp->a = p->a;  tmp->c = p->c;
            r->next = tmp;  r = tmp;
            p = p->next;
            continue;
        }
        if (q->c < p->c) {
            LinkList tmp = new LNode;
            tmp->a = -q->a;  tmp->c = q->c;
            r->next = tmp;  r = tmp;
            q = q->next;
            continue;
        }
        if (p->c == q->c) {
            LinkList tmp = new LNode;
            tmp->a = p->a - q->a;  tmp->c = p->c;
            r->next = tmp;  r = tmp;
            p = p->next;    q = q->next;
            continue;
        }
    }
    while (p) {
        LinkList tmp = new LNode;
        tmp->a = p->a;  tmp->c = p->c;
        r->next = tmp;  r = tmp;
        p = p->next;
    }
    while (q) {
        LinkList tmp = new LNode;
        tmp->a = -q->a;  tmp->c = q->c;
        r->next = tmp;  r = tmp;
        q = q->next;
    }
    r->next = NULL;
}

void MulList(LinkList A, LinkList B, LinkList& C)
{
    LinkList D = new LNode; LinkList E = new LNode; LinkList F = new LNode;
    C->next=D->next = E->next = F->next = NULL;
    LinkList p1 = A->next, p2 = B->next, p4 = D;
    int cnt = 0;
    while (p2) {
        while (p1) {
            LinkList tmp = new LNode;
            tmp->a = p1->a * p2->a;
            tmp->c = p1->c + p2->c;
            p4->next = tmp; p4 = tmp; p4->next = NULL;
            p1 = p1 -> next;
        }
        if (cnt % 2 == 0) {
            AddList(C, D, E);
        }
        if (cnt % 2 == 1) {
            AddList(E, D, C);
        }
        cnt++;
        p1 = A->next;   p4 = D;     p4->next = NULL;
        p2 = p2->next;
    }
    if (cnt % 2 == 1) {
        AddList(F, E, C);
    }
    Clear(D);   Clear(E);    Clear(F);
}

void DeleteZeroLink(LinkList& L)
{
    LinkList q = L->next, pre = L;
    while (q) {
        if (q->a == 0.0) {
            pre->next = q->next;
            q = q->next;
        }
        else {
            q = q->next;
            pre = pre->next;
        }
    }
}

void PrintList(LinkList& L)
{
    DeleteZeroLink(L);
    LinkList q = L->next;
    printf("The answer is:\n");
    if (!q) {
        printf("0\n");
        return;
    }
    if (q->c == 0)  printf("%.2f ", q->a);
    else if (q->a == 1.0 || q->a == -1.0) {
        if (q->a == 1.0)printf("x^%d ", q->c);
        else printf("x^%d ", q->c);
    }
    else  printf("%.2f*x^%d ", q->a, q->c);
    q = q->next;
    while (q) {
        if (q->a > 0) printf("+");
        if (q->a == 1.0 || q->a == -1.0) {
            if (q->a == 1.0)printf(" x^%d ", q->c);
            else printf(" -x^%d ", q->c);
        }
        else printf(" %.2f*x^%d ", q->a, q->c);
        q = q->next;
    }
}

int main()
{
    int order;
    LinkList A = new LNode; LinkList B = new LNode; LinkList C = new LNode;
    while (1) {
        A->next = NULL; B->next = NULL; C -> next = NULL;
        InterFace(order);
        if (order == 1) {
            ScanfList(A, B);
            AddList(A, B, C);
            PrintList(C);
        }
        else if (order == 2) {
            ScanfList(A, B);
            SubList(A, B, C);
            PrintList(C);
        }
        else if (order == 3) {
            ScanfList(A, B);
            MulList(A, B, C);
            PrintList(C);
        }
        else if (order == 4) {
            double num_x=0;
            ScanfList_2(A, num_x);
            Cal_x(A, num_x);
        }
        else if (order == 0) {
            break;
        }
        else {
            printf("Invalid Instruction！\n");
            continue;
        }
        Clear(A);   Clear(B); Clear(C);
        printf("\n");
    }
    return 0;
}

