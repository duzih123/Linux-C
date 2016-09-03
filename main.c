#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef Debug
int main(void)
{
    int number,guess,i=8;
    srand(time(NULL));
    number =rand()%100+1;
    printf("Please guess a number:\n");
    while (i>0) {
        scanf("%d",&guess);
        if(guess==number){
            printf("wonderful you are right\n");
            return 0;
        }else {
            printf("Sorry ,you are wrong\n");
            if(guess>number){
                printf("too high\n");
            }else {
                printf("too low\n");
            }
        }
        i--;
    }
    printf("Game over! \n\n");
    return 0;
}
#endif

