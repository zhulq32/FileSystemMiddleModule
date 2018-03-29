// Fs_middle.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "list_linux.h"

struct test_list  
{  
	const char *name;  
	struct list_head _node;  
	int value;  
};  
static struct test_list myList[5] = {0};
struct list_head myHead = {  
	&(myHead), &(myHead)  
};  

extern void fsmid_test_task(void*);

int _tmain(int argc, _TCHAR* argv[])
{
	myList[0].value = 0;
	myList[0].name = "i0";
	myList[1].value = 1;
	myList[1].name = "i1";
	myList[2].value = 2;
	myList[2].name = "i2";
	myList[3].value = 3;
	myList[3].name = "i3";
	myList[4].value = 4;
	myList[4].name = "i4";
	unsigned int i = 0;  
# if 1
	list_add_tail(&myList[0]._node, &myHead);  
	list_add_tail(&myList[1]._node, &myHead);  
	list_add_tail(&myList[2]._node, &myHead);  
	list_add_tail(&myList[3]._node, &myHead);  
	list_add_tail(&myList[4]._node, &myHead);  
#else
	list_add(&myList[0]._node, &myHead);  
	list_add(&myList[1]._node, &myHead);  
	list_add(&myList[2]._node, &myHead);  
	list_add(&myList[3]._node, &myHead);  
	list_add(&myList[4]._node, &myHead);  
#endif
	struct list_head *container;  
	struct test_list *iterator;  
	list_for_each(container, &myHead)  
	{  

		iterator = list_entry(container, struct test_list, _node);  
		printf("%d %s\n", iterator->value, iterator->name);  
	}         
	iterator = list_entry(container, struct test_list, _node);  
	printf("%d %s\n", iterator->value, iterator->name);  

	fsmid_test_task(NULL);
	getchar();  
	return 0;
}

