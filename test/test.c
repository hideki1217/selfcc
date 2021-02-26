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
int test29(){int a;a=4;int d; d=3*a; +(a+d)*3;}
int test30(){int abc;abc=4;}
int test31(){int abc;int abcd;abc=5;abcd=abc/5;}
int test32(){
    int x;
    x=6;
    /*x=9;
    x++;
    */
    return x;
}
int test33(){
    int x;
    x=6;
    //x=9;
    return x;
}
int test34(){int aho;int aho_aho;aho=3; aho_aho=aho*2; return aho_aho*aho;}
int test35(){int x_1;int x_2;x_1=100;x_2=2;return x_1/x_2; return 0;}
int test36(){int x;x=9; if(x==9)return 5;return 3;}
int test37(){int x;x=9; if(x!=9)return 5;return 3;}
int test38(){int x;x=9; if(x!=9)return 5;else return 3;return 1;}
int test39(){int x;x=9; if(x==5)x=x*3;else x=x*4;return x;}
int test40(){int x;x=9; if(x==3) if(x==8)return 3; else return 5; else return 6;}

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
    assert(48,test29());
    assert(4,test30());
    assert(1,test31());
    assert(6,test32());
    assert(6,test33());
    assert(18,test34());
    assert(50,test35());
    assert(5,test36());
    assert(3,test37());
    assert(3,test38());
    assert(36,test39());
    assert(6,test40());

    print("OK\n");
}