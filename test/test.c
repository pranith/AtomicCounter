#include<stdio.h>

volatile int v;

extern void test23(void);

__attribute__((noinline)) void test1(void)
{
  v += 1;
}

__attribute__((noinline)) void test2(void)
{
  test1();
  test1();
  test1();
  __sync_fetch_and_add(&v, 5);
  v += 2;
}

__attribute__((noinline)) void test3(void)
{
  __sync_fetch_and_add(&v, 5);
  __sync_fetch_and_add(&v, 5);
  __sync_fetch_and_add(&v, 5);
}

int main()
{
  int i = 0;
  test1();
  test2();
  for (i = 0; i < 10; i++) {
    test3();
    v += 3;
  }
  //test23();

  printf("%d\n", v);

  return 0;
}
