#include <stdio.h>

void print_use_index(int *ages, char **names, int count) {
	for(int i = 0; i < count; i++) {
		printf("%s has %d years alive.\n", names[i], ages[i]);
	}
}

void print_use_pointer(int *ages, char **names, int count) {
    for(int i = 0; i < count; i++) {
        printf("%s is %d years old.\n", *(names+i), *(ages+i));
    }
}

void print_with_pointer_increment(int *ages, char **names, int count) {
    int *cur_age = ages;
    char **cur_name = names;
	/*
    for(; (cur_age - ages) < count; cur_name++, cur_age++) {
        printf("%s lived %d years so far.\n", *cur_name, *cur_age);
    }*/
    while((cur_age - ages) < count) {
        printf("%s lived %d years so far.\n", *cur_name, *cur_age);
        cur_name++;
        cur_age++;
    }
}

int main(int argc, char *argv[])
{
    // create two arrays we care about
    int ages[] = {23, 43, 12, 89, 2};
    char *names[] = {
        "Alan", "Frank",
        "Mary", "John", "Lisa"
    };

    // safely get the size of ages
    int count = sizeof(ages) / sizeof(int);
    int i = 0;
	
    /* first way using indexing
    for(i = 0; i < count; i++) {
        printf("%s has %d years alive.\n",
                names[i], ages[i]);
    }*/
	print_use_index(ages, names, count);
    printf("---\n");

    // setup the pointers to the start of the arrays
    int *cur_age = ages;
    char **cur_name = names;

    /* second way using pointers
    for(i = 0; i < count; i++) {
        printf("%s is %d years old.\n",
                *(cur_name+i), *(cur_age+i));
    }*/
	print_use_pointer(ages, names, count);
    printf("---\n");

    /* third way, pointers are just arrays
    for(i = 0; i < count; i++) {
        printf("%s is %d years old again.\n",
                cur_name[i], cur_age[i]);
    }*/
	print_use_pointer(ages, names, count);
    printf("---\n");

    /* fourth way with pointers in a stupid complex way
    for(cur_name = names, cur_age = ages;
            (cur_age - ages) < count;
            cur_name++, cur_age++)
    {
        printf("%s lived %d years so far.\n",
                *cur_name, *cur_age);
    }*/
	print_with_pointer_increment(ages, names, count);
	printf("---\n");

	for(i = 0; i < count; i++) {
		printf("ages[%d]  pointer address: %p\n", i, (void*)&cur_age[i]);
		printf("names[%d] pointer address: %p\n", i, (void*)&cur_name[i]);
	}

    return 0;
}
