#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

static sigset_t sigset;
static void wait_next_activation(void)
{
    int dummy;

    sigwait(&sigset, &dummy);
}
int start_periodic_timer(uint64_t offs, int period)
{
    struct itimerval t;

    t.it_value.tv_sec = offs / 1000000;
    t.it_value.tv_usec = offs % 1000000;
    t.it_interval.tv_sec = period / 1000000;
    t.it_interval.tv_usec = period % 1000000;

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGALRM);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    return setitimer(ITIMER_REAL, &t, NULL);
}

static void job_body(void)
{
    static int cnt;
    static uint64_t start;
    uint64_t t;
    struct timeval tv;

    if (start == 0) {
        gettimeofday(&tv, NULL);
	start = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
    }
        
    gettimeofday(&tv, NULL);
    t = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
    if (cnt && (cnt % 100) == 0) {
        printf("Avg time: %f\n", (double)(t - start) / (double)cnt);
    }
    cnt++;
}

//Tasks

void task1(void)
{
  int i,j;
 
  for (i=0; i<3; i++) {
    for (j=0; j<1000; j++) ;
    printf("1");
    fflush(stdout);
  }
  printf("\t");
}

void task2(void)
{
  int i,j;

  for (i=0; i<5; i++) {
    for (j=0; j<10000; j++) ;
    printf("2");
    fflush(stdout);
  }
  printf("\t");
}

void task3(void)
{
  static uint64_t previous;
  uint64_t t;
  struct timeval tv;

  if (previous == 0) {
    gettimeofday(&tv, NULL);
    previous = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
  }

  gettimeofday(&tv, NULL);
  t = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
  printf("\tT: %Lu\n", t - previous);
  previous = t;
}

//Main
int main(int argc, char *argv[])
{
    int res;
    int n;

    res = start_periodic_timer(1000, 50);
    if (res < 0) {
        perror("Start Periodic Timer");

        return -1;
    }

    while(1) {
      n = 1;
      
      while(n <= 6){
        //50ms
        wait_next_activation();
        task1();
        if (n % 2 == 0)
          task2();
        if (n % 3 == 0)
          task3();
        n++;
        
      }

      printf("\nEND LOOP: 150ms\n");
    }

    return 0;
}