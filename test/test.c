int test1(){0;}
int test2(){42;}
int test3(){4+3-5;}
int test4(){9-10+4;}
int test5(){ 4 + 3   - 5;}
int test6(){9 -  10 +4;}
int test7(){5+6*7;}
int test8(){5*(9-6);}
int test9(){(3+5)/2;}
int test10(){-8+10;}
int test11(){+(-8)-(-10);}
int test12(){4+1>3;}
int test13(){4*1>=4;}
int test14(){4*1<+3;}
int test15(){4*1 <= -(-4);}
int test16(){(4*3)-1!=5*2;}
int test17(){int x;x=3; x*4+3;}
int test18(){
    int x=0;
    return ++x;
}
int test19(){
    int x=1;
    return --x;
}
int test20(){
    int x=0;
    return x++;
}
int test21(){
    int x=1;
    return x--;
}
int test22(){
    int x=0;
    for(int i=0;i<5;i++){
        x=x+i;
    }
    return x;
}
int test23(){
    char *x="ahoaho";
    x++;
    return x[0]-x[2];
}
int test24(){
    int x[5];
    for(int i=0;i<5;i++){
        x[i]=i;
    }
    int *y=x+2;
    print_num(*y);
    y++;
    print_num(*y);
    y--;
    print_num(*y);
    y--;
    return *y;
}
int test25(){
    int x=9;
    x+=3;
    return x;
}
int test26(){
    int x=9;
    x-=4;
    return x;
}
int test27(){
    int x=9;
    x*=2;
    return x;
}
int test28(){
    int x=9;
    x/=3;
    return x;
}

int count;
int assert(int answer,int res){
    if(answer == res){
        print("test%d is pass :: expect: %d, actual: %d\n",count,answer,res);
    }else{
        print("test%d is failed :: expect: %d, actual: %d\n",count,answer,res);
        doexit(1);
    }
    count=count+1;
}

int main(){
    count=1;
    assert(0,test1());
    assert(42,test2());
    assert(2,test3());
    assert(3,test4());
    assert(2,test5());
    assert(3,test6());
    assert(47,test7());
    assert(15,test8());
    assert(4,test9());
    assert(2,test10());
    assert(2,test11());
    assert(1,test12());
    assert(1,test13());
    assert(0,test14());
    assert(1,test15());
    assert(1,test16());
    assert(15,test17());

    assert(1,test18());
    assert(0,test19());
    assert(0,test20());
    assert(1,test21());
    assert(10,test22());
    assert(7,test23());
    assert(1,test24());
    assert(12,test25());
    assert(5,test26());
    assert(18,test27());
    assert(3,test28());

    print("OK\n");
}