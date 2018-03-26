// Fs_middle.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "fsmid_type.h"
#include "list_linux.h"

struct test_list  
{  
	const char *name;  
	struct list_head test;  
	int value;  
};  
static struct test_list lists[5] = {0};
struct list_head head = {  
	&(head), &(head)  
};  

FSMID_FILE f;  

int _tmain(int argc, _TCHAR* argv[])
{
	lists[0].value = 0;
	lists[0].name = "i0";
	lists[1].value = 1;
	lists[1].name = "i1";
	lists[2].value = 2;
	lists[2].name = "i2";
	lists[3].value = 3;
	lists[3].name = "i3";
	lists[4].value = 4;
	lists[4].name = "i4";
	memset(&f,0,sizeof(f));
	struct list_head *mod;  
	unsigned int i = 0;  
# if 0
	list_add_tail(&lists[0].test, &head);  
	list_add_tail(&lists[1].test, &head);  
	list_add_tail(&lists[2].test, &head);  
	list_add_tail(&lists[3].test, &head);  
	list_add_tail(&lists[4].test, &head);  
#else
	list_add(&lists[0].test, &head);  
	list_add(&lists[1].test, &head);  
	list_add(&lists[2].test, &head);  
	list_add(&lists[3].test, &head);  
	list_add(&lists[4].test, &head);  
#endif
	struct test_list *inode;  
	list_for_each(mod, &head)  
	{  

		inode = list_entry(mod, struct test_list, test);  
		printf("%d %s\n", inode->value, inode->name);  
	}         
	inode = list_entry(mod, struct test_list, test);  
	printf("%d %s\n", inode->value, inode->name);  

	getchar();  
	return 0;
}

