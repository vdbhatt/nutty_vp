
int mul(int a, int b){
    int sum = 0;
    while(b > 0){
        sum += a;
        --b;
    }
    return sum;
}
int dotp(int *a, int *b, int length){
    int sum = 0 ;
    for(int i = 0; i< length ; ++i){
        sum += mul(a[i],b[i]);
    }
    return sum;
}

int main(){
    int x[4]  = {1,2,3,4} ;
    int y[4]  = {1,2,3,4} ;
    volatile int z = dotp(x,y,4);

 __asm__("ebreak");
 for(;;);
 return 0;
}