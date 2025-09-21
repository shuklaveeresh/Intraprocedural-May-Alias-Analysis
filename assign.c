#include <stdio.h>

void test1(){
  int a=10,b=11;
  int *p,*y;

  p=&a;
  y=&b;

  //OUTPUT(From LLVM IR)

  /*
    Alias information as observed at the last program point in function test1:
    p -> {}
    y -> {}
  
  
  */   

}

void test2(){
  int a=10,b=11;
  int *p,*y;

  if(b%2){
    p=&a;
  }
  else {
    p=&b; 	
  }
  y=&a;
  //OUTPUT(From LLVM IR)

  /*
    Alias information as observed at the last program point in function test2:
    p -> {y}
    y -> {p}
  
  
  */   

}

void test3(){
  int *p,*y,a,b,**p1;

  p=&a;
  y=&b;

  p1=&p;

  *p1=&b;
  //OUTPUT(From LLVM IR)

  /*
    Alias information as observed at the last program point in function test3:
    p -> {y, p1}
    y -> {p, p1}
    p1 -> {p, y}
  
  
  */   

}

void test4(){
  int x = 10, y = 20;
  int *p = &x;
  int *q = &y;
 
  if (x > 5) {
    q = p;
  }

  *p = 30;
  //OUTPUT(From LLVM IR)

  /*
    Alias information as observed at the last program point in function test4:
    p -> {q}
    q -> {p}
  */ 
}

void test5(int *x, int *y)
{
  int a1 = 10, b1 = 20;
  int *c = &a1, *d = &b1;
  x = c;
  y = d;
  //OUTPUT(From LLVM IR)

  /*
    Alias information as observed at the last program point in function test5:
    x -> {c}
    y -> {d}
    c -> {x}
    d -> {y}
  */ 
}

void test6() {
    int arr[5] = {1, 2, 3, 4, 5};
    int a;
    int *p = &arr[0];
    int *q = &a;

    // Assume that in compile time the condition can not be validated
    if (arr[0] == arr[1]) {
        q = &arr[1];
    }
    /*
    Alias information as observed at the last program point in function test6:
    p -> {q}
    q -> {p}
  */ 
}

void test7() {
    int x = 10, y = 20;
    int *p = &x;
    int *q = &y;

    // Loop with conditional pointer reassignment
    for (int i = 0; i < 5; i++) {
        if (i == 2) {
            q = p;  // p and q now alias each other
        }
        *p = *p + 1;  // Modify value pointed by p
        *q = *q - 1;  // Modify value pointed by q
    }

    // Final values of x and y
    printf("x: %d, y: %d\n", x, y);
    /*
    Alias information as observed at the last program point in function test6:
    p -> {q}
    q -> {p}
  */ 
}

void test8(int **r, int flag) {
    int a, b;
    int *p1 = &a;
    int *p2 = &b;

    // At this point:
    // p1 points to a, p2 points to b.

    if (flag == 1) {
        r = &p2;  // r points to p2, which points to b
    } else {
        r = &p1;  // r points to p1, which points to a
    }

    **r = 20;  // Dereferencing r will modify *a or *b depending on flag.

    // p may now point to either a or b, so p and q may alias indirectly through r
    *p1 = 30;  // Dereferencing p (which might point to either a or b)
    *p2 = 40;  // Dereferencing q (which might point to either a or b)
    /*
    Alias information as observed at the last program point in function test6:
    r -> {p1, p2}
    p1 -> {r}
    p2 -> {r}
  */ 
}

int main()
{
  int a = 5, b = 5;
  test1();
  test2();
  test3();
  test4();
  test5(&a, &b);
  test6();
  test7();
  int **r, flag=0;
  test8(r, flag);
}