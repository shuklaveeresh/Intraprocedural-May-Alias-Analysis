
// WRITE YOUR TEST CASES IN THIS FILE.
// OUTPUT OF EACH FUNCTION MUST BE WRITTEN AT THE END OF THE CORRESPONDING FUNCTION IN CODE COMMENTS.
// OUTPUT MUST BE WRITTEN IN THE PRESCRIBED FORMAT ONLY.

#include <stdio.h>
#include <stdlib.h>


void testCaseOne()
{
    // Declaring integer variables
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;
    // Declaring integer pointers
    int *p = &a;
    int *q = &b;
    int *r = &c;
    int *s = &d;
    int *t = &e;
    // Declaring double pointers
    int **pp = &p;
    int **qq = &q;
    int **rr = &r;
    // Declaring triple pointers
    int ***ppp = &pp;
    int ***qqq = &qq;
    // Self-referential pointer
    int ***selfRefPointer = &selfRefPointer;
    // Array of integer pointers
    int *arr[5] = {p, q, r, s, t};
    int **ptrArr = arr;
    // Function pointer aliasing
    void (*funcPointerAlias)(int **, int **) = NULL;
    // Array of function pointers
    void (*funcPointerArray[2])(int **, int **) = {NULL, NULL};
    // Nested loops for modifying pointer values
    for (int i = 0; i < 3; i++)
    {
        for (int j = i; j < 3; j++)
        {
            int **temp = (i % 2 == 0) ? pp : qq;
            *temp = (j % 2 == 0) ? r : s;
        }
        // Modifying self-referential pointer
        *selfRefPointer = (i % 2 == 0) ? &p : &q;
        **selfRefPointer = (i % 2 == 0) ? q : r;
    }
    // performing pointer arithmetic with array of pointers
    int **ptrOffset = ptrArr + 2;
    *ptrOffset = s;

    ***ppp = s;
    ***qqq = t;
    // Assigning function pointer alias
    funcPointerAlias = (a % 2 == 0) ? funcPointerArray[0] : funcPointerArray[1];

    /*  Function : testCaseOne

        p -> {q, r, s}
        q -> {p, r, s}
        r -> {p, q, s}
        s -> {p, q, r, t}
        t -> {s}
        pp -> {}
        qq -> {}
        rr -> {}
        ppp -> {}
        qqq -> {}
        selfRefPointer -> {p, q}
        ptrOffset -> {s}
    */
}


void testCaseTwo()
{
    // Declaring and initializing integer variables
    int a = 10;
    int b = 20;
    int c = 30;
    int d = 40;
    int e = 50;
    // Declaring and initialize pointers to these integers
    int *p = &a;
    int *q = &b;
    int *r = &c;
    int *s = &d;
    int *t = &e;
    // Declaring double pointers pointing to single pointers
    int **pp = &p;
    int **qq = &q;
    int **rr = &r;
    // Declaring triple pointers pointing to double pointers
    int ***ppp = &pp;
    int ***qqq = &qq;
    // Creating an array of pointers to integers
    int *arr[5] = {p, q, r, s, t};
    // It is the Pointer to the first element of the array
    int **ptrArr = arr;
    // doing Offset pointer moves to the third element of the array
    int **ptrOffset = ptrArr + 2;
    *ptrOffset = s;
    // Loop to modify pointer aliasing behavior
    for (int i = 0; i < 3; i++)
    {
        if (i % 2 == 0)
        {
            *ppp = rr;
        }
        else
        {
            *qqq = pp;
        }
    }
    // Retrieving values indirectly using pointer arithmetic
    int *x = *(ptrArr + 1);
    int *y = *(ptrArr + 3);

    **rr = *y;

    /* Function : testCaseTwo

        p -> {r, s}
        q -> {p, r, s}
        r -> {s}
        s -> {r}
        t -> {}
        pp -> {}
        qq -> {}
        rr -> {}
        ppp -> {}
        qqq -> {}
        ptrOffset -> {s}
        x -> {q}
        y -> {s}
    */
}


void testCaseThree()
{
    // Declaring integer variables
    int a = 10;
    int b = 20;
    int c = 30;
    int d = 40;
    int e = 50;
    // Array of pointers
    int *arr[5] = {&a, &b, &c, &d, &e};
    // Pointers referencing elements of the array
    int *p = arr[0];
    int *q = arr[1];
    int *r = arr[2];
    int *s = arr[3];
    int *t = arr[4];

    int **ptrArr = arr;
    int **ptrOffset = ptrArr + 1;
    // Modifying array elements conditionally
    for (int i = 0; i < 3; i++)
    {
        if (i % 2 == 0)
        {
            ptrArr[i] = s;
        }
        else
        {
            ptrArr[i] = r;
        }
    }

    *(ptrArr + 2) = t;
    **(ptrArr + 1) = **(ptrOffset + 1);
    // Indirect pointer assignment
    int **indirectPtr = &ptrArr[0];
    *indirectPtr = ptrArr[3];
    // Assigning p to even indices
    for (int j = 0; j < 5; j++)
    {
        if (j % 2 == 0)
        {
            ptrArr[j] = p;
        }
    }

    /*  Function : testCaseThree

        p  -> {s, r}
        q  -> {r, t}
        r  -> {s, t}
        s  -> {p, r}
        t  -> {r, q}
        ptrArr[0] -> {s, p}
        ptrArr[1] -> {r, q}
        ptrArr[2] -> {t}
        ptrArr[3] -> {s}
        ptrArr[4] -> {p}
        ptrOffset -> {q}
        indirectPtr -> {s}
    */
}


int main()
{
    testCaseOne();
    testCaseTwo();
    testCaseThree();
   
}
