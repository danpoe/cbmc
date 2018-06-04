int x;

void func()
{
  int i = 0;
}

int main()
{
  int nondet;

  func();

  if(nondet)
  {
    x = 1;
  }

  func();

  return 0;
}
