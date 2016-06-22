extern volatile int v;

void test23(void)
{
  __sync_fetch_and_add(&v, 5);
}
